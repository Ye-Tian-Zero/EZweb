#ifndef EZ_WRAP_H
#define EZ_WRAP_H

#include<iostream>
#include<cstring>
#include<cstdlib>
#include<cstdio>
#include<sstream>

#include<errno.h>
#include<signal.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<syslog.h>
#include<fcntl.h>

#define MAXLINE 4096
#define SERV_PORT 9898
#define SA struct sockaddr

//#define DEBUG

#define EZ_INFO(s) {	\
		cout << "\033[34;1m" << s << "\033[0m" << flush;\
}

#define EZ_WARN(s) {	\
		cout << "\033[33;1m" << s << "\033[0m" << flush;\
}

#define EZ_ERR(s){		\
		cout << "\033[31;1m" << s << "\033[0m" << flush;\
}

#ifdef DEBUG
#define EZ_DBG(s){		\
		cout << "\033[35;1m" <<"Debug Info: "<< s << "\033[0m" << flush;\
}
#else
#define EZ_DBG(s){		\
}
#endif


#define EZ_R_CAST(bar, type) reinterpret_cast<type>(bar)

#define EZ_S_CAST(bar, type) static_cast<type>(bar)

using std::string;
using std::stringstream;
using std::endl;
using std::cout;
using std::cin;
using std::flush;

void (*signal_(int signo, void(*func)(int))) (int);

int dameon_init(const char* pname, int facility);

typedef void Sigfunc(int);

string sock_ntop(const struct sockaddr*, socklen_t salen);

int Socket(int, int, int);

void Bind(int, SA*, socklen_t);

void Listen(int, int);

int Accept(int, SA*, socklen_t*);

ssize_t Writen(int fd, const void *vptr, size_t n);

void Inet_pton(int family, char* src, void* dst);

void Connect(int, SA*, socklen_t);

ssize_t Readline(int, void*, size_t);

ssize_t Readline(int fd, string& str);

static ssize_t my_read(int fd, char *ptr);

Sigfunc *sigty(int signo, Sigfunc *func);

ssize_t readn(int fd, void* vptr, size_t n);


#endif
