#include "client.h"
// char *Request_char;
char * Response_char;
char * Request_XML;
char * Response_XML;

int main(int argc, char * argv[]){
    if (argc <= 2) {
        printf("Unknown Error: Please follow the command line format i.e. ./object server_IP server_PORT\n");
        exit(1);
    }
    // char* portnumber;
    // char *IP;

    // ssize_t x = 0;
    // printf("Enter IP:\n");
    // assert(getline(&IP, &x, stdin)!=-1);
    // printf("Enter port number :\n");x = 0;

    // assert(getline(&portnumber, &x, stdin)!=-1);
    int server_port_number = read_integer(argv[2]);
    // IP[strlen(IP)-1] = 0;
    // int server_port_number= read_integer(portnumber);
    // printf("IP:%s\n", IP);
    // printf("PN:%d", server_port_number);
    // exit(0);
    init_client();
    
    start_client(server_port_number, argv[1]);    
    return 0;
}

int read_integer(char * line){
	int ans = 0;
	char c;
	for(int index = 0; (line[index]!= '\n'&&line[index]!= '\0'); ++index) {
        c = line[index];
        if((c >= '0') && (c <= '9')) {
            ans = ans * 10 + (c-'0');
        }
        else {
			//no need of freeing here . we are exiting
			printf("%s\n",line);
            printf("Unknown Error: Invalid configuration File\n");
            exit(1);
        }
    }
	return ans;
}

void init_client() {
    Response_XML = (char*)malloc(MAXXMLSIZE);
    Response_char = (char*)malloc(MAXXMLSIZE);
    Request_XML = (char*)malloc(MAXXMLSIZE);
    if(Response_XML == NULL || Response_char == NULL || Request_XML == NULL) {
        printf("Unknown Error: Unable to allocate memory in kvclient");
        exit(1);
    }
}

void start_client(int server_port_number, char* server_ip) {
    FILE *fp,*wp;
    wp = fopen(OUTPUT_FILE, "w+");
    fp = fopen(INPUT_FILE,"r");
    if(!fp) {
        perror("Unknown Error: Input Request File at client does not exist\n");
        exit(1);
    }
    if(!wp) {
        perror("Unknown Error: Unable to open a file for writing\n");
         exit(1);
    }
    // printf("Succesful makefile\n");
    // exit(0);
    
    char * request;
    size_t z = 0;

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port_number);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    printf("%d, %s\n", server_port_number, server_ip);
    // sockfd = 
    if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        printf("Network Error: Could not connect\n");
        exit(1);
    }
    // FILE * cp = fopen("testingwrite.txt", "w");
    int readd;
    int success;
    int max_CSV_size = MAXKEYSIZE +MAXVALUESIZE + CSVOVERHEAD;

    int length_data = MAXXMLSIZE;

    while(readd = getline(&request, &z, fp)!= -1){
        // if (readd > max_CSV_size) {
            // fprintf(wp,"%s", "Error: Too large CSV ROW (either oversized key or value). Trying next request\n");
            // continue;
        // }
        memset(Request_XML,0, length_data);
        success = to_xml_client(request,wp,Request_XML);
        if (success == 0){
            continue;
        }
        // printf("%d\n", strlen(Request_XML));
        // send(sockfd, Request_XML, strlen(Request_XML),0);
        length_data =strlen(Request_XML);
        int datasent = send(sockfd, Request_XML, length_data,0);
        printf("datasent = %d  length_data = %d\n", length_data,datasent);
        while(datasent<length_data){
            // printf("hi write from client \n");
            int temp = send(sockfd, Request_XML+datasent, length_data-datasent,0);
            datasent = datasent + temp;
        }
        memset(Response_XML,0,MAXXMLSIZE);
        int bytes_count = recv(sockfd, Response_XML,MAXXMLSIZE,0);
        // printf("data received from server\n");
        char * temp = Response_XML + bytes_count;
		while(strstr(Response_XML, "</KVMessage>")== NULL){
            bytes_count = recv(sockfd, temp, MAXXMLSIZE, 0);
            temp=  temp + bytes_count ;
            // printf("Hi read KVclient\n");
		}
		// printf("%s\n", Response_XML);
		// getchar();
        success = from_XML_to_out(Response_XML, Response_char);
        if (success == 0){
            fprintf(wp,"%s\n","XML Error: Received unparseable message");
            continue;
        }         
        fprintf(wp,"%s\n",Response_char);
        memset(request,0, readd); 
    }
    // fclose(cp);
    fclose(wp);
    fclose(fp);
    return;
    
}