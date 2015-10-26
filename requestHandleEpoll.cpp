#include "requestHandleEpoll.h"

void* EZ_thread_handle(void* arg)
{
	POD_arg* arg_real = reinterpret_cast<POD_arg*>(arg);
	//int t_sock_fd = static_cast<int>(reinterpret_cast<long>(arg));
	//
	int index = arg_real->sock_fd; //e_fds index
	requestHandleEpoll *const rPtr = arg_real->ptr; //requestHandleEpoll pointer

	delete arg_real;
	arg_real = NULL;


	if((rPtr->_events)[index].data.fd == rPtr->_listen_fd)
	{
		auto addr = getCliAddr();
		socklen_t len = sizeof(addr);

		do 
		{
			int connfd;
			connfd = accept(rPtr->_listen_fd, EZ_R_CAST(&getCliAddr(), SA*), &len);

			if(connfd < 0 && errno == EWOULDBLOCK)
				break;
			if(errno == EINTR)
				continue;
			
			if(connfd < 0)
			{
				EZ_ERR("connfd < 0 \n");
				exit(1);
			}


			sockaddr_in peer_info;
			socklen_t length = sizeof(peer_info);

			if((getpeername(connfd, EZ_R_CAST(&peer_info, SA*), &length)) < 0)
			{
				EZ_ERR("setConnFd ERR\n");
				exit(1);
			}
			string peer = sock_ntop(EZ_R_CAST(&peer_info, SA*), length);


			pthread_mutex_lock(&rPtr->m_data_lock);//lock data

			rPtr->peerName[connfd] = peer;

			pthread_mutex_unlock(&rPtr->m_data_lock);//unlock data

			struct epoll_event l_ev;
			l_ev.data.fd = connfd;
			l_ev.events = EPOLLIN;

			pthread_mutex_lock(&rPtr->epoll_fd_lock);//lock epoll_fd

			epoll_ctl(rPtr->epoll_fd, EPOLL_CTL_ADD, connfd, &l_ev);

			pthread_mutex_unlock(&rPtr->epoll_fd_lock);//unlock epoll_fd

#ifdef DEBUG
			EZ_INFO("ADD A FD");
			EZ_INFO(connfd);
			EZ_INFO("\n");
#endif
		}while(true);
	}

	else if((rPtr->_events)[index].events & EPOLLIN)
	{
		EZ_DBG("EPOLLIN\n");

		int t_sock_fd;

		if((t_sock_fd = (rPtr->_events)[index].data.fd) < 0)
		{
			EZ_ERR("Wrong Sock Fd\n");
		}
		else
		{
			string cur_text;
			int cnt(0);

			cout << "in" << endl;
			while((cnt = Readline(t_sock_fd, cur_text)) > 0)//It's not efficient;
			//each socket file descriptor should have a buffer for unblocked IO;
			{
				EZ_WARN("one loop\n");
				if(cur_text == "\r\n")
				{

					pthread_mutex_lock(&rPtr->m_data_lock);

					rPtr->fd2cmd[t_sock_fd] = rPtr->extractHeaders(rPtr->headers[t_sock_fd]);
					rPtr->headers.erase(t_sock_fd);

					pthread_mutex_unlock(&rPtr->m_data_lock);
					
					struct epoll_event l_ev;
					l_ev.data.fd = t_sock_fd;
					l_ev.events = EPOLLOUT;

					pthread_mutex_lock(&rPtr->epoll_fd_lock);

					epoll_ctl(rPtr->epoll_fd, EPOLL_CTL_MOD, t_sock_fd, &l_ev);

					pthread_mutex_unlock(&rPtr->epoll_fd_lock);
					break;
				}
				else
				{
					pthread_mutex_lock(&rPtr->m_data_lock);
					//cout << cur_text<<endl;
					rPtr->headers[t_sock_fd].push_back(cur_text);

					pthread_mutex_unlock(&rPtr->m_data_lock);
				}
			}
			cout << "out"<< endl;

			if(cnt <= 0)
			{
				EZ_ERR("Destoryed\n");
				rPtr->destory(t_sock_fd);
			}
		}
	}

	else if((rPtr->_events)[index].events & EPOLLOUT)
	{
		EZ_INFO("EPOLLOUT\n");
		int sock_fd;

		if((sock_fd=(rPtr->_events)[index].data.fd) < 0)
		{
			EZ_ERR("Wrong Sock Fd\n");
		}
		else
		{

			cout << "output_fd: " << sock_fd << endl;

			for(auto cmd : (rPtr->fd2cmd)[sock_fd])				
			{
				rPtr->processCmd(cmd.first, cmd.second, sock_fd, (rPtr->peerName)[sock_fd]);
			}
			
			cout << "want lock m_data" << endl;
			pthread_mutex_lock(&rPtr->m_data_lock);
			cout << "locked" << endl;
			rPtr->fd2cmd.erase(sock_fd);
			pthread_mutex_unlock(&rPtr->m_data_lock);

			struct epoll_event l_ev;
			l_ev.data.fd = sock_fd;
			l_ev.events = EPOLLIN;

			pthread_mutex_lock(&rPtr->epoll_fd_lock);
			bool ret = epoll_ctl(rPtr->epoll_fd, EPOLL_CTL_MOD, sock_fd, &l_ev) ;
			pthread_mutex_unlock(&rPtr->epoll_fd_lock);

			if(ret < 0)
				rPtr->destory(sock_fd);

		}
	}

	else if((rPtr->_events)[index].events&(EPOLLHUP|EPOLLRDHUP|EPOLLERR))
	{
		EZ_ERR("error occ\n");
	}

	pthread_mutex_lock(&rPtr->e_fds_lock);
	if(--rPtr->e_fds <= 0)
	{
		pthread_mutex_lock(&rPtr->epoll_cond_lock);
		pthread_mutex_unlock(&rPtr->epoll_cond_lock);
		pthread_cond_signal(&rPtr->epoll_cond);
	}
	pthread_mutex_unlock(&rPtr->e_fds_lock);

}
