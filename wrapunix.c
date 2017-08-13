
/************************************************************************/
/* Purpose - Define common network functions including error handling   */
/* Function - All wrapper functions are capitalized in the first letter */
/************************************************************************/

#include "wrapunix.h"

int Socket (int family, int type, int protocol) {
  int n;
  if ((n = socket(family, type, protocol)) < 0)
    err_ret("Socket error");
  return n;
}

void Bind(int listenfd, void *servaddr, int size) {
  if (bind(listenfd, servaddr, size) < 0)
    err_ret("Bind error");
}
/*listenfd，用于表示一个已捆绑未连接的套接口的描述字
backlog等待连接队列的最大长度
无错误发生，listen()返回值为0；否则返回-1*/
void Listen(int listenfd, int backlog) {
  char *ptr;
  if ((ptr = getenv("LISTENQ")) != NULL)
    backlog = atoi(ptr);
  /*atoi表示ASCII to integer是把字符串转换成整数型数的函数*/
  if (listen(listenfd, backlog) < 0)
    err_ret("Listen error");
}
/*sockfd套接字描述符，该套接字在listen()后监听连接
addr指针，指向缓冲区，其中接收为通讯层所知的连接实体的地址
addr参数的实际格式由套接口创建时所产生的地址族确定
addrlen指针，输入参数，配合addr一起使用，指向存有addr地址长度的
整型数*/
int Accept(int sockfd, void *servaddr, socklen_t *size) {
  int connfd;
  if ((connfd = accept(sockfd, servaddr, size)) < 0) {
    err_ret("Connect error");
  }
  return connfd;
}

void Write(int sockfd, char *buff, int size) {
  if (write(sockfd, buff, size) < 0) {
    err_ret("Write error");
  }
}

int Read(int sockfd, char *buff, int size) {
  int n;
  if ((n = read(sockfd, buff, size)) < 0) {
    err_ret("Read error");
  }
  return n;
}

void Close(int fd) {
  if (close(fd) < 0) {
    err_ret("close error");
  }
}
/*这个函数转换网络二进制结构到ASCII类型的地址，参数的作用和inet_pton相同，只是多了一个参数socklen_t cnt,
他是所指向缓存区dst的大小，避免溢出，如果缓存区太小无法存储地址的值，则返回一个空指针，并将errno置为ENOSPC。*/
const char *Inet_ntop(int family, const void *addr, char *line, size_t line_size) {
  const char *string;
  if ((string = inet_ntop(family, addr, line, line_size)) == NULL)
    err_ret("Inet_ntop error");
  printf("String = %s", string);
  return string;
}
/*IP地址转换函数，可以在将IP地址在“点分十进制”和“二进制整数”
之间的转换，而且这两个函数能够处理ipv4和ipv6
点分十进制（Dotted Decimal Notation）全称为点分（点式）十进制表示法，
是IPv4的IP地址标识方法。IPv4中用四个字节表示一个IP地址，每个字节按照
十进制表示为0~255。点分十进制就是用4个从0~255的数字，来表示一个IP地址。*/

void Inet_pton(int family, char *ip_str, void *s_addr) {
  if (inet_pton(family, ip_str, s_addr) < 0)
    err_ret("Inet_pton error");
}

int Recv(int sockfd, char *line, int *len) {
  if (recv(sockfd, line, *len, MSG_WAITALL) <= 0)
    err_ret("Recv error: ");
  return *len;
}

int Connect(int sockfd, void *servaddr, socklen_t len) {
  if (connect(sockfd, servaddr, len) < 0)
    err_quit("Connection error: ");
  return len;
}

int Select(int nfds, fd_set *rfds, fd_set *wrds, fd_set *efds, struct timeval *timeout) {
  int flg;
  flg = select(nfds, rfds, wrds, efds, timeout);
  if (0 > flg) {
    err_ret("Select error");
  }
  else if (0 == flg) {
    err_ret("Timeout");
  }
  return flg;
}
/*fcntl()针对文件描述符提供控制，参数fd是被参数cmd操作的描述符
针对cmd的值，fctnl能够接受第三个参数int arg
fd代表欲设置的文件描述符
cmd代表打算操作的指令*/
int Fcntl(int fd, int cmd, int arg) {
  int n;
  if ((n = fcntl(fd, cmd, arg)) == -1) {
    err_ret("Fcntl error");
  }
  return n;
}

pid_t Fork() {
  pid_t pid;
  if ((pid = fork()) < 0) {
    err_ret("Fork error");
  }
  return pid;
}

void *Malloc(size_t size) {
  void *ptr;
  if ((ptr = malloc(size)) == NULL) {
    err_ret("Malloc error");
  }
  return ptr;
}

void Pthread_create(pthread_t *tid, const pthread_attr_t *attr, void *(*func)(void *), void *arg) {
  if (pthread_create(tid, attr, func, arg) != 0) {
    err_ret("Pthread create error");
  }
}
/*计算机创建一个线程默认状态是joinable
*/
void Pthread_detach(pthread_t tid) {
  if (pthread_detach(tid) != 0) {
    err_ret("Pthread detach error");
  }
}

void Pthread_mutex_lock(pthread_mutex_t *mptr) {
  if (pthread_mutex_lock(mptr) != 0) {
    err_ret("Pthread mutex lock error");
  }
}

void Pthread_mutex_unlock(pthread_mutex_t *mptr) {
  if (pthread_mutex_unlock(mptr) != 0) {
    err_ret("Pthread mutex unlock error");
  }
}


/*******************************************/
/* error.c - Define error handling         */
/* 目的 - Define generic error handling    */
/*******************************************/
/*错误机制*/
static void err_doit(int errnoflg, const char *fmt, va_list ap);

/* Output error statement (errno ant） */
void err_ret(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  err_doit(1, fmt, ap);
  va_end(ap);
  return;
}

/* Output error statement (errno none） */
void err_msg(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  err_doit(0, fmt, ap);
  va_end(ap);
  return ;
}

/* Output an error statement and kill it */
void err_quit(const char *fmt, ...) {
  va_list ap;
  /*ap是这个va_list类型的对象，存储了有关额外参数和检索状态的信息，
  该对象应在第一次调用va_arg之前通过调用va_start初始化*/
  va_start(ap, fmt);
  err_doit(0, fmt, ap);
  va_end(ap);
  exit(1);
}

static void err_doit(int errnoflg, const char *fmt, va_list ap) {
  int errno_save, n;
  char buf[MAXLINE + 1];
  errno_save = errno;
  vsprintf(buf, fmt, ap);
  if (errnoflg) {
    n = strlen(buf);
    snprintf(&buf[n], MAXLINE - n, ": %s", strerror(errno_save));
  }
  strcat(buf, "\n");
  fflush(stdout);
  fputs(buf, stderr);
  fflush(stderr);
  return;
}