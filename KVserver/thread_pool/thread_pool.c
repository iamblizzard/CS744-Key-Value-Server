#include "../common_headers/main_server.h"
thread_manager* manager;
Queue* Q;
void init_thread_pool(int thread_count) {
	manager = (thread_manager*)malloc(sizeof(thread_manager));
	manager->thread_count = thread_count;
	manager->threads = (pthread_t*)malloc(thread_count * sizeof(pthread_t));
	for(int i=0; i<thread_count; ++i) {
		pthread_create(&manager->threads[i], NULL, (void*)do_work, NULL); 
		// pthread_detach(manager->threads[i]); 
	}
}
void init_queue(void){
	Q = (Queue*)malloc(sizeof(Queue));
	Q->request_XML_queue = (Queue_Item **)malloc(MAXQUEUESIZE*sizeof(Queue_Item *));
	for(int i = 0; i < MAXQUEUESIZE; ++i) {
		Q->request_XML_queue[i] = (Queue_Item*)malloc(sizeof(Queue_Item));
	}
	Q->next_out =-1;
	Q->next_in=-1;
	lock_init(&(Q->queue_lock));
	cond_init(&(Q->queue_empty));
	cond_init(&(Q->queue_full));
	return;
}
void add_work(char  * request_XML, int client_fd){
	
	lock_acquire(&Q->queue_lock);
	Q->next_in = (Q->next_in +1)%MAXQUEUESIZE;
	while( Q->next_in== Q->next_out){
		cond_wait(&(Q->queue_full), &(Q->queue_lock));
	}
	printf("Queue_Item  Added\n");
	strcpy(Q->request_XML_queue[Q->next_in]->request_XML ,request_XML);
	Q->request_XML_queue[Q->next_in]->client_fd = client_fd;
	cond_signal(&(Q->queue_empty), &(Q->queue_lock));
	lock_release(&(Q->queue_lock));
	return;
}
 void *  do_work(void* arg){
	Request * request = (Request *)malloc(sizeof (Request));
	if (request == NULL){
		printf("Unknown Error: in memory allocation in do_work\n"); exit(1);
	}
	
	Response * response = (Response *)malloc(sizeof(Response));
	if (response == NULL){
		printf("Unknown Error: in memory allocation in do_work\n"); exit(1);
	}
	char *responseXML = (char *)malloc(MAXXMLSIZE);
	
	if (responseXML== NULL){
		printf("Unknow Error: in memory allocation in do_work\n"); exit(1);
	}
	Queue_Item * current;
	while(1){
		current = dequeue();
		// memset(current->request_XML, 0, MAXXMLSIZE);
		int success = from_clientXML_to_Request(current->request_XML, request);
		if (success == 0){
			printf("Unkown Error: Thread1 IO Error\n");
			printf("%s\n", current->request_XML);
			//exit(1);
		}
		response->key = request->key;
		response-> val = request->val;
		response->typereq = request->typereq;
		query_KVcache(request, response);
		
		success = ServerResponse_to_XML_server(response, responseXML );
		if (success == 0){
			printf("Unkown Error: Thread2 IO Error\n");
			exit(1);
		}
		// printf("%s\n", responseXML);
		// getchar();
		int length_data =strlen(responseXML);
        int datasent = send(current->client_fd, responseXML, length_data,0);
        printf("datasent = %d  length_data = %d\n", length_data,datasent);
        while(datasent<length_data){
            printf("hi write from thread pool\n");
            int temp = send(current->client_fd, responseXML+datasent, length_data-datasent,0);
            datasent = datasent + temp;
        }
		// getchar();
		// send(current->client_fd,responseXML, strlen(responseXML),0);
	}
	return NULL;
}

Queue_Item * dequeue(void){
	Queue_Item * ans;
	lock_acquire(&Q->queue_lock);
	while (Q->next_out == Q->next_in)
	 	cond_wait(&(Q->queue_empty), &(Q->queue_lock));
	Q->next_out = (Q->next_out + 1) % MAXQUEUESIZE;
	ans = Q->request_XML_queue[Q->next_out];
	cond_signal(&(Q->queue_full), &(Q->queue_lock));
	lock_release(&(Q->queue_lock));
	return ans;
}

void printresponse(Response *response){
	int typereq = response->typereq;
	if (typereq == TPUT){
		printf("%d\n",response->typereq);
		printf("%d\n", response->success);
	}
	else if (typereq == TGET){
		printf("%d\n",response->typereq);
		if (response->success){
			printf("%s\n", response->key);
			printf("%s\n", response->val);
		}
		else{
			printf("Failure on TGET \n");
			printf("%d\n", response->success);
		}		
	}
	else{	printf("Delete Request\n");
			printf("%d\n", response->typereq);
			printf("%d\n", response->success);
	}
	return;
}

	// printf("thread_pool initialized = %d \n",manager->thread_count);

// typedef struct Queue_Item{
//     char* request_XML;
// 	int client_fd;
// }Queue_Item;

// typedef struct Queue{
// 	Queue_Item ** request_XML_queue;
//     struct lock queue_lock;
//     struct condition queue_empty;
//     struct condition queue_full;
//     int next_out;
// 	int next_in;
//  }Queue;
