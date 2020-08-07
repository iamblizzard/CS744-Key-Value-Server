#include "../common_headers/main_server.h"
int read_integer(char * line, int startindex, int maxsize){
	int ans = 0;
	char c;
	for(int index = startindex; (line[index]!= '\n'&&line[index]!= '\0'); ++index) {
        c = line[index];
        if((c >= '0') && (c <= '9')) {
            ans = ans * 10 + (c-'0');
        }
        else {
			//no need of freeing here . we are exiting
			printf("%s\n",line);
            printf("Invalid configuration File\n");
            exit(1);
        }
    }
	return ans;


}
void read_server_configration(Server_Configuration* SF){
	FILE *fp;
    char *filename = "./config_file/server.conf";
    fp = fopen(filename, "r");
    if (fp == NULL) { printf("Unable to open  configuration file\n"); exit(1); }
    char *sets = NULL;
    char * entries = NULL;
	char * port = NULL;
	char * thread_pool = NULL;
    char * ip = NULL;
    size_t z = 0;
    ssize_t char_size_of_set = getline(&sets, &z, fp);
    z = 0;
    ssize_t char_size_of_entry  = getline(&entries, &z, fp);
	z = 0;
	ssize_t char_port_number = getline(&port, &z, fp);
	z = 0;
	ssize_t char_thread_pool_size = getline(&thread_pool, &z, fp);
    z = 0;
    getline(&ip, &z, fp);
    
    ip[strlen(ip)-1] =0;
    strcpy(SF->ip_address, ip + 3);
    fclose(fp);
    // strcpy()
    if (char_size_of_set <=5 || char_size_of_entry <= 8|| char_port_number<=12||char_thread_pool_size<=17) {
        free(sets);
        free(entries);
		free(port);
		free(thread_pool);
        printf("Invalid configuration File\n");
        exit(1);
    }

    SF->num_sets= read_integer(sets, 5, char_size_of_set);
	// printf("Hi\n");
    SF->num_entries= read_integer(entries, 8, char_size_of_entry);
	// printf()
    SF->port_number= read_integer(port, 12, char_port_number);
    SF->thread_pool_size= read_integer(thread_pool, 17, char_thread_pool_size);
	free(sets);
	free(entries);
	free(port);
	free(thread_pool);
    return;
}
void print_server_config(Server_Configuration* SF) {
    printf("Sets : %d\n", SF->num_sets);
    printf("Entries : %d\n", SF->num_entries);
	printf("Port Number :%d\n", SF-> port_number);
	printf("Thread pool size :%d\n", SF->thread_pool_size);
    printf("IP address:%s\n", SF->ip_address);
    return;
}