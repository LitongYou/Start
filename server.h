#ifndef __KVS_SERVER_H
#define __KVS_SERVER_H

#include "wrapunix.h"
#include "hash.h"
#include "str.h"
/*kvs-server是一个用于hash.c的网络包装器*/
extern void daemonize();
//This is about the kernel thread protection
//其作用就是创建一个守护进程

#endif //__KVS_SERVER_H
