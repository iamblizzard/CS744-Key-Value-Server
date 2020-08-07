#include "cs744_thread.h"
// Common libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// #include <assert.h>
#include <pthread.h>
// cache libraries
 #include <arpa/inet.h>
#include <openssl/sha.h>
// server network libraries
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <net/if.h>

#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sys/socket.h>
#define MAXKEYSIZE 256
#define MAXVALUESIZE 262144
#define XMLOVERHEAD 192
#define MAXXMLSIZE 262800
#define MAX_CLIENT_CONN 20
#define TPUT 2
#define TGET 3
#define TDEL 5
#define CSVOVERHEAD 20
#define MAXQUEUESIZE 30
#define EMPTY 'e'
#define SECOND_CHANCE_GIVEN 'g'
#define SECOND_CHANCE_TAKEN 't'
#define DATA_LIMIT_PER_FILE 10

typedef struct server_configuration{
    int num_sets;
    int num_entries;
    int port_number;
    int thread_pool_size;
    char ip_address[40];
    }Server_Configuration;
typedef struct Request {
    int typereq;
    char key[MAXKEYSIZE+8];
    char val[MAXVALUESIZE+8];
}Request;
typedef struct Response {
    int typereq;
     int success;
     char *key;
     char *val;
 }Response;

typedef struct Queue_Item{
    char request_XML[MAXXMLSIZE];
	int client_fd;
}Queue_Item;

typedef struct Queue{
	Queue_Item ** request_XML_queue;
    struct lock queue_lock;
    struct condition queue_empty;
    struct condition queue_full;
    int next_out;
	int next_in;
 }Queue;

typedef struct cache_entry {
    char key[MAXKEYSIZE+8];
    char val[MAXVALUESIZE+8];
}cache_entry;
//Server initialization
void init_server(void);
//Read Server Configuration
int read_integer(char * line, int startindex, int maxsize);
void read_server_configration(Server_Configuration* SF);
void print_server_config(Server_Configuration* SF);
//network functions (if any)

//Cache Functions
void query_KVcache(Request *req, Response *response);
void write_entry_cache(Request *req,int set_index );
void init_cache(Server_Configuration *SF);
void destroy_cache(void);
int getSetId(char* key,int num_sets);
void print_cache_entry(cache_entry* CF);
void print_cache_configuration(void);
//Queue and Thread Pool functions
typedef struct thread_manager{
    int thread_count;
	pthread_t* threads;
}thread_manager;

void init_thread_pool(int thread_count);
void init_queue(void);
void add_work(char  * request_XML, int client_fd);
 void * do_work(void *);
Queue_Item * dequeue(void);
void init_network();
void start_server();
void init_server();
int ServerResponse_to_XML_server(Response* r, char * ans);
 int from_clientXML_to_Request(char * RequestXML, Request * ans);
 void printreqattributes(Request * r);
void print_cache(void);
//Converter-Parser functions
int ServerResponse_to_XML_server(Response* r, char * ans);
int from_clientXML_to_Request(char * RequestXML, Request * req);
// void printreqattributes(Request * r);
void printresponse(Response *);
void insert_KVstore(Request *request, Response * response, char file_name[40], int);
void delete_KVstore(Request *request, Response * response, char file_name[40], int);
int get_KVstore(Request *, int *, FILE* fp, int,int);
void query_KVstore(Request * request, Response * response);
void init_KVstore(void);

//KV-store
