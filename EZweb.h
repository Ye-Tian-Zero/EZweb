#ifndef EZ_WEB_H
#define EZ_WEB_H

#include "EZwrap.h"

void init(int port = SERV_PORT);

inline int& getConnectFd()
{
	static int ConnectFd;
	return ConnectFd;
}

inline int& getListenFd()
{
	static int listen_fd;
	return listen_fd;
}

inline sockaddr_in& getCliAddr()
{
	static sockaddr_in cli_addr;
	return cli_addr;
}

#endif
