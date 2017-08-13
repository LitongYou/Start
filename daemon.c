
/******************************************
 * Script to become a daemon process      *
 ******************************************/
/*The library hash.c created for kvs store is simple, 
but it is 10 million in about 3 seconds, 1 GB of text 
information can be retrieved by O (1) in memory.
Kvs-server is a network wrapper for hash.c 
Memcached compatible protocol implementation 
(with some unimplemented protocols)*/
/*一个进程包括代码、数据和分配给进程的资源，fork函数通过系统调用创建一个与原来进程近乎完全相同的进程，也就是两个进程可以
做完全相同的事情，但如果出事参数或者传入的变量不同，两个进程也可以做不同的事情。
一个进程调用fork函数后，系统先给新的进程分配资源，例如存储数据和代码的空间，然后把原来的进程的所有值都复制到新的进程中，
只有少数值与原来的进程值不同，相当于克隆一个自己*/
#include <syslog.h>
#include "wrapunix.h"

/* Process becomes daemon 
守护进程的创建*/
void daemonize() {
  pid_t pid;//pid表示fork函数返回的值
  /*pid_tshiyig typedef定义类型
  用它来表示进程ID类型
  */
  /* End parent process */
  if (pid<0){
	  printf("Error in fork!");
  }
  else if ((pid = fork()) != 0) {
    exit(0);
	//结束父进程，证明已经创建一个子进程
	//pid如果不为零，证明子进程已经创建
  }
  
  /* Promoted to session leader */
  setsid();
  /*调用进程不能是进程首进程，也就是说想setsid调用成功，那么调用者就不能是进程组长
  */
  signal(SIGHUP, SIG_IGN);
  /*第一个参数的意义发送给具有Terminal的Controlling Process，当terminal被disconnect时候发送；
  第二个参数为handler，其描述了与信号相关的动作，SIG_IGN代表由interruptKey产生，通常是Ctrl+C
  当指定的信号到达时就会跳转到参数handler指定的函数执行*/
  return;
}
