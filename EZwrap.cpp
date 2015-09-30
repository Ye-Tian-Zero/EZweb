#include"EZwrap.h"

void (*signal_(int signo, void(*func)(int))) (int)
{
	struct sigaction act, oact;
	act.sa_handler = func;
	act.sa_flags = 0;
	if(signo == SIGALRM)
	{
		act.sa_flags |= SA_INTERRUPT;
	}
	else
		act.sa_flags |= SA_RESTART;
	sigemptyset(&act.sa_mask);
	if(sigaction(signo, &act, &oact) < 0)
	{
		return SIG_ERR;
	}
	return oact.sa_handler;
}

string sock_ntop(const struct sockaddr* sa, socklen_t slen)
{
	char str[128];
	string ret;

	switch(sa->sa_family)
	{
		case AF_INET:{
			 const struct sockaddr_in *sin = reinterpret_cast<const struct sockaddr_in*>(sa);

			 if(inet_ntop(AF_INET, &sin->sin_addr, str,sizeof(str)) == nullptr)
				 return string();
			
			 ret = str;

			 unsigned short port = ntohs(sin->sin_port);
			 if(port != 0)
			 {
				 stringstream strm;
				 strm << port << flush;
				 ret += ":" + strm.str();
			 }

			 return ret;
		 }

	}
}

int Socket(int family, int type, int protocol)
{
	int sockFd(socket(family, type, protocol));
	if(sockFd >= 0)
	{
		return sockFd;
	}
	else
	{
		cout << "Socket Process Error!" << endl;
		cout << "errno = " << errno << endl;
		exit(0);
	}
}

void Bind(int sockfd, SA* servaddr, socklen_t len)
{
	if(bind(sockfd, servaddr, len) != 0)
	{
		cout << "Bind Error!" << endl;
		cout << "errno = " << errno << endl;
		exit(0);
	}
}

void Listen(int listenfd, int backlog)
{
	if(listen(listenfd, backlog) != 0)
	{
		cout << "Listen ERROR!" << endl;
		cout << "errno = " << errno;
		exit(0);
	}
}

int Accept(int listenfd, SA* cliaddr, socklen_t *len)
{
	int connfd;
	while((connfd = accept(listenfd, cliaddr, len)) < 0)
	{
		if(errno == EINTR)
			continue;
		cout << "Accept Error!" << endl;
		cout << "errno = " << errno;
		exit(0);
	}

	return connfd;
}

ssize_t Writen(int fd, const void *vptr, size_t n)
{
	string s_info(static_cast<const char*>(vptr), static_cast<const char*>(vptr) + n);
	//EZ_WARN(s_info);
	size_t nleft(n);
	ssize_t nwritten;
	const char *ptr(reinterpret_cast<const char*>(vptr));
	while(nleft > 0)
	{

		if( (nwritten = write(fd, ptr, nleft)) <= 0)
		{
			if(nwritten < 0 && errno == EINTR)
				nwritten = 0;
			else
			{
#ifdef DEBUG
				EZ_ERR("write error\n");
#endif
				return -1;
			}	
		}

		nleft -= nwritten;
		ptr += nwritten;
	}

	return n;
}

void Inet_pton(int family, char* src, void* dst)
{
	if(inet_pton(family, src, dst) <= 0)	
	{
		cout << "Inet_pton Error!" << endl;
		cout << "errno = " << errno << endl;
		exit(0);
	}
}

void Connect(int sockfd, SA* servaddr, socklen_t len)
{
	if(connect(sockfd, servaddr, len) != 0)
	{
		cout << "Connect Error!" << endl;
		cout << "errno = " << errno << endl;
		exit(0);
	}
}

static ssize_t my_read(int fd, char *ptr)
{

}

ssize_t Readline(int fd, void* vptr, size_t maxlen)
{
	ssize_t n = 0, rc;
	char	c, *ptr(reinterpret_cast<char*>(vptr));

	for(n = 0; n < maxlen - 1;)
	{
		if( (rc = read(fd, &c, 1)) == 1)
		{
			if(c == '\n')
				break;
			*ptr++ = c;
		}
		else if(rc == 0)
		{
			*ptr = 0;
			return (n - 1);
		}
		else
		{
			if(errno == EINTR)
			{
				continue;
			}
			return -1;
		}
		++n;
	}
	*ptr = 0;
	return n;
}

ssize_t Readline(int fd, string& str)
{
	str.clear();

	ssize_t cnt = 0, read_count;
	char	c;

	for(;;)		//What's the limitation of cnt?;
	{
		if( (read_count = read(fd, &c, 1)) == 1)
		{
			if(c == '\n')
			{
				str += c;
				++ cnt;
				break;
			}
			str +=c;
		}
		else if(read_count == 0)
		{
			return cnt;
		}
		else
		{
			if(errno == EINTR)
			{
				continue;
			}
			return -1;
		}
		++cnt;
	}
	return cnt;
}

Sigfunc *sigty(int signo, Sigfunc* func)
{
	struct sigaction act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if(signo == SIGALRM)
	{
#ifdef SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;
#endif
	}
	else
	{
#ifdef SA_RESTART
		act.sa_flags |= SA_RESTART;
#endif
	}
	if(sigaction(signo, &act, &oact) < 0)
		return SIG_ERR;
	return oact.sa_handler;
}

ssize_t readn(int fd, void* vptr, size_t n)
{
	size_t nleft(n);
	ssize_t nread(0);
	char* ptr(reinterpret_cast<char*>(vptr));

	while(nleft > 0)
	{
		if( (nread = read(fd, ptr, nleft)) < 0)
		{
			if(errno == EINTR)
				nread = 0;
			else
				return -1;
		}
		else if(nread == 0)
		{
			break;
		}
		nleft -= nread;
		ptr  += nread;
	}
	return n - nleft;
}

int dameon_init(const char* pname, int facility)
{
	pid_t pid;

	if( (pid = fork()) < 0)
	{
		return -1;
	}
	else if (pid)
	{
		exit(0);
	}

	if(setsid() < 0)
		return -1;

	signal(SIGHUP, SIG_IGN);

	//fork again necessary?
	if( (pid = fork()) < 0)
	{
		return -1;
	}
	else if (pid)
	{
		exit(0);
	}

	chdir("/");

	for(int i(0); i != 64; ++ i)
	{
		close(i);
	}

	int fd = open("/dev/null", O_RDWR);

	dup2(fd, STDIN_FILENO);
	dup2(fd, STDERR_FILENO);
	dup2(fd, STDOUT_FILENO);

	openlog(pname, LOG_PID, facility);
}




