#pragma once

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

class CSocket
{
public:
	CSocket();
	CSocket(bool bReuse);
	~CSocket();

	/* Get Set Socket Handle */
	int GetSockHandle();
	void SetSockHandle(int nSocket);

	/* Returns true if socket is open */
	bool IsOpen();
	
	/* Create close socket connection. */
	bool Create(unsigned short nSocketPort, int nSocketType, const char* pSocketAddress = NULL);
	bool Connect(const char* lpszHostAddress, unsigned short uHostPort);
	void Close();
	bool ReuseAddress();

	/* Read from the socket. */
	bool Listen(int nConnectionBacklog = SOMAXCONN);
	bool Accept(sockaddr* lpSockAddr = NULL, unsigned int* lpSockAddrLen = NULL);

	bool GetSockName(std::string& csSocketAddress, unsigned short& uSocketPort);
	bool GetPeerName(std::string& csPeerAddress, unsigned short& uPeerPort);

	/* Send recieve from socket. */
	ssize_t Send(const void* lpBuffer, int nBufferLen, int nFlags = 0);
	ssize_t Receive(void* lpBuffer, int nBufferLen, int nFlags = 0, int timeout = 0);

	bool SetOptions(unsigned int nFlags);

private:
	int m_nSocket;		/* Socket Handle. */
	bool m_bReuse;		/* reuse address */

	/* Binding code etc called from within Create. */
	bool InitializeSocket(unsigned short uPort, const char* pSocketAddress);
};
