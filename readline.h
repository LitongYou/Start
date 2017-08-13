#ifndef __READLINE_H
#define __READLINE_H

#include "wrapunix.h"

#define READLINE_BUFF 4096
extern ssize_t readline(int fd, void *vptr, size_t maxlen);

#endif // __READLINE_H
/*程序的功能就是从命令行中读取一串字符，包括空格*/

/*EINTR错误的产生：当阻塞于某个慢系统调用的一个进程捕获某个信号且相应信号处理函数返回时，
该系统调用可能返回一个EINTR错误。例如：在socket服务器端，设置了信号捕获机制，有子进程，
当在父进程阻塞于慢系统调用时由父进程捕获到了一个有效信号时，内核会致使accept返回一个EINTR错误
(被中断的系统调用)。当碰到EINTR错误的时候，可以采取有一些可以重启的系统调用要进行重启，而对于
有一些系统调用是不能够重启的。例如：accept、read、write、select、和open之类的函数来说，是可以
进行重启的。不过对于套接字编程中的connect函数我们是不能重启的，若connect函数返回一个EINTR错误
的时候，我们不能再次调用它，否则将立即返回一个错误。针对connect不能重启的处理方法是，必须调用
select来等待连接完成。*/