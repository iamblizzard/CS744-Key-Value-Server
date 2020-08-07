#include "../common_headers/main_server.h"

Server_Configuration SF; 
// char ** request_XML_format;
int main(int argc, char** argv) {
	init_server();
	print_server_config(&SF);
	start_server();
	
	return 0;
}
void start_server() {
	//int fds[MAX_CLIENT_CONN];
	struct sockaddr_in addr;
	struct sockaddr_in client_sockaddr_in;
	int addrlen, curr_clients = 0, on = 1;
	int sockfd, temp;
	// struct ifreq ifr;
	struct pollfd pollfds[MAX_CLIENT_CONN];

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1) {
		printf("Error in socket creation: %s\n", strerror(errno));
	}

	/* making the socket reusable*/
	temp = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	if(temp < 0) {
		perror("setsockopt() failed");
	    close(sockfd);
	    exit(-1);
	}

	// /* making accept() as a non-blocking call */
	temp = ioctl(sockfd, FIONBIO, (char *)&on);
	if(temp < 0) {
		perror("ioctl() failed");
		close(sockfd);
		exit(-1);
	}

	memset(&addr, 0, sizeof (addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SF.port_number);
	addr.sin_addr.s_addr = inet_addr(SF.ip_address);
	if(bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error in socket binding: %s\n", strerror(errno));
		close(sockfd);
		exit(-1);
	}
	if(listen(sockfd, MAX_CLIENT_CONN) == -1) {
		printf("Error in socket listen: %s\n", strerror(errno));
		close(sockfd);
		exit(-1);
	}

	printf("Server Running ...\n");

	addrlen = sizeof(client_sockaddr_in);
	//why only sizeof(fds) here .should not it be sizeof( struct pollfd)i.e.
	//memset(pollfds, 0 , sizeof(struct pollfd));
	memset(pollfds, 0 , sizeof(pollfds));
	size_t can_close[MAX_CLIENT_CONN];
	// int req_xml_index = 0;
	char * request_xml = (char *)malloc(MAXXMLSIZE);
	while(1) {
		if(curr_clients < MAX_CLIENT_CONN) {
			memset(&client_sockaddr_in, 0, sizeof(client_sockaddr_in));
			while(1) {
				pollfds[curr_clients].fd = accept(sockfd, (struct sockaddr*)&client_sockaddr_in, &addrlen);
				if(pollfds[curr_clients].fd == -1) {
					if (errno != EWOULDBLOCK) {
						perror("error when accepting connection");
						close(sockfd);
						exit(1);
					}
					break;
				}
				else {
					pollfds[curr_clients].events = POLLIN;
					can_close[curr_clients] = 1;
					++curr_clients;
					printf("new client %d added: \n", curr_clients);
				}
			}
		}
		poll(pollfds, curr_clients, 1000);
		int total_clients = curr_clients, bytes_count;
		for(int i=0; i<total_clients; ++i) {
			if(pollfds[i].revents & POLLIN) {
				pollfds[i].revents = 0;
				memset(request_xml, 0, MAXXMLSIZE);
				bytes_count = recv(pollfds[i].fd, request_xml, MAXXMLSIZE, 0);
				if(bytes_count <= 0 && can_close[i]) {
					printf("client %d connection closed\n", i+1);
					close(pollfds[i].fd);
					pollfds[i].fd = -1;
					--curr_clients;
				} else {
					can_close[i] = 0;
					// printf("%s\n", request_XML_format[req_xml_index]);
				    // if(strlen(request_XML_format[req_xml_index]) == 0) {
					// 	printf("##################\n, %d, %d\n", bytes_count, request_XML_format[req_xml_index][bytes_count]);
					// 	// for(int i=0;i<10;++i)
					// 	// 	printf("%i ", (unsigned int)request_XML_format[req_xml_index]);
					// 	// printf("\n");
					// 	can_close[i] = 1;
					// 	continue;
					// }
					//seems correct
					char * temp = request_xml + bytes_count;
					while(strstr(request_xml, "</KVMessage>")== NULL){
						bytes_count = recv(pollfds[i].fd, temp, MAXXMLSIZE, 0);
						temp= temp + bytes_count;
						// printf("Hi read KVserver main\n");
					}
					add_work(request_xml, pollfds[i].fd);
					// req_xml_index = (req_xml_index+1) % MAXQUEUESIZE;
					// write(pollfds[i].fd, "gotcha...", 11);
					can_close[i] = 1;
				}
			}
		}	

		if(curr_clients < total_clients) {
			int j = 0;
			for(int i=0; i<total_clients; ++i) {
				if(pollfds[i].fd != -1) {
					if(i != j) {
						pollfds[j] = pollfds[i];
						can_close[j] = can_close[i];
					}
					++j;
				}
 			}
		}
	}	
}
void init_server(void){
	read_server_configration(&SF);
	init_cache(&SF);
	init_queue();
	init_KVstore();
	init_thread_pool(SF.thread_pool_size);
	
}
