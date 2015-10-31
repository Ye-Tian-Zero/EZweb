#ifndef REQ_HAND_EPOLL
#define REQ_HAND_EPOLL
#include "requestHandle.h"
#include "sys/epoll.h"
#include "EZwrap.h"
#include <vector>
#include <map>
#include <sstream>
#include <fcntl.h>
#include <pthread.h>
#include "Tpool.h"

using std::vector;
using std::map;
using std::string;
using std::stringstream;


void* EZ_thread_handle(void* arg);

class requestHandleEpoll : public requestHandle{
	friend void* EZ_thread_handle(void* arg);
public:
	requestHandleEpoll(int listen_fd, string root_dir, size_t thread_n = 50,int events_num = 1024, string index_page = "index.html"): requestHandle(root_dir, index_page), _events(events_num), _thread_pool(thread_n), _thread_arg(this){

		e_fds = 0;
		_listen_fd = listen_fd;
		epoll_fd = epoll_create(2048);

		pthread_mutex_init(&m_data_lock, NULL);
		pthread_mutex_init(&epoll_fd_lock, NULL);
		pthread_mutex_init(&e_fds_lock, NULL);
		pthread_mutex_init(&epoll_cond_lock, NULL);
		pthread_cond_init(&epoll_cond, NULL);
		pthread_rwlock_init(&fd2cmd_rwlock, NULL);

		_thread_pool.start();
	}

	virtual void execute(int unblock = 1)
	{
		_ev.data.fd = _listen_fd;
		_ev.events = EPOLLIN|EPOLLET;

		int flags = fcntl(_listen_fd, F_GETFL, 0);
		fcntl(_listen_fd, F_SETFL, flags | O_NONBLOCK);

		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _listen_fd, &_ev);

		for(;;)
		{

			//EZ_ERR("wait Start\n");
			e_fds = epoll_wait(epoll_fd, &_events[0], _events.size(), -1);
		//	cout << "efds: " << e_fds << endl;
			//EZ_ERR("wait End\n");

			pthread_mutex_lock(&epoll_cond_lock);
			int e_fds_copy(e_fds);
			for(int i(0); i != e_fds_copy; ++ i)
			{
				_thread_arg.sock_fd = i;
				//cout << "sock_fd: " << _thread_arg.sock_fd << endl;
				_thread_pool.add(EZ_thread_handle, &_thread_arg);
			}

			pthread_cond_wait(&epoll_cond, &epoll_cond_lock);
			pthread_mutex_unlock(&epoll_cond_lock);


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

		pthread_mutex_lock(&m_data_lock);
		pthread_rwlock_rdlock(&fd2cmd_rwlock);
		close(sock_fd);
		fd2cmd.erase(sock_fd);
		headers.erase(sock_fd);
		peerName.erase(sock_fd);
		pthread_mutex_unlock(&m_data_lock);

		pthread_mutex_lock(&epoll_fd_lock);
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sock_fd, NULL);
		pthread_rwlock_unlock(&fd2cmd_rwlock);
		pthread_mutex_unlock(&epoll_fd_lock);
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


	void* EZ_thread_send(void* arg);

	map<int, map<string, string> > fd2cmd;

	map<int, vector<string> > headers;

	vector<struct epoll_event> _events;

	struct epoll_event _ev;

	int epoll_fd;

	map<int, string> peerName;

	int _listen_fd;
	int sock_fd;

	pthread_mutex_t m_data_lock;
	pthread_rwlock_t fd2cmd_rwlock;
	pthread_mutex_t epoll_fd_lock;
	pthread_mutex_t e_fds_lock;

	pthread_cond_t epoll_cond;
	pthread_mutex_t epoll_cond_lock;

	Tpool _thread_pool;
	
	POD_arg _thread_arg;

	int e_fds;
};

#endif
