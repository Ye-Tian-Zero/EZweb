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
	Listen(getListenFd(), 2048);
	
	requestHandleEpoll ins(getListenFd(), "./WebSiteSrc/html_book_20150808/reference");
	ins.execute(1);
	
	return 0;
}

