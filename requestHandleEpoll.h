#ifndef REQ_HAND_EPOLL
#define REQ_HAND_EPOLL
#include "requestHandle.h"
#include "sys/epoll.h"
#include "EZwrap.h"
#include <vector>
#include <map>
#include <sstream>

using std::vector;
using std::map;
using std::string;
using std::stringstream;

class requestHandleEpoll : requestHandle{
public:
	requestHandleEpoll(int listen_fd, string root_dir, int events_num = 512, string index_page = "index.html"): requestHandle(root_dir, index_page), _listen_fd(listen_fd), _events(events_num) 
	{
		epoll_fd = epoll_create(1024);
		_ev.data.fd = _listen_fd;
		_ev.events = EPOLLIN;

		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _listen_fd, &_ev);
	}

	virtual void execute()
	{
		int e_fds = 0;
		for(;;)
		{
			EZ_DBG("wait Start\n");
			e_fds = epoll_wait(epoll_fd, &_events[0], _events.size(), -1);
			EZ_DBG("wait End\n");

			for(int i(0); i != e_fds; ++ i)
			{
				if(_events[i].data.fd == _listen_fd)
				{
					auto addr = getCliAddr();
					socklen_t len = sizeof(addr);
					int connfd = Accept(_listen_fd, EZ_R_CAST(&getCliAddr(), SA*), &len);

					if(connfd < 0)
					{
						EZ_ERR("connfd < 0 \n");
						exit(1);
					}

					_ev.data.fd = connfd;
					_ev.events = EPOLLIN;

					sockaddr_in peer_info;
					socklen_t length = sizeof(peer_info);

					if((getpeername(connfd, EZ_R_CAST(&peer_info, SA*), &length)) < 0)
					{
						EZ_ERR("setConnFd ERR\n");
						exit(1);
					}

					string peer = sock_ntop(EZ_R_CAST(&peer_info, SA*), length);
					peerName[connfd] = peer;

					epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connfd, &_ev);
#ifdef DEBUG
					EZ_INFO("ADD A FD");
					EZ_INFO(connfd);
					EZ_INFO("\n");
#endif

				}
				else if(_events[i].events & EPOLLIN)
				{
					EZ_DBG("EPOLLIN\n");
					int cnt(0);

					if((sock_fd = _events[i].data.fd) < 0)
					{
						EZ_ERR("Wrong Sock Fd\n");
						continue;
					}

					while((cnt = Readline(sock_fd, cur_text)) > 0)//It's not efficient;
					//each socket file descriptor should have a buffer for unblocked IO;
					{
						if(cur_text == "\r\n")
						{
							fd2cmd[sock_fd] = extractHeaders(headers[sock_fd]);
							headers.erase(sock_fd);
							_ev.data.fd = sock_fd;
							_ev.events = EPOLLOUT;

							epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sock_fd, &_ev);
							break;
						}
						else
						{
							headers[sock_fd].push_back(cur_text);
						}
					}

					if(cnt <= 0)
					{
						destory(sock_fd);
					}

				}
				else if(_events[i].events & EPOLLOUT)
				{
					EZ_DBG("EPOLLOUT\n");
					if((sock_fd=_events[i].data.fd) < 0)
					{
						EZ_ERR("Wrong Sock Fd\n");
						continue;
					}

					for(auto cmd : fd2cmd[sock_fd])				
					{
						processCmd(cmd.first, cmd.second, sock_fd, peerName[sock_fd]);
					}

					_ev.data.fd = sock_fd;
					_ev.events = EPOLLIN;

					fd2cmd.erase(sock_fd);

					epoll_ctl(epoll_fd, EPOLL_CTL_MOD, sock_fd, &_ev);
				}
				else if(_events[i].events&(EPOLLHUP|EPOLLRDHUP|EPOLLERR))
				{
					EZ_ERR("error occ\n");
				}
			}
		}
	}

private:

	inline void destory(int sock_fd)
	{
#ifdef DEBUG
		EZ_INFO("ERASE A FD");
		EZ_INFO(sock_fd);
		EZ_INFO("\n");
#endif
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sock_fd, NULL);
		close(sock_fd);
		fd2cmd.erase(sock_fd);
		headers.erase(sock_fd);
		peerName.erase(sock_fd);

	}

	inline map<string, string> extractHeaders(const vector<string>& header_vec)
	{
		map<string, string> header;
		string::size_type pos;
		for(auto str : header_vec)
		{
			string command, content;
			dispart(command, content, str);
			header[command] = content;
		}
		return header;
	}

	map<int, map<string, string> > fd2cmd;
	map<int, vector<string> > headers;
	map<int, string> peerName;
	int _listen_fd;
	struct epoll_event _ev;
	vector<struct epoll_event> _events;
	int epoll_fd;
	int sock_fd;
	string cur_text;
};

#endif
