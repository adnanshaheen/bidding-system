
#pragma once

/*
 * header files
 */
#include "socket.h"

/*
 * Info struct
 * bid from bidder
 * socket of bidder
 */
typedef struct info {
	unsigned int nBid;
	int nSocket;
} INFO;

class CManager
{
public:
	/* Constructor/Destructor */
	CManager(unsigned int nBidders = DEFAULT_BIDDERS, unsigned short nPort = DEFAULT_MANAGER_PORT);
	~CManager();

	/* start the process */
	int Start();

	/* Order the bidders to start bidding */
	int StartBidding();

	/* Accept bidding from the bidders */
	int AcceptBidders(int nTimeout = 0);

	/* Create bidders, fork new processes */
	int CreateBidders();

private:
	/* Send all data */
	int SendAllData(int nSock, const char* pBuffer, size_t* pSize);

	/* Send data to all clients */
	int SendToAll(const char* pBuffer, size_t nSize);

	/* Send kill message */
	int SendKill(int nSock, pid_t nPID);

	/* Find the winner and display other bids */
	int FindWinner();

	/* Remove the losers from map */
	int DeleteBidder(const int nClient);

private:
	CSocket m_cServer;				/* Manager's socket */
	unsigned short m_nServerPort;	/* Manager's port */
	unsigned int m_nBidders;		/* Number of bidders */
	std::map<pid_t, INFO> m_cBids;	/* Map to keep track of PID, Bid, and Socket */
};
