/* include readline1 */

/*readline函数每次读回的都是一行，免除手动拼接buffer的琐碎
比较高效，相对于一个字符/字节的读取、转换，其有一个缓冲区，
读满缓冲区之后才返回
*/

#include "readline.h"

static pthread_key_t	rl_key;//readline key
static pthread_once_t	rl_once = PTHREAD_ONCE_INIT;//初始化readline进程

static void
readline_destructor(void *ptr)
{
	free(ptr);
}

static void
readline_once(void)
{
	pthread_key_create(&rl_key, readline_destructor);
}

typedef struct {
  int	 rl_cnt;			/* initialize to 0 */
  char	*rl_bufptr;			/* initialize to rl_buf */
  char	 rl_buf[READLINE_BUFF];
} Rline;
/* end readline1 */

/* include readline2 */
//使用定义函数readline
static ssize_t
my_read(Rline *tsd, int fd, char *ptr)
{
	if (tsd->rl_cnt <= 0) {
again:
		if ( (tsd->rl_cnt = read(fd, tsd->rl_buf, MAXLINE)) < 0) {
          if (errno == EINTR) 
		  /*EINTR为Linux函数中的返回状态，在不同的函数中意义不同
	  表示某种阻塞的操作，被接收到的信号中断，造成一种错误的返回值*/
		  {
            printf("EINTR ERROR\n");
            goto again;//once operate this sentence, then go to operate again
          }
			return(-1);
		} else if (tsd->rl_cnt == 0)
			return(0);
		tsd->rl_bufptr = tsd->rl_buf;
	}

	tsd->rl_cnt--;
	*ptr = *tsd->rl_bufptr++;
	return(1);
}
/*write The call was interrupted by a signal before any data was written.
read The call was interrupted by a signal before any data was read.
sem_wait The call was interrupted by a signal handler.
recv function was interrupted by a signal that was caught, before any data
was available.*/
//ssize_t即为signed size_t
ssize_t
readline(int fd, void *vptr, size_t maxlen)
{
	int		n, rc;
	char	c, *ptr;
	Rline	*tsd;

	pthread_once(&rl_once, readline_once);
	if ( (tsd = pthread_getspecific(rl_key)) == NULL) {
		tsd = calloc(1, sizeof(Rline));		/* init to 0 */
		pthread_setspecific(rl_key, tsd);
	}

	ptr = vptr;
	for (n = 1; n < maxlen; n++) {
		if ( (rc = my_read(tsd, fd, &c)) == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;
		} else if (rc == 0) {
			if (n == 1)
				return(0);	/* EOF, no data read */
			else
				break;		/* EOF, some data was read */
		}
        else {
          return(-1);		/* error, errno set by read() */
        }
	}

	*ptr = 0;
	return(n);
}
/* end readline2 */

/*readline的异常抛出函数，当读入的命令行数小于零时，即为-1时*/
ssize_t
Readline(int fd, void *ptr, size_t maxlen)
{
	ssize_t		n;

	if ( (n = readline(fd, ptr, maxlen)) < 0)
		err_ret("readline error");
	return(n);
}
