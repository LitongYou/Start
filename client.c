#include "wrapunix.h"

#define HOST_NAME "localhost"

static int sockfd;
void *send_msg(void *arg);

/* Global settings */
struct {
  int port;
  char *host;
} settings;

/* The process */
void process(int connfd) {
  pthread_t tid;
  int n;
  sockfd = connfd;
  Pthread_create(&tid, NULL, send_msg, NULL);
  //线程创建
  /* Server==>Standard output */
  while (1) {
    char recvline[MAXLINE] = "";
    if ((n = read(sockfd, recvline, MAXLINE)) == 0) break;
	/*fputs是一种函数，具有的功能是向指定的文件写入一个字符串（不自动写入字符串结束标记符‘\0’）。
	成功写入一个字符串后，文件的位置指针会自动后移，函数返回值为非负整数；否则返回EOF(符号常量，其值为-1)。*/
    fputs(recvline, stdout);
    printf("kvstore > "); 
	fflush(stdout);
	//使用fflush（out）后，立刻清空输出缓冲区，并把缓冲区内容输出
	//flush的作用是将内存中缓冲的内容实际写入外存媒介
  }
}

/* Server<==Standard input */
void *send_msg(void *arg) {
  printf("kvstore > ");
  while (1) {
    char sendline[MAXLINE] = "";
    fflush(stdout);
    if (fgets(sendline, MAXLINE - 2, stdin) == NULL) break;
    sendline[strlen(sendline) - 1] = '\0';
    strcat(sendline, "\r\n");
    Write(sockfd, sendline, strlen(sendline));
  }
  shutdown(sockfd, SHUT_WR);
  return NULL;
}

/* New connection */
int conn_new() {
  struct sockaddr_in servaddr;
  struct hostent *hptr;
  struct in_addr **pptr;
  int connfd;
  connfd = Socket(AF_INET, SOCK_STREAM, 0);
  //ipv4的地址格式定义
  //16位的地质类型定义（AF_INET）+16位的端口号+32位IP地址+8字节的填充
  //TCP是面向流的协议
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  /*AF_INET（又称 PF_INET）是 IPv4 网络协议的套接字类型，AF_INET6 则是 IPv6 的；而 AF_UNIX 则是 Unix 系统本地通信。
选择 AF_INET 的目的就是使用 IPv4 进行通信。因为 IPv4 使用 32 位地址，相比 IPv6 的 128 位来说，计算更快，便于用于局域网通信。
而且 AF_INET 相比 AF_UNIX 更具通用性，因为 Windows 上有 AF_INET 而没有 AF_UNIX。*/
  servaddr.sin_port = htons(settings.port);
  /*htons是将整型变量从主机字节顺序转变成网络字节顺序,
  就是整数在地址空间存储方式变为：高位字节存放在内存的低地址处。*/
  if ((hptr = gethostbyname(settings.host)) == NULL) {
    err_quit("Invalid host name");
  }
  pptr = (struct in_addr **)hptr->h_addr_list;
  memcpy(&servaddr.sin_addr, *pptr, sizeof(struct in_addr));
  printf("Connecting to server %s\n", hptr->h_name);
  Connect(connfd, &servaddr, sizeof(servaddr));
  return connfd;
}

/* Initialize settings */
void settings_init(void) {
  settings.port = 9877;
  settings.host = HOST_NAME;
}

/* Instruction of using */
void usage(void) {
  printf("-h ... Designate the host to connect to.\n" \
         "-p ... Specify the port number of the host to be connected.\n");
  exit(1);
}

int main(int argc, char **argv) {
  int connfd;
  char ch;
  usage();
  
  settings_init();
  while (-1 != (ch = getopt(argc, argv, "p:h:"))) {
    switch (ch) {
    case 'p':
      settings.port = atoi(optarg);
	  //全域变量optarg 即会指向此额外参数
      break;
    case 'h':
      settings.host = optarg;
      break;
    default:
      fprintf(stderr, "Illigal option \"%c\"\n", ch);
      usage();
      return 1;
    }
  }
  
  connfd = conn_new();
  process(connfd);
  return 0;
}
