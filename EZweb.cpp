#include "EZweb.h"
#include "requestHandle.h"
#include "requestHandleEpoll.h"
#include <fstream>
using std::fstream;
int main(int argc, char* argv[])
{
	init();

	auto addr = getCliAddr();

	socklen_t len = sizeof(addr);

	Listen(getListenFd(), 10);

	while(1)
	{
	//	getConnectFd() = Accept(getListenFd(), reinterpret_cast<SA*>(&getCliAddr()), &len);

		//cout << sock_ntop(reinterpret_cast<SA*>(&getCliAddr()), len) << endl;

	/*	pid_t pid(fork());
		
		if(pid < 0)
		{
			EZ_ERR("fork error!\n");
			sleep(2);
			continue;
		}

		if(pid == 0)
		{
			requestHandle req("./WebSiteSrc/html_book_20150808/reference");
			req.setConnFd(getConnectFd());
			req.execute();
			close(getConnectFd());
			exit(0);
		}
		else
		{
			close(getConnectFd());
		}
		*/

		requestHandleEpoll ins(getListenFd(), "./WebSiteSrc/html_book_20150808/reference");
		ins.execute(1);
	}	

	return 0;
}

