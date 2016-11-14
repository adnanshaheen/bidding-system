
#include "support.h"
#include "log.h"

#include "socket.h"

/*
 * Constructor
 * default for reuse port is true
 */
CSocket::CSocket() : m_bReuse(true)
{
	m_nSocket = INVALID_SOCKET;		/* Initialize as invalid socket handle */
}

/*
 * Constructor
 */
CSocket::CSocket(bool bReuse)
{
	m_bReuse = bReuse;				/* Set reuse port */
	m_nSocket = INVALID_SOCKET;		/* Initialize as invalid socket handle */
}

/*
 * Destructor
 */
CSocket::~CSocket()
{
	if (m_nSocket != INVALID_SOCKET)	/* If socket is valid, close it */
		Close();
}

/*
 * Get socket handle
 */
int CSocket::GetSockHandle()
{
	return m_nSocket;
}

/*
 * Set socket handle
 */
void CSocket::SetSockHandle(int nSocket)
{
	if (IsOpen())	/* If there is already a socket open, close it */
		Close();

	m_nSocket = nSocket;
}

/*
 * Check if socket is open
 */
bool CSocket::IsOpen()
{
	return (m_nSocket == INVALID_SOCKET) ? false : true;
}

/*
 * Create a new socket
 */
bool CSocket::Create(unsigned short nSocketPort, int nSocketType, const char* pSocketAddress/* = NULL*/)
{
	bool bRet = true;

	/* Create a socket */
	m_nSocket = socket(AF_INET, nSocketType, 0);
	if (m_nSocket == INVALID_SOCKET)
		bRet = false;

	/* Bind to the port */
	if (!bRet || !InitializeSocket(nSocketPort, pSocketAddress)) {

		perr_printf("Socket Initializtion failed");		/* error message */
		bRet = false;
		Close();		/* close socket */
	}

	return bRet;
}

/*
 * Close socket
 */
void CSocket::Close()
{
	if (m_nSocket != INVALID_SOCKET) {		/* If valid socket, close it */

		if (close(m_nSocket) == INVALID_SOCKET)
			perr_printf("Can't close socket");	/* error message, if failed to close */

		m_nSocket = INVALID_SOCKET;
	}
}

/*
 * Set reuse port options
 */
bool CSocket::ReuseAddress()
{
	assert(m_nSocket != INVALID_SOCKET);	/* check if socket is valid */

	if (m_bReuse) {		/* If we're reusing the port */

		int nReuse = 1;		/* enable reuse */
		int nRes = setsockopt(m_nSocket, SOL_SOCKET, SO_REUSEADDR, (const char*) &nReuse, sizeof(int));
		if (nRes == INVALID_SOCKET) {

			perr_printf("Couldn't set socket options");		/* error message, if failed */
			return false;
		}
	}

	return true;
}

/*
 * Initialize socket
 */
bool CSocket::InitializeSocket(unsigned short uPort, const char* pSocketAddress)
{
	/* Socket must be created with socket() */
	assert(m_nSocket != INVALID_SOCKET);

	/* Make up address */
	sockaddr_in	SockAddr;
	memset(&SockAddr,0, sizeof(SockAddr));

	SockAddr.sin_family = AF_INET;
	if (pSocketAddress == NULL)
		SockAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Local address. */
	else
	{
		in_addr_t lResult = inet_addr(pSocketAddress);
		if (lResult == INADDR_NONE) {

			perr_printf("Couldn't initialize socket");
			return false;
		}
		SockAddr.sin_addr.s_addr = lResult;
	}

	SockAddr.sin_port = htons(uPort);

	/* Are we reusing address, don't return error */
	ReuseAddress();

	/*
	 * Bind to the address and port
	 * if no error then returns 0
	 */
	int nRes = bind(m_nSocket, (sockaddr*) &SockAddr, sizeof(SockAddr));
	if (nRes == 0)
		return true;

	return false;
}

/*
 * Connect to a socket
 */
bool CSocket::Connect(const char* lpszHostAddress, unsigned short uHostPort)
{
	assert(lpszHostAddress != 0);

	/* Create socket? */
	if (m_nSocket == INVALID_SOCKET)
	{
		m_nSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (m_nSocket == INVALID_SOCKET) {

			perr_printf("Creating socket failed");
			return false;
		}
	}

	/* Fill address machinery of sockets. */
	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));

	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(lpszHostAddress);

	/*
	 * If the address is not dotted notation, then do a DNS 
	 * lookup of it.
	 */
	if (sockAddr.sin_addr.s_addr == INADDR_NONE) {

		hostent* lphost = gethostbyname(lpszHostAddress);
		if (lphost != NULL)
			sockAddr.sin_addr.s_addr = ((in_addr*) lphost->h_addr)->s_addr;
		else {

			perr_printf("Can't get host by name");
			return false;
		}
	}
	sockAddr.sin_port = htons(uHostPort);

	/* Are we reusing address, don't return on error */
	ReuseAddress();

	/* Connects to peer */
	if (connect(m_nSocket, (sockaddr*) &sockAddr, sizeof(sockAddr)) == INVALID_SOCKET) {

		perr_printf("Can't connect");
		return false;
	}

	return true;
}

/*
 * Listen to connections
 * Use SOMAXCONN as default maximum
 */
bool CSocket::Listen(int nConnectionBacklog/* = SOMAXCONN*/)
{
	assert(m_nSocket != INVALID_SOCKET);
	if (listen(m_nSocket, nConnectionBacklog) == INVALID_SOCKET) {

		perr_printf("Listening error");		/* error message if fail to listen */
		return false;
	}

	return true;
}

/*
 * Get socket name and port number
 */
bool CSocket::GetSockName(std::string& csSocketAddress, unsigned short& uSocketPort)
{
	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));

	unsigned int nSockAddrLen = sizeof(sockAddr);

	int nRes = INVALID_SOCKET;

	while ((nRes = getsockname(m_nSocket, (sockaddr*) &sockAddr, &nSockAddrLen)) == INVALID_SOCKET) {

		if (errno != EBUSY) {		/* If busy, don't log error message */

			perr_printf("Couldn't get socket name");		/* log error message */
			return false;
		}
	}

	/* find port and address */
	uSocketPort = ntohs(sockAddr.sin_port);
	csSocketAddress = inet_ntoa(sockAddr.sin_addr);

	return true;
}

/*
 * Get peer name and port
 */
bool CSocket::GetPeerName(std::string& csPeerAddress, unsigned short& uPeerPort)
{
	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));

	unsigned int nSockAddrLen = sizeof(sockAddr);

	bool bResult = (getpeername(m_nSocket, (sockaddr*) &sockAddr, &nSockAddrLen) != INVALID_SOCKET) ? true : false;
	if (bResult) {

		/* get port and address */
		uPeerPort = ntohs(sockAddr.sin_port);
		csPeerAddress = inet_ntoa(sockAddr.sin_addr);
	}
	else
		perr_printf("Couldn't get peer name");		/* log error message */

	return bResult;
}

/*
 * Accept connections
 */
bool CSocket::Accept(sockaddr* lpSockAddr/* = NULL*/, unsigned int* lpSockAddrLen/* = NULL*/)
{
	int hSock = INVALID_SOCKET;

	hSock = accept(m_nSocket, lpSockAddr, lpSockAddrLen);

	if (hSock == INVALID_SOCKET)		/* Can't accept connections, log error message */
		perr_printf("Couldn't accept a connection");

	SetSockHandle(hSock);

	return (hSock != INVALID_SOCKET) ? false : true;
}

/*
 * Send data
 */
ssize_t CSocket::Send(const void* lpBuffer, int nBufferLen, int nFlags/* = 0*/)
{
	ssize_t nBytes = send(m_nSocket, (char*) lpBuffer, nBufferLen, nFlags);

	if (nBytes == INVALID_SOCKET)
		perr_printf("Can't send");

	return nBytes;
}

/*
 * Receive data
 */
ssize_t CSocket::Receive(void* lpBuffer, int nBufferLen, int nFlags/* = 0*/, int timeout/* = 0*/)
{
	if (m_nSocket == INVALID_SOCKET)
		return INVALID_SOCKET;

	/* Added so that socket blocks maximum for some time (20 secs at the moment). */
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(m_nSocket, &readfds);

	timeval tVal;
	tVal.tv_sec = timeout;
	tVal.tv_usec = 0;

	/* We need to pass the current address + 1 in the below command. */
	int nRes = 0;
	if (timeout == 0)
		nRes = select(m_nSocket + 1, &readfds, NULL, NULL, NULL);		/* wait unless data is provided */
	else
		nRes = select(m_nSocket + 1, &readfds, NULL, NULL, &tVal);		/* wait for timeout only */
	if (nRes <= 0) {

		if (nRes == 0)
			nRes = ERR_TIMEOUT;
		else
			perr_printf("select failed");		/* error log message */

		return nRes;
	}

	/* receive data */
	ssize_t nBytes = recv(m_nSocket, (char*) lpBuffer, nBufferLen, nFlags);
	if (nBytes == INVALID_SOCKET)
		perr_printf("Recv failed");

	return nBytes;
}
