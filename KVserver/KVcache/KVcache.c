#include "../common_headers/main_server.h"

cache_entry ** Cache;
char** second_chance_list;
int *round_robin_pointer;
int cache_num_sets;
int cache_num_entries;
struct read_write_lock* Cache_Locks;

void print_cache_entry(cache_entry* CF) {
    printf("Key : %s\n", CF->key);
    printf("Value : %s\n", CF->val);
    return;
}
void print_cache(void){
    for (int i = 0; i < cache_num_sets; i++){
        printf("Set Number = %d\n", i);
        for(int j = 0; j < cache_num_entries; j++){
            printf("Set Entry = %d\n",j);
            printf("%s\n", Cache[i][j].key);
            printf("%s\n", Cache[i][j].val);
        }
    }
    return;
}
int getSetId(char* key, int num_sets) {
    SHA256_CTX c;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Init(&c);
    SHA256_Update(&c, key, strlen(key));
    SHA256_Final(hash, &c);
    int setid = 0;
    int i = 0;
    while((setid < num_sets) && (i < SHA256_DIGEST_LENGTH)) {
        int x = (int)hash[i];
        setid = (setid << 8) | x;
        ++i;
    }
    if(setid >= num_sets) {
        setid = setid % num_sets;
    }
    return setid;
}
void write_entry_cache(Request *req, int set_index){
    int curr_pointer = *round_robin_pointer;
    for(int i = 0; i < cache_num_entries; i++) {
        if(second_chance_list[set_index][i] == EMPTY) {
            strcpy(Cache[set_index][i].val,req->val);
            strcpy(Cache[set_index][i].key,req->key);
            second_chance_list[set_index][i] = SECOND_CHANCE_GIVEN;
            round_robin_pointer[set_index] = (i + 1) % cache_num_entries;
            return;
        }
    }
    while(1) {
       if(second_chance_list[set_index][curr_pointer] == SECOND_CHANCE_TAKEN) {
            second_chance_list[set_index][curr_pointer] = SECOND_CHANCE_TAKEN;
            strcpy(Cache[set_index][curr_pointer].val,req->val);
            strcpy(Cache[set_index][curr_pointer].key,req->key);
            round_robin_pointer[set_index] = (curr_pointer + 1) % cache_num_entries;
            break;
        } else {
            second_chance_list[set_index][curr_pointer] = SECOND_CHANCE_TAKEN;
            curr_pointer += 1;
            curr_pointer %= cache_num_entries;
        }
    }
    return;
} 
void query_KVcache(Request *req, Response *response){
    response->success = 0;
    int set_index = getSetId(req->key, cache_num_sets);
    int cache_hit = 0;
    if(req->typereq == TPUT) {//put request//write request
        write_lock_acquire(Cache_Locks+set_index);
        for(int i = 0; i < cache_num_entries; ++i) {
            int keycmp = strcmp(Cache[set_index][i].key, req->key);
            if (keycmp == 0) {
                second_chance_list[set_index][i] = SECOND_CHANCE_GIVEN;
                strcpy(Cache[set_index][i].key,req->key);
                strcpy(Cache[set_index][i].val,req->val);
                response->success = 1;
                cache_hit = 1;
                round_robin_pointer[set_index] = (i+1)%cache_num_entries;
                break;
            }            
        }
        if(cache_hit == 0){
            write_entry_cache(req, set_index);
        }
        response-> success = 1;
        query_KVstore(req, response);
        read_write_lock_release(Cache_Locks+set_index);
    } else if(req->typereq == TGET) {//get request//red request
        response->success = 0;
        read_lock_acquire(Cache_Locks+set_index);
        for(int i = 0; i < cache_num_entries; ++i) {
            if(strcmp(Cache[set_index][i].key, req->key) == 0) {
                second_chance_list[set_index][i] = SECOND_CHANCE_GIVEN;
                strcpy(response->key, Cache[set_index][i].key);
                strcpy(response->val, Cache[set_index][i].val);
                response->success = 1;
                read_write_lock_release(Cache_Locks+set_index);
                return;
            }
        }
        query_KVstore(req,response);
        if (response->success == 1) {
            write_entry_cache(req,set_index);
        }
        read_write_lock_release(Cache_Locks+set_index);
    } else {//TDEL //write request
        write_lock_acquire(Cache_Locks+set_index);
        int checkdel = 0;
        for(int i = 0; i < cache_num_entries; ++i) {
            if(strcmp(Cache[set_index][i].key,req->key) == 0) {
                second_chance_list[set_index][i] = EMPTY;
                strcpy(Cache[set_index][i].key,"");
                strcpy(Cache[set_index][i].val,"");
                checkdel = 1;//successfel delete from cache 
                response->success = 1;
                break;
            }
        }
        query_KVstore(req,response);
        if(checkdel &&  response->success != 1){
             printf("Unknown Error: problem with delete response\n"); exit(1);
         }
        read_write_lock_release(Cache_Locks+set_index);
    }
    // print_cache();
    return;
}

void print_cache_configuration(void) {
    printf("Num_sets: %d\n", cache_num_sets);
    printf("Entries : %d\n", cache_num_entries);
    return;
}

void init_cache(Server_Configuration *SF) {
    cache_num_sets = SF->num_sets;
    cache_num_entries = SF-> num_entries;
    Cache_Locks = (struct read_write_lock *)malloc(cache_num_sets * sizeof(struct read_write_lock));
    for (int i = 0; i < cache_num_sets; i++){
        read_write_lock_init(Cache_Locks+i);
    }
    Cache = (cache_entry**)malloc(cache_num_sets * sizeof(cache_entry*));
    if (Cache == NULL) {
        printf("Unknown Error: Unable to allocate memory for KVCache\n"); exit(1);
    }
    for(int i = 0; i < cache_num_sets; i++) {
        Cache[i] = (cache_entry *)malloc(cache_num_entries * sizeof(cache_entry));
        if (Cache[i] == NULL) {
            printf("Unknown Error: Unable to allocate memory for KVCache\n"); exit(1);
        }
        for(int j = 0; j < cache_num_entries; j++){
            strcpy(Cache[i][j].val, "");
            strcpy(Cache[i][j].key, "");
        }
    }
    //initializing clock replacement algorithm parameters
    second_chance_list = (char **)malloc(cache_num_sets * sizeof(char *));
    if(second_chance_list == NULL) {
        printf("Unknown Error: Unable to allocate memory for KVCache\n"); exit(1);
    }
    for(int i = 0; i < cache_num_sets; i++) {
        second_chance_list[i] = (char *)malloc(cache_num_entries);
        if(second_chance_list[i] == NULL) {
            printf("Unknown Error: Unable to allocate memory for KVCache\n"); exit(1);
        }
        for(int j = 0; j < cache_num_entries; j++) {
            second_chance_list[i][j] = EMPTY;
        }   
    }
    // initializing round robin pointer
    round_robin_pointer = (int *)calloc(cache_num_sets, sizeof(int));
    if (round_robin_pointer == NULL){
        printf("Unknown Error: Unable to allocate memory for KVCache\n"); exit(1);
    }
    // print_cache_configuration();
    return ;
}
void destroy_cache(void){
    //free round robin pointer
    free(round_robin_pointer);
    // free second chance list
    for(int i = 0; i < cache_num_sets; i++){
        free(second_chance_list[i]);
    }
    free(second_chance_list);
    //freeing Cache
     for(int i = 0; i < cache_num_sets; i++){
        free(Cache[i]);
    }
    free(Cache);
    return;
}


// int main() {
//     Cache_configuration CF;
//     read_cache_configration(&CF);
//     printf("SETS = %u\n", CF.num_sets);
//     printf("ENTRIES = %u\n", CF.num_entries);
//     char** second_chance_list = init_second_chance_list(CF.num_sets, CF.num_entries);
//     int* round_robin_pointer = (int *)calloc(CF.num_sets, sizeof(int));
//     cache_entry** Cache = init_cache(CF.num_sets, CF.num_entries);
//     // cache_entry* cache_test_cases = (cache_entry *)malloc(sizeof(cache_entry) * TESTCASES);
//     Request * reqarray  = (Request *)malloc(TESTCASES*sizeof(Request));
//     char sample_key[MAXKEYSIZE] = "vnkcbs";
//     char  space_char[10] = "M";
//     char sample_value[MAXVALUESIZE] = "To live is to suffer and to survive is to find some meaning in the suffering.";
//     char n_char[10] = "N";
//     for (int i = 0; i < TESTCASES; i+=3){
//         strcat(sample_key, space_char);
//         strcat(sample_value,n_char);
//         strcpy (reqarray[i].key,sample_key);
//         strcpy (reqarray[i].val, sample_value);
//         strcpy (reqarray[i+1].key,sample_key);
//         strcpy (reqarray[i+1].val, sample_value);
//         strcpy (reqarray[i+2].key,sample_key);
//         strcpy (reqarray[i+2].val, sample_value);
//         reqarray[i].typereq = TPUT;
//         reqarray[i+1].typereq = TGET;
//         reqarray[i+2].typereq = TDEL; 
//     }
//     Response R;
//     printf("Enter requests \n");
//     int k = 0;
//     while(k <= 100){
//         int request_index = 0;
//         scanf("%d", &request_index);
//         if (request_index>= 0 && request_index < TESTCASES){
//             read_cache(reqarray+request_index,&R, &CF,Cache,second_chance_list, round_robin_pointer);
//         }
//         printf("Do you want to print cache state?(1 for yes, 0 for No \n");
//         int c;
//         scanf("%d", &c);
//         if (c==1){
//             print_cache(Cache,CF.num_sets, CF.num_entries);
//         }



//         k++;

//     }

//     return 0;
// }