#include "EZweb.h"
void init(int port)
{
	getListenFd() = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	sockaddr_in serv_addr, cli_addr;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	int val = 1;

	setsockopt(getListenFd(), SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));

	Bind(getListenFd(), reinterpret_cast<SA*>(&serv_addr), static_cast<socklen_t>(sizeof(serv_addr)));

	signal(SIGCHLD, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
}
