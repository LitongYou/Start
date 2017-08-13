//控制socket编程的wrap
#ifndef __WRAPUNIX_H
#define __WRAPUNIX_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <stdbool.h>
#include <stdarg.h>
#include <pthread.h>

#define MAXLINE 1024
#define SERV_PORT 9877
#define LISTENQ 1024
/*backlog参数决定了未完成队列和已完成队列中连接数目之和的最大值*/
#define SERV_PORT_STR "9877"
#define SERV_ADDR "127.0.0.1"
#define BUFFSIZE 8192
#define MAXSOCKADDR 128
#define SA struct sockaddr

#define min(a, b) ( (a) < (b) ? (a) : (b) )//if a < b is true, then min(a,b)=a;else min(a,b)=b
#define max(a, b) ( (a) > (b)) ? (a) : (b) )//if a > b is true, then max(a,b)=a;else max(a,b)=b

/* Define network wrapper functions */
extern int Socket(int family, int type, int protocol);//socket functions
extern void Bind(int listenfd, void *servaddr, int size);//bind the machine
extern void Listen(int listenfd, int backlog);//
extern int Accept(int sockfd, void *servaddr, socklen_t *size);
extern void Write(int sockfd, char *buff, int size);//socketfd socket file descriptor
/*这种一般是BSD socket的用法，用在UNIX/Linux系统中，在Unix/Linux系统下，一个socket句柄
可以看做是一个文件，在socket上收发数据，相当于对一个文件竞选读写，所以一个socket句柄通常也用表示文件
句柄的fd来表示*/
extern int Read(int sockfd, char *buff, int size);
extern void Close(int fd);
extern const char *Inet_ntop(int family, const void *addr, char *line, size_t line_size);
extern void Inet_pton(int family, char *ip_str, void *s_addr);
extern int Recv(int sockfd, char *line, int *len);
extern int Connect(int sockfd, void *servaddr, socklen_t len);
extern int Select(int nfds, fd_set *rfds, fd_set *wrds, fd_set *efds, struct timeval *timeout);
/*select()用来等待文件描述词状态的改变. 参数n 代表最大的文件描述词加1, 参数readfds、writefds 和exceptfds 称为描述词组, 
是用来回传该描述词的读, 写或例外的状况. 底下的宏提供了处理这三种描述词组的方式：
   FD_CLR(inr fd, fd_set* set); 用来清除描述词组set 中相关fd 的位
   FD_ISSET(int fd, fd_set *set); 用来测试描述词组set 中相关fd 的位是否为真
   FD_SET(int fd, fd_set*set); 用来设置描述词组set 中相关fd 的位
   FD_ZERO(fd_set *set); 用来清除描述词组set 的全部位
*/
extern int Fcntl(int fd, int cmd, int arg);
extern pid_t Fork();
extern void *Malloc(size_t size);
extern void Pthread_create(pthread_t *tid, const pthread_attr_t *attr, void *(*func)(void *), void *arg);
extern void Pthread_detach(pthread_t tid);
extern void Pthread_mutex_lock(pthread_mutex_t *mptr);//锁
extern void Pthread_mutex_unlock(pthread_mutex_t *mptr);//释放锁

/* Functions related to errors */
extern void err_ret(const char *fmt, ...);
extern void err_msg(const char *fmt, ...);//错误处理函数
extern void err_quit(const char *fmt, ...);

#endif /* __WRAPUNIX_H */

