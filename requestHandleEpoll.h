#ifndef REQ_HAND_EPOLL
#define REQ_HAND_EPOLL
#include "requestHandle.h"
#include "sys/epoll.h"

class requestHandleEpoll : requestHandle{
public:
	requestHandleEpoll(int listen_fd, string root_dir, events_num = 512; string index_page = "index.html"): requestHandle(root_dir, index_page), _listen_fd(listen_fd), _events(events_num) 
	{
		epoll_fd = epoll_create(512);
		ev.data.fd = _listen_fd;
		ev.events = EPOLLIN|EPOLLET;

		epoll.ctl(epfd, EPOLL_CTL_ADD, _listen_fd, &ev);
	}
	virtual void execute();
	{
		int e_fds = 0;
		for(;;)
		{
			e_fds = epoll_wait(epfd, &_events[0], -1);

			for(int i(0); i != e_fds; ++ i)
			{
				if(_events[i].data.fd == _listen_fd)
				{
					auto addr = getCliAddr();
					socklen_t len = sizeof(addr);
					int connfd = Accept(_listen_fd, EZ_R_C(&getCliAddr(), SA*), &len);
					if(connfd < 0)
					{
						EZ_ERR("connfd < 0 \n");
						exit(1);
					}

					ev.data.fd = connfd;
					ev.events = EPOLLIN;

					epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connfd, &ev);
				}
				else if(_events[i].events & EPOLLIN)
				{
					if((sock_fd=_events[i].data.fd) < 0)
					{
						EZ_ERR("Wrong Sock Fd\n");
						continue;
					}

					while((cnt = Readline(sock_fd, cur_text)) > 0)
					{
						if(cur_text = "\r\n")
						{
							map<string, string> header;
							header = extractHeaders(headers[sock_fd]);
							for(auto h:header)
							{
								processCmd(h.first, h.second);
							}
						}
						else
						{
							headers[sock_fd].push_back(cur_test);
							//asdglsajdglkjlksadgjlksadgjlsdg//
						}
					}
				}
			}
		}
	}
	virtual void processCmd(const string& command, const string& content);
private:
	map<int, vector<string> > headers;
	int _listen_fd;
	struct epoll_event _ev;
	vector<struct epoll_event> _events;
	int epoll_fd;
	int sock_fd;
	string cur_text;
};
#endif
