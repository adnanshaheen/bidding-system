#pragma once

#include "socket.h"

class CBidder
{
public:
	CBidder(std::string csServer, unsigned short nServerPort);	/* constructor */
	~CBidder();				/* destructor */

	int Init();				/* initialize the bidders */
	int SendBid();			/* Send a bid manager */
	int RecieveOrder(int nTimeout = 0);	/* Recieve a message from server. e.g. bid/kill/re-bid etc */
	inline pid_t GetPID() const
	{
		return m_nPID;
	}
	inline void SetPID(pid_t nPID)
	{
		m_nPID = nPID;
	}

private:
	CSocket m_cSocket;				/* client socket */
	std::string m_csServer;			/* manager address */
	unsigned short m_nServerPort;	/* manager port */
	pid_t m_nPID;					/* PID for child process */
};
