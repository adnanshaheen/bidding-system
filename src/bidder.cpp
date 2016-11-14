
/*
 * header files
 */
#include "support.h"
#include "log.h"
#include "bidder.h"

/*
 * constructor
 */
CBidder::CBidder(std::string csServer, unsigned short nServerPort)
{
	m_csServer = csServer;
	m_nServerPort = nServerPort;
	SetPID(getpid());
}

/*
 * destructor
 */
CBidder::~CBidder()
{
	m_cSocket.Close();		/* close socket, NOTE: it should be autometically done in ~CSocket() */
}

/*
 * Initialize bidders
 */
int CBidder::Init()
{
	int nRes = 0;
	debug_log("Entering %s ...", __FUNCTION__);		/* debug message to enter function */
	char cBuffer[MAX_MESSAGE_SIZE] = { 0 };
	std::string csAddress;
	unsigned short uSockPort = 0;

	try {
		debug_log("Creating client");
		if (!m_cSocket.Connect(m_csServer.c_str(), m_nServerPort)) {		/* Connect to manager */

			/* Couldn't connect to manager, log error message */
			perr_printf("client connection to server failed");
			nRes = ERR_SOCKET_CONNECT;
			throw nRes;
		}

		if (!m_cSocket.GetSockName(csAddress, uSockPort)) {					/* Get my address and port */

			/* Log error message */
			perr_printf("Couldn't get god's address");
			nRes = ERR_SOCKET_CONNECT;
			throw nRes;
		}

		/* send bidder port and pid number */
		sprintf(cBuffer, "%d: %d", uSockPort, GetPID());
		debug_log("sending to server: %s", cBuffer);			/* log the message for debugging */

		nRes = m_cSocket.Send(cBuffer, strlen(cBuffer), 0);		/* send the data to manager */
	}
	catch (std::exception e) {

		/* In case of any exception, log exception and close bidder */
		perr_printf(e.what());
		m_cSocket.Close();
	}
	catch (...) {

		/* In case of unknown exception, handle it. Close bidder */
		err_printf("Unknown Exception...");
		m_cSocket.Close();
	}

	debug_log("Exiting %s with code %d (0x%x)...", __FUNCTION__, nRes, nRes);
	return nRes;
}

/*
 * Send the bid to manager
 */
int CBidder::SendBid()
{
	int nRes = 0;
	char cMessage[MAX_MESSAGE_SIZE_2] = { 0 };
	debug_log("Entering %s ...", __FUNCTION__);
	try {
		srand(time(NULL) ^ (GetPID() << 16));		/* Make a random bid from other bidders */
		int nBid = rand() % 100;
		sprintf(cMessage, "%d: %d", GetPID(), nBid);
		debug_log(cMessage);
		nRes = m_cSocket.Send(cMessage, strlen(cMessage), 0);	/* send the bid and pid to manager */
	}
	catch (std::exception e) {

		/* catch and log any exception */
		perr_printf(e.what());
	}
	catch (...) {

		/* catch unkown exception */
		err_printf("Unknown exception...");
	}
	debug_log("Exiting %s with code %d (0x%x)...", __FUNCTION__, nRes, nRes);
	return nRes;
}

/*
 * Receive order from manager
 * Manager can ask to 'start' a bid
 * Manager cal also send 'kill'
 */
int CBidder::RecieveOrder(int nTimeout/* = 0*/)
{
	int nRes = 0;
	debug_log("Entering %s ...", __FUNCTION__);
	char cBuffer[MAX_MESSAGE_SIZE_2] = { 0 };
	try {
		nRes = m_cSocket.Receive(cBuffer, MAX_MESSAGE_SIZE_2, 0, nTimeout);		/* Receive the data from manager */
		if (nRes > 0) {
			debug_log("recieved %d bytes \"%s\"", nRes, cBuffer);
			if (strncasecmp(cBuffer, "kill", nRes) == 0) {

				/* Check if bidder lost */
				nRes = ERR_KILLED;
			}
			else if (strncasecmp(cBuffer, "start", nRes) == 0) {
				/*
				 * We are good to start bidding
				 */
				nRes = ERR_SUCCESS;
			}
		}
		else if (nRes == 0) {
			/*
			 * Nothing here so far, keep waiting for the order
			 */
			nRes = ERR_KEEP_WAITING;
		}
		else {
			nRes = ERR_KEEP_WAITING;
		}
	}
	catch (std::exception e) {

		/* Catch and log exception */
		perr_printf(e.what());
	}
	catch (...) {

		/* Catch and log an unknown exception */
		err_printf("Unknown exception...");
	}
	debug_log("Exiting %s with code %d (0x%x)...", __FUNCTION__, nRes, nRes);
	return nRes;
}
