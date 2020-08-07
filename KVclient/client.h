#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <unistd.h>
 #include <arpa/inet.h>

//macros at client side
#define MAXKEYSIZE 256
#define MAXVALUESIZE 262144
#define XMLOVERHEAD 192
#define TPUT 2
#define TGET 3
#define TDEL 5
#define REQUESTOVERHEAD 10
// #define SERVER_PORT_NUMBER 8080
#define INPUT_FILE "./IO_Files/batchRun.txt"
#define OUTPUT_FILE "./IO_Files/batchResponses.txt"
#define CSVOVERHEAD 20
#define MAXXMLSIZE 262800
#define MAXCSVSIZE 262800
int from_XML_to_out(char * responseXML, char * ans);
int to_xml_client(char * request, FILE * wp, char * ans);
int read_integer(char * line);
void init_client();
void start_client(int, char*);

//functions at client side
//converter function 
// int to_xml_client(char * request, FILE * wp, char * ans)
// //parser function
// char * from_XML_to_out(char * responseXML);