
/************************************
 * simple key value store           *
 ************************************/

#include "server.h"
#include "readline.h"
#include "wrapunix.h"
#include "hash.h"
#include "str.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;//线程初始化
int line_num = 0;//进行中的工作线程数目

/* Interaction message with client */
typedef struct {
  int connfd;//connection file descriptor
  char *recvline;//接受字节
  char *sendline;//发送字节
  int argc;//argument counter，第一个是计算提供的参数到程序
  char **argv;//argument vector，其是对字符串数组的指针
  /*argc，整数，用来统计运行程序的时候，送给主函数的命令行参数的个数
  *argv[]，字符串数组，用来存放指向你的字符串参数的参数数组，每一个元素指向一个参数
  argv[0]指向程序运行的全路径名称，argv[1]指向在DOS命令行中执行程序名后的第一个字符串
  argv[2]指向执行程序名后的第二个字符串*/
  bool fin_flg;//查找flag
} client_t;

/* Command statistics */
struct {
  unsigned int get;
  unsigned int set;
  unsigned int delete;
  unsigned int find;
  unsigned int mem;
} hash_state = {
  0, 0, 0, 0, 0,//确定hash表中的各个状态
};

/* Overall setting */
struct {
  bool do_daemon;//The process begin
  int port;//
  char *inter;//指针
  double uptime;//The time of look up time 
} settings;

/* Function prototype declaration
协议的声明 */
void cmd_set(client_t *client);
void cmd_get(client_t *client);
void cmd_find(client_t *client);
void cmd_delete(client_t *client);
void cmd_status(client_t *client);
void cmd_error(client_t *client);
void cmd_help(client_t *client);
void cmd_quit(client_t *client);
void cmd_purge(client_t *client);//清除

/* Command table */
struct {
  char *name;
  void (*func)(client_t *client);
} cmd_tb[] = {
  { "set", cmd_set },
  { "get", cmd_get },
  { "find", cmd_find },
  { "delete", cmd_delete },
  { "status", cmd_status },
  { "error", cmd_error },
  { "purge", cmd_purge },
  { "help", cmd_help },
  { "quit", cmd_quit },
};
int cmd_tb_size = sizeof(cmd_tb) / sizeof(cmd_tb[0]);
/*定义命令行table的大小*/

/* Set the current time */
double get_time() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec * 1e-6;
}

/* Set up a message
va_list其为一个字符指针，可以理解为指向当前参数的一个指针，取参
必须通过这个指针进行
1、在调用参数之前，定义一个va_list类型的变量，此为ap
2、然后应该对ap进行初始化，让它指向可变参数表里面的第一个参数，这是通过va_start来实现的，第一个参数是
ap本身，第二个参数是在变参表前面，紧挨着一个变量，“...”之前的参数
3、然后是获取参数，调用va_arg，它的第一个参数是ap，第二个参数是要获取的参数的指定类型，然后返回这个指定类型的值，并且把
ap的位置指向变参表的下一个变量位置
4、获取所有的参数之后，我们有必要将这个ap指针关掉，以免发生危险，方法是调用va_end，它是输入的参数，ap置为NULL

 */
void msg_set(client_t *client, char *fmt, ...) {
//...为变参数表，其除去前几个参数是可变的，后面的参数个数和类型是可变的，
//故用三个点“...”做参数占位符
  va_list ap;
  /*ap是这个va_list类型的对象，存储了有关额外参数和检索状态的信息，
  该对象应在第一次调用va_arg之前通过调用va_start初始化*/
  char line[MAXLINE] = "";
  va_start(ap, fmt);
  //读取可变参数的过程就是在堆栈中，使用指针，遍历堆栈段中的参数列表，从低地址到高地址一个一个地把参数内容读取出
  vsnprintf(line, MAXLINE - 2, fmt, ap);
  //_vsnprintf为C语言函数之一，属于可变参数，用于向字符串中打印数据、数据格式用户自定义
  /*函数原型：int vsnprinf(char *str, size_t size, const char *format, va_list, ap);
  函数说明：将可变参数格式化输出到一个字符数组
  参数：str输出到的数组，size指定大小，防止越界，format格式化参数，ap可变参数列表函数用法*/
  strcat(line, "\r\n");
  //将两个char类型连接
  //client->sendline = malloc(sizeof(char) * (strlen(line) + 1));
  client->sendline = malloc(sizeof(char) * (strlen(line) + 1));
  strcpy(client->sendline, line);
  //把从src地址开始且含有‘\0’结束符的字符串复制到以dest开始的地址空间
  va_end(ap);
  /*某一链表的开始和结束*/
}

/* Set up (key,value) 
插入键值对*/
void cmd_set(client_t *client) {
  char *key, *value;
  if (client->argc < 3) {
    msg_set(client, "set the format is invalid. [set <key> <value>]");
    return;//client发送的参数数小于三，则证明不是set语句
  }
  
  //第0个不是key or value，是命令
  key = client->argv[1];
  value = client->argv[2];
  Pthread_mutex_lock(&mutex);
  arr_insert(key, value);
  hash_state.set++;
  hash_state.mem +=
    sizeof(hash_record_t) + strlen(key) + strlen(value) + 2;
  Pthread_mutex_unlock(&mutex);
  msg_set(client, "Success. (key, value): (%s, %s)", key, value);
}

/* Get key value */
void cmd_get(client_t *client) {
  char *value = NULL, *key = NULL;
  if (client->argc < 2) {
	  //argc，整数，用来统计运行程序的时候，送给主函数的命令行参数的个数
    msg_set(client, "get the format is invalid [get <key>]");
  }
  key = client->argv[1];
  Pthread_mutex_lock(&mutex);
  value = arr_get(key);
  hash_state.get++;
  Pthread_mutex_unlock(&mutex);
  if (value == NULL) {
    msg_set(client, "%s has no value", key);
  }
  else {
    msg_set(client, "The (key, value) is: (%s, %s)", key, value);
  }
}

/* Confirm wheter or not it exists the key */
void cmd_find(client_t *client) {
  bool check;
  if (client->argc < 2) {
    msg_set(client, "find the format is invalid [find <key>]");
  }
  check = arr_find(client->argv[1]);
  Pthread_mutex_lock(&mutex);
  hash_state.find++;
  Pthread_mutex_unlock(&mutex);
  if (check) {
    msg_set(client, "True. The Key exists in the Server.");
  }
  else {
    msg_set(client, "False. The Key does not exist in the Server.");
  }
}

/* Delete value associated with key */
void cmd_delete(client_t *client) {
  int check;
  char *value = NULL, *key = NULL;
  if (client->argc < 2) {
    msg_set(client, "delete the format is invalid [delete <key>]");
  }
  key = client->argv[1];
  value = arr_get(key);
  check = arr_delete(client->argv[1]);
  if (check) {
  Pthread_mutex_lock(&mutex);
  hash_state.delete++;
  hash_state.mem -=
    sizeof(hash_record_t) + strlen(key) + strlen(value) + 2;
  Pthread_mutex_unlock(&mutex);
    msg_set(client, "Deleted (key, value): (%s, %s)", key, value);
	//msg_set(client, "success");
  }
  else {
    msg_set(client, "This operation was failed.");
  }
}

/* Obtain the current status */
void cmd_status(client_t *client) {
  int number = arr_get_num();
  msg_set(client,
          "Uptime            	 ... %9.2lf sec\r\n" \
          "Port number       	 ... %9d\r\n" \
          "Key numbers       	 ... %9d\r\n" \
          "get cmd_num       	 ... %9d\r\n" \
          "set cmd_num           ... %9d\r\n" \
          "delete cmd_num    	 ... %9d\r\n" \
          "find cmd_num      	 ... %9d\r\n" \
          "Memory allocated      ... %9d",
		  //内存分配情况
          (get_time() - settings.uptime), settings.port,
          number, hash_state.get, hash_state.set,
          hash_state.delete, hash_state.find,
          hash_state.mem);
}

/* Display of help */
void cmd_help(client_t *client) {
  msg_set(client,
          "set <key> <value>  ...  <key> <value>save the pair of (key,value).\r\n"  \
          "get <key>  ...  <key> Get the value corresponding to the key.\r\n" \
          "delete <key>  ...  Delete the value corresponding to <key>.\r\n" \
          "find <key>  ...  Check if <key> exists.\r\n" \
          "purge  ...  Delete cache.\r\n" \
          "help   ...  Output this message.\r\n" \
          "status  ...  Show server status.\r\n" \
          "quit  ...  Disconnect from the server." \
          );
}

/* Delete cache or called delete the whole hash table*/
void cmd_purge(client_t *client) {  
  Pthread_mutex_lock(&mutex);
  arr_free();
  arr_init();
  Pthread_mutex_unlock(&mutex);
}
//删除掉缓存的数据+

/* Disconnect */
void cmd_quit(client_t *client) {
  client->fin_flg = true;
}

/* Error message */
void cmd_error(client_t *client) {
  msg_set(client, "---- System Error ! ----");
}
//client and server之间的通信
/* Message initialization */
void message_init(client_t *client, int connfd) {
  client->recvline = NULL;
  client->sendline = NULL;
  client->argv = NULL;
  client->argc = 0;
  client->connfd = connfd;
  client->fin_flg = false;
}

/* Get message */
void message_get(client_t *client) {
  int n;
  //client->recvline = malloc(sizeof(char) * MAXLINE);
  client->recvline = malloc(sizeof(char) * MAXLINE);
  /*使用readline.c中的函数readline()*/
  if ((n = readline(client->connfd, client->recvline, MAXLINE)) <= 0) {    
    /* The other party closed */
    client->fin_flg = true;
  }
  else {
    if (client->recvline != NULL) {
      Pthread_mutex_lock(&mutex);
      line_num++;
      printf("## total = %d, line = %s\n", line_num, client->recvline);
      Pthread_mutex_unlock(&mutex);
	  //锁的释放
    }
  }
}

/* Analyze message 
分析获取的message包含的内容信息*/
void message_parse(client_t *client) {
  char **recvline, *cmd;
  int recv_size;
  int i;
  if (client->fin_flg == true || client->recvline == NULL) return;
  recvline = split(client->recvline, &recv_size);//根据空格来划分
  client->argc = recv_size;
  client->argv = recvline;
  cmd = recvline[0];

  /* Execute command*/
  for (i = 0; i < cmd_tb_size; i++) {
    if (strcmp(cmd_tb[i].name, cmd) == 0) {
      cmd_tb[i].func(client);
      break;
    }
  }
  if (i == cmd_tb_size) {
    cmd_error(client);
  }
}

/* Send message */
void message_send(client_t *client) {
  if (client->sendline != NULL) {
    write(client->connfd, client->sendline, strlen(client->sendline));
  }
}

/* Release pointer 
释放通信时的指针*/
void message_free(client_t *client) {
  free(client->recvline);
  free(client->sendline);
  if (client->argv != NULL) {    
    free(client->argv);
  }
  client->argc = 0;
}

/* Dialogue with clients */
void process_client(int connfd) {
  
  client_t *client = malloc(sizeof(client_t));
  while (1) {
    message_init(client, connfd);
    message_get(client);
    message_parse(client);
    message_send(client);
    message_free(client);
    if (client->fin_flg) break;//收到client的消息之后
  }
  
  free(client);
}

/* Client individual processing(This process) */
void *process(void *arg) {
  int connfd;
  /*conn_fd是一个标识已连接套接口的描述字，buffer_r：用于接受数据的缓冲区*/
  connfd = *((int *)arg);
  free(arg);
  Pthread_detach(pthread_self());
  /*在Web服务器中当主线程为每个新来的连接请求创建一个子线程进行处理的时候，主线程并不希望因为
  调用pthread_join而阻塞（因为还要继续处理之后到来的连接请求），这时可以在子线程中加入代码
    pthread_detach(pthread_self())
  */
  //pthread_detach是计算机用语，创建一个线程默认的状态是joinable
  /*创建一个线程默认的状态是joinable，如果一个线程结束运行但没有被join，则它的状态类似于进程中的Zombie Process
  即还有一部分的资源没有被回收（退出状态码），所以创建线程者应该pthread_join来等待线程运行的结束，并可以得到线程的
  退出代码，回收其资源，类似于wait,waitpid；
  但是调用pthread_join(pthread_id)后，如果该线程没有运行结束，调用者会被阻塞，在有些情况下我们并不希望如此，比如在web服务器中
  当主线程为每个新来的链接创建一个子线程进行处理的时候，主线程并不希望因为调用pthread_join阻塞（因为还有后续到来的链接需要处理）
  在此时可以在子线程中加入代码pthread_detach(pthread_self())，或者父线程调用pthread_detach(thread_id)
  非阻塞，可以立即返回，这将该子线程的状态设置为detach，则该线程运行结束后会自动释放所有资源*/
  process_client(connfd);
  Close(connfd);
  return NULL;
}

/* Prepare listenfd */
/*Listen file descriptor定义*/
int conn_listen(void) {
  struct sockaddr_in servaddr;
  int listenfd;
  listenfd = Socket(AF_INET, SOCK_STREAM, 0);
  /*函数原型，int socket(int domain, int type, int protocol);
  参数说明：domain：协议域，又称协议簇，其决定了socket的地址类型，在通信中必须采用对应的地址，AF_INET决定了要用ipv4地址，32位于端口号
  16位的组合；
  type:指定socket类型，流式socket（socket_stream）是一种面向连接的socket，针对面向连接的TCP服务应用
  protocol：参数为0时，会自动选择第二个参数类型对应的默认类型；连接失败时，返回值会是-1*/
  bzero(&servaddr, sizeof(servaddr));//置字节字符串前n个字节为零且包括'\0'&servaddr是要置零的数据的起始地址，sizeof是要置零的数据字节个数
  servaddr.sin_family = AF_INET;
  //sin_family表示协议簇，一般用AF_INET表示tcp/ip协议
  servaddr.sin_port = htons(settings.port);
  //sin_port是要设置端口，客户端、服务端都有自己的端口，但不同的是服务端是绑定的，客户端却不一定是绑定的
  if (settings.inter != NULL) {
    Inet_pton(AF_INET, settings.inter, &servaddr.sin_addr.s_addr);
  }
  else {
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  }
  Bind(listenfd, &servaddr, sizeof(servaddr));
  /*函数原型,int bind(socket, const struct sockaddr* address, socklen_t address_len);
  参数说明，socket:是一个套接字描述符；address:是一个sockaddr结构指针，该结构中包含了要结合的地址和端口号
  address_len:确定address缓冲区的长度；执行成功返回值为0，否则为SOCKET_ERROR*/
  Listen(listenfd, LISTENQ);
  /*listen在套接字函数中表示让一个套接字处于监听到来的连接请求的状态，listen函数使用主动连接套接字变为被连接套接口，使得一个
  进程可以接受其他进程的请求，从而成为一个服务器进程
  listenfd一个已绑定未连接的套接字描述符；backlog连接请求队列(queue of pending connections)的最大长度(一般由2-4)*/
  return listenfd;
}

/* Initialize setting */
void settings_init() {
  settings.do_daemon = false;
  settings.port = 9877;
  settings.uptime = get_time();
  settings.inter = NULL;
}

/* Display usage */
void usage() {
  printf("-p ... Specify the port number to listen to.\n" \
         "-l ... Specify the port number to listen to.\n" \
         "-h ... Output a help message.\n" \
         "-d ... Operate in daemon mode\n");
}

int main(int argc, char *argv[]) {
  struct sockaddr_in *cliaddr = NULL;
  int listenfd;
  int *connfd = NULL;
  socklen_t len;
  pthread_t tid;
  char ch;
  usage();//标记
  
  settings_init();
  while (-1 != (ch = getopt(argc, argv, "dhp:l:"))) {
    switch (ch) {
    case 'p' :
      settings.port = atoi(optarg);
      break;
    case 'h':
      usage();
      exit(0);
      break;
    case 'd':
      settings.do_daemon = true;
      break;
    case  'l':
      settings.inter = optarg;
      break;
    default:
      fprintf(stderr, "Illegal argument \"%c\"", ch);
      return 1;
    }
  }

  if (settings.do_daemon) {
    daemonize();
  }
  
  signal(SIGPIPE, SIG_IGN); 
  /*signal(a,b)，a是我们将要进行处理的信号；b我们处理的方式
  SIGPIPE其是在reader终止之后写pipe的时候发送*/
  /*当服务器close一个连接时，若client端接着发数据；根据TCP协议的规定，会收到一个RST响应，client再往这个服务器发送数据时，系统会发
  出一个SIGPIPE信号给进程，告诉进程这个连接已经断开了，不要再写了，根据信号的默认处理规则SIGPIPE信号的默认执行动作是terminate(终止、退出)
  所以client会退出，若不想客户端退出可以把SIGPIPE设为SIG_IGN，即上述所写，把SIGPIPE交给了系统处理*/
  listenfd = conn_listen();
  arr_init();
  while (1) {
    len = sizeof(*cliaddr);
    //connfd = malloc(sizeof(int));
    connfd = malloc(sizeof(int));
    *connfd = Accept(listenfd, NULL, NULL);
    Pthread_create(&tid, NULL, &process, connfd);
	/*pthread_create(),&tid为线程标识符；connfd为传递给start_routine的参数；&process为线程函数的起始地址*/
  }
  arr_free();
  return 0;
}
