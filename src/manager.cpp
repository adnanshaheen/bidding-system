
#include "support.h"
#include "log.h"
#include "manager.h"
#include "bidder.h"

void ChildSignal(int signal __attribute__((unused)))
{
	/* Signal handler to check what happened */
	debug_log("Signal %d child", signal);
	//while(waitpid(CManager::m_sPID, NULL, WNOHANG) > 0);
}

/*
 * Constructor
 */
CManager::CManager(unsigned int nBidders/* = DEFAULT_BIDDERS*/, unsigned short nPort/* = DEFAULT_MANAGER_PORT*/)
{
	/* Initialize port and bidders */
	if (nPort != 0)
		m_nServerPort = nPort;
	else
		m_nServerPort = DEFAULT_MANAGER_PORT;	/* Set default port i.e. '5000' */

	if (nBidders > 3)
		m_nBidders = nBidders;
	else
		m_nBidders = DEFAULT_BIDDERS;			/* Set default bidders i.e. 3 */
}

/*
 * Destructor
 */
CManager::~CManager()
{
}

/*
 * Start Process
 */
int CManager::Start()
{
	int nRes = 0;
	debug_log("Entering %s ...", __FUNCTION__);
	try {
		/* Check for server port */
		if (m_nServerPort < 1024 || m_nServerPort > 65535) {
			err_printf("Port not allowed for user apps");
			nRes = ERR_INVALID_PORT;
			throw nRes;
		}

		/* Create the server socket */
		debug_log("Creating manager");
		if (!m_cServer.Create(m_nServerPort, SOCK_STREAM)) {
			debug_log("Manager creation failed");
			nRes = ERR_SOCKET_OPEN;
			throw nRes;
		}

		/* Start listening on the socket */
		debug_log("Manager is ready to listen on socket %d", m_cServer.GetSockHandle());
		if (!m_cServer.Listen()) {
			debug_log("Manager can't listen");
			nRes = ERR_SOCKET_LISTEN;
			throw nRes;
		}

		/* Create bidders */
		CreateBidders();
	}
	catch (std::exception e) {
		perr_printf(e.what());
	}
	catch (...) {
		err_printf("Unknown Exception...");
	}
	debug_log("Exiting %s with code %d (0x%x)...", __FUNCTION__, nRes, nRes);
	return nRes;
}

int CManager::CreateBidders()
{
	int nRes = 0;					/* result */
	debug_log("Entering %s ...", __FUNCTION__);	/* debug message for entry point function */
	try {
		for (size_t nBidder = 0; nBidder < m_nBidders; ++ nBidder) {	/* iterate through all bidders to create them */

			/* attach a signal handler to find child termination */
			struct sigaction signal;
			signal.sa_handler = ChildSignal;
			sigemptyset(&signal.sa_mask);
			signal.sa_flags = SA_RESTART;
			if (sigaction(SIGCHLD, &signal, NULL) == -1)
				perr_printf("Could not attach signal.\n");	/* couldn't attach signal, print error */

			debug_log("Creating %d bidder", nBidder);		/* debug log to show number of bidders */
			pid_t nPID = fork();							/* create new processes */
			if (nPID == -1) {
				/* fork failed */
				perr_printf("Couldn't fork");
				nRes = ERR_FORK_FAILED;
				throw nRes;
			}
			else if (nPID == 0) {
				/* child process */
#ifdef DEBUG
				sleep(10);
				debug_log("wait for child ended");
#endif

				unsigned short uSockPort = 0;
				std::string csAddress;
				if (!m_cServer.GetSockName(csAddress, uSockPort)) {	/* get socket address */
					perr_printf("Couldn't get Manager's address");
					nRes = ERR_GET_SOCK_NAME;
					return -1;	/* just return, as this is another process now */
				}

				CBidder cBidder(csAddress, m_nServerPort);	/* create bidder */
				cBidder.Init();		/* Initialize bidder */

				/*
				 * client is created, check for incoming messages
				 */
				int nTimeout = 0;
				int nRes = 0;
				while (1) {
					nRes = cBidder.RecieveOrder(nTimeout);	/* wait unless bidders recieve the message to start bids */
					if (nRes < 0 && nRes != ERR_TIMEOUT)
						break;
					nRes = cBidder.SendBid();		/* start bidding */
					if (nRes < 0 && nRes != ERR_TIMEOUT)
						break;
				}
				debug_log("Exiting %s with code %d (0x%x)...", __FUNCTION__, nRes, nRes);
				exit(0);
			}
			else {
				/* parent process */
				log_message("#%d bidder's PID is %d.", nBidder, nPID);	/* print bidder PID */
				INFO cInfo = { 0 };
				m_cBids.insert(std::make_pair(nPID, cInfo));		/* add pid to map, in order to wait for them */
				if (nBidder == m_nBidders - 1) {
					/*
					 * Bidders have recieved the start message
					 * Start accepting the bids
					 */
					AcceptBidders();
				}
			}
		}
	}
	catch (std::exception e) {
		perr_printf(e.what());
	}
	catch (...) {
		err_printf("Unknown Exception...");
	}
	debug_log("Exiting %s with code %d (0x%x)...", __FUNCTION__, nRes, nRes);
	return nRes;
}

int CManager::StartBidding()
{
	int nRes = 0;
	debug_log("Entering %s ...", __FUNCTION__);
	char cBuffer[MAX_MESSAGE_SIZE] = { 0 };
	size_t nBufferLen = 0;
	try {
		/*
		 * Send 'start' message to bidders
		 * once bidders receive this, they will start bidding
		 */
		sprintf(cBuffer, "start");
		nBufferLen = strlen(cBuffer);
		nRes = SendToAll(cBuffer, nBufferLen);	/* Send to all bidders */
		/*
		 * TODO: check for errors
		 */
	}
	catch (std::exception e) {
		perr_printf(e.what());
	}
	catch (...) {
		err_printf("Unknown exception...");
	}
	debug_log("Exiting %s with code %d (0x%x)...", __FUNCTION__, nRes, nRes);
	return nRes;
}

/*
 * AcceptBidding
 */
int CManager::AcceptBidders(int nTimeout/* = 0*/)
{
	int nRes = 0;
	debug_log("Entering %s ...", __FUNCTION__);
	try {
		struct timeval cTimeout;
		fd_set cReadSet;
		fd_set cMasterSet;
		ssize_t nBytesRecv = 0;
		char cBuffer[MAX_MESSAGE_SIZE_2] = { 0 };
		int nMaxSocket = m_cServer.GetSockHandle();
		int nBidders = 0;

		FD_ZERO(&cReadSet);
		FD_ZERO(&cMasterSet);

		FD_SET(m_cServer.GetSockHandle(), &cMasterSet);

		if (nTimeout) {
			cTimeout.tv_sec = nTimeout;	/* Set the tiem interval */
			cTimeout.tv_usec = 0;
		}

		debug_log("Manager has started to link clients");
		while (true) {

			if (m_cBids.size() == 0)	/* If no more bidders, no more data to recv */
				break;
			cReadSet = cMasterSet;
			if (nTimeout == 0)
				nRes = select(nMaxSocket + 1, &cReadSet, NULL, NULL, NULL);
			else
				nRes = select(nMaxSocket + 1, &cReadSet, NULL, NULL, &cTimeout);

			if (nRes == 0)
				nRes = ERR_TIMEOUT;
			if (nRes == -1) {
				if (errno == EINTR)
					continue;

				perr_printf("select failed");
				throw nRes;
			}
			else {
				int nClient = 0;
				for (nClient = 0; nClient <= nMaxSocket; ++ nClient) {
					if (FD_ISSET(nClient, &cReadSet)) {
						if (nClient == m_cServer.GetSockHandle()) {
							/* Accept new connection */
							struct sockaddr_in cClientAddr;
							socklen_t nAddrLength = sizeof(cClientAddr);
							int nNewSocket = accept(m_cServer.GetSockHandle(),
										(struct sockaddr*) &cClientAddr,
										&nAddrLength);
							if (nNewSocket == INVALID_SOCKET) {
								perr_printf("Couldn't accept client connection");
								continue;
							}
							
							/*
							 * Set the id of net socket in our set
							 */
							FD_SET(nNewSocket, &cMasterSet);
							if (nNewSocket > nMaxSocket)
								nMaxSocket = nNewSocket;
							sprintf(cBuffer, "New connection %s on socket %d (0x%x)",
								inet_ntoa(cClientAddr.sin_addr),
								nNewSocket,
								nNewSocket);

							debug_log(cBuffer);

							strcat(cBuffer, "\n");
							nBytesRecv = strlen(cBuffer);
							memset(cBuffer, '\0', nBytesRecv);

							/*
							 * Receive the data from new connection
							 */
							nBytesRecv = recv(nNewSocket, cBuffer, MAX_MESSAGE_SIZE_2, 0);
							if (nBytesRecv <= 0) {
								if (nBytesRecv == 0) {
									struct sockaddr_in cClientAddr;
									err_printf("Connection closed while checking new connections");
								}
								else if (nBytesRecv == INVALID_SOCKET) {
									perr_printf("Couldn't receive data from socket %d (0x%x)",
										nClient,
										nClient);
								}
								close(nClient);
								FD_CLR(nClient, &cMasterSet);
							}
							else {
								/*
								* new connection should send it's pid_t, and bid if available
								* we already have nNewSocket as SOCKET
								* insert all this information in m_cBids
								*/
								unsigned short nPort = 0;
								pid_t nPID = 0;
								std::string csID;
								std::string csItem;
								std::string csLine = cBuffer;

								csItem = csLine.substr(0, csLine.find(":"));
								nPort = atoi(csItem.c_str());		/* Get port number of bidder */

								int nIndex = csLine.find(":") + 1;
								csItem = csLine.substr(nIndex, csLine.rfind(":") - nIndex);
								nPID = atoi(csItem.c_str());		/* Get PID of bidder */

#ifdef DEBUG
								const char* pID = csID.c_str();
								debug_log("after parsing message %d: %d", nPort, nPID);
#endif
								std::map<pid_t, INFO>::iterator cIter = m_cBids.find(nPID);	/* Find the PID in our map */
								if (cIter != m_cBids.end()) {

									(*cIter).second.nSocket = nNewSocket;
									debug_log("Client has sent: PID:%d SOCKET:%d",
										(*cIter).first,
										nNewSocket);
									++ nBidders;		/* we have a connection, increment it */
									if (nBidders == m_nBidders) {
										/*
										 * We have information from all bidders
										 * Let's start bidding process
										 */
										StartBidding();
										nBidders = 0;
									}
								}
								else {

									/*
									 * Manager don't have this bidder in map
									 * Report it
									 */
									err_printf("Can't find ID %d in map", nPID);
								}
							}
						}
						else {
							/* data from client */
							debug_log("recv from client");
							memset(cBuffer, '\0', MAX_MESSAGE_SIZE_2);
							nBytesRecv = recv(nClient, cBuffer, MAX_MESSAGE_SIZE_2, 0);
							if (nBytesRecv <= 0) {
								if (nBytesRecv == 0) {

									/*
									 * This bidder has left
									 */
									struct sockaddr_in cClientAddr;
									err_printf("Connection closed");
									socklen_t addr_length = sizeof(cClientAddr);
									getsockname(nClient, (struct sockaddr*) &cClientAddr, &addr_length);
									sprintf(cBuffer, "Client %d from %s left",
										nClient, inet_ntoa(cClientAddr.sin_addr));
									err_printf(cBuffer);

									/*
									* remove this item from our list m_cBids
									*/
									DeleteBidder(nClient);
								}
								else if (nBytesRecv == INVALID_SOCKET) {
									perr_printf("Couldn't receive data from socket %d (0x%x)",
										nClient,
										nClient);
								}
								close(nClient);
								FD_CLR(nClient, &cMasterSet);
							}
							else {
								/*
								* Bid is sent in message
								*/
								pid_t nPID = 0;
								std::string csLine = cBuffer;
								std::string csPID;
								std::string csBid;
								int nIndex = csLine.find(":");
								int nNextIndex = csLine.find(":", nIndex + 1);

								csPID = csLine.substr(0, nIndex);
								nPID = atoi(csPID.c_str());	/* PID */

								++ nIndex; /* space */
								csBid = csLine.substr(nIndex + 1, nNextIndex - (nIndex + 1));	/* Bid */
#ifdef DEBUG
								const char* pPID = csPID.c_str();
								const char* pBid = csBid.c_str();
								debug_log("Client sent: PID \"%s\" Bid \"%s\"", pPID, pBid);
#endif
								/*
								 * Find the bidder in Manager's map
								 */
								std::map<pid_t, INFO>::iterator cIter = m_cBids.find(nPID);
								if (cIter != m_cBids.end()) {

									/*
									 * Bidder found, update the map with his bid
									 */
									(*cIter).second.nBid = atoi(csBid.c_str());
									debug_log("PID:%d BID:%d",
										(*cIter).first,
										(*cIter).second.nBid);
									++ nBidders;
									if (nBidders == m_nBidders) {
										/*
										 * We got the last bid
										 * Now compare the bids
										 */
										nRes = FindWinner();
										if (nRes == ERR_RESTART_BIDS) {

											/*
											 * More than one winners
											 * Losers are removed
											 * Restart bidding
											 */
											nBidders = 0;
											StartBidding();
										}
										else if (nRes == ERR_MANAGER_DONE) {
											/*
											 * Winner declared, end Manager
											 */
											break;
										}
									}
								}
								else {

									/*
									 * Couldn't find the bidder in map
									 * Report it
									 */
									err_printf("Can't find ID %s in map", csPID.c_str());
								}
							}
						}
					}
				}
			}
		}
	}
	catch (std::exception e) {
		perr_printf(e.what());
	}
	catch (...) {
		err_printf("Unknown exception...");
	}
	debug_log("Exiting %s with code %d (0x%x)...", __FUNCTION__, nRes, nRes);
	return nRes;
}

/*
 * Send the data to all bidders
 */
int CManager::SendToAll(const char* pBuffer, size_t nSize)
{
	int nRes = 0;
	debug_log("Entering %s ...", __FUNCTION__);
	std::map<pid_t, INFO>::const_iterator cIter = m_cBids.begin();
	while (cIter != m_cBids.end()) {
		/*
		 * If the bidder has a valid socket
		 * Send him data through that socket
		 */
		if ((*cIter).second.nSocket != 0) {

			/*
			 * Send all data
			 */
			nRes = SendAllData((*cIter).second.nSocket,
				       pBuffer,
				       &nSize);
		}
		++ cIter;
	}
	debug_log("Exiting %s with code %d (0x%x)...", __FUNCTION__, nRes, nRes);
	return nRes;
}

/*
 * Send kill message to bidder
 */
int CManager::SendKill(int nSock, pid_t nPID)
{
	int nRes = 0;
	debug_log("Entering %s ...", __FUNCTION__);
	char cBuffer[MAX_MESSAGE_SIZE] = { 0 };
	size_t nBufferLen = 0;
	try {
		DeleteBidder(nSock);	/* Remove him from map before killing him */
		sprintf(cBuffer, "kill");
		nBufferLen = strlen(cBuffer);

		/*
		 * Send all data to bidder
		 */
		nRes = SendAllData(nSock, cBuffer, &nBufferLen);
		/*
		 * TODO: check for errors
		 */
#if 0
		/*
		 * Manager can also send kill signal
		 * But Manager uses TCP/IP
		 */
		nRes = kill(nPID, SIGUSR1);
#endif
	}
	catch (std::exception e) {
		perr_printf(e.what());
	}
	catch (...) {
		err_printf("Unknown exception...");
	}
	debug_log("Exiting %s with code %d (0x%x)...", __FUNCTION__, nRes, nRes);
	return nRes;
}

/*
 * Send all data
 * Only required when data length is more
 * Manager data is small, and always send it in one pass
 */
int CManager::SendAllData(int nSock, const char* pBuffer, size_t* pSize)
{
	int nRes = 0;
	ssize_t nBytesLeft = *pSize;
	ssize_t nBytesSent = 0;
	ssize_t nWritten = 0;

	debug_log("Entering %s ...", __FUNCTION__);

	/*
	 * keep sending the data, unless we have some
	 */
	while (nBytesLeft != 0) {
		debug_log("sending %s to client %d", pBuffer, nSock);

		/*
		 * Send the data
		 */
		nWritten = send(nSock, pBuffer + nBytesSent, nBytesLeft, 0);
		if (nWritten == -1) {
			nRes = ERR_SOCKET_SEND;
			break;
		}

		/*
		 * Calculate the data send and remaining
		 */
		nBytesLeft -= nWritten;
		nBytesSent += nWritten;
	}

	*pSize = nBytesSent;
	debug_log("Exiting %s with code %d (0x%x)...", __FUNCTION__, nRes, nRes);
	return nRes;
}

/*
 * Find the winner
 */
int CManager::FindWinner()
{
	int nRes = 0;
	debug_log("Entering %s ...", __FUNCTION__);
	try {
		int nMaxBid = 0;
		for (std::map<pid_t, INFO>::const_iterator cIter = m_cBids.begin();
			cIter != m_cBids.end();
			++ cIter) {

			/*
			 * Find maximum bidder
			 */
			if (nMaxBid < (*cIter).second.nBid)
				nMaxBid = (*cIter).second.nBid;

			/*
			 * Log message for all bids
			 */
			log_message("Bidder %d has bid %d", (*cIter).first, (*cIter).second.nBid);
		}

		for (std::map<pid_t, INFO>::const_iterator cIter = m_cBids.begin();
			cIter != m_cBids.end();
			++ cIter) {
			/*
			 * kill the bidders, which are less then bids
			 */
			if (nMaxBid > (*cIter).second.nBid)
				SendKill((*cIter).second.nSocket, (*cIter).first);
		}

		if (m_cBids.size() == 1) {
			/*
			 * If we have only one winner
			 * Declare him as winner
			 */
			std::map<pid_t, INFO>::const_iterator cIter = m_cBids.begin();
			if (nMaxBid == (*cIter).second.nBid)
				log_message("Winner is %d", (*cIter).first);

			/*
			 * Kill the winner
			 * Others are already killed
			 */
			SendKill((*cIter).second.nSocket, (*cIter).first);

			/*
			 * Manager is done
			 */
			log_message("Exiting Manager...");
			nRes = ERR_MANAGER_DONE;
		}
		else if (m_cBids.size() > 1) {

			/*
			 * We have more winners, restart bidding
			 * Loosers are already killed
			 */
			log_message("More than one winners, restart bidding amongst winners");
			nRes = ERR_RESTART_BIDS;
		}
	}
	catch (std::exception e) {
		perr_printf(e.what());
	}
	catch (...) {
		err_printf("Unknown exception...");
	}
	debug_log("Exiting %s with code %d (0x%x)...", __FUNCTION__, nRes, nRes);
	return nRes;
}

/*
 * Delete the bidder
 */
int CManager::DeleteBidder(const int nClient)
{
	int nRes = 0;
	try {
		for (std::map<pid_t, INFO>::iterator cIter = m_cBids.begin();
			cIter != m_cBids.end();
			++ cIter) {

			/*
			 * Find the bidder
			 */
			if ((*cIter).second.nSocket == nClient) {

				/*
				 * Remove the bidder from the map
				 */
				debug_log("Removing %d from map", (*cIter).first);
				m_cBids.erase(cIter);
			}
		}
	}
	catch (std::exception e) {
		perr_printf(e.what());
	}
	catch (...) {
		err_printf("Unknown exception...");
	}
	debug_log("Exiting %s with code %d (0x%x)...", __FUNCTION__, nRes, nRes);
	return nRes;
}
