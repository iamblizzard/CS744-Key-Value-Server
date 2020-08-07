#include "../common_headers/main_server.h"

struct read_write_lock* ReadWritelock[16];
char* line[16];

void query_KVstore(Request * request, Response *response){
    int request_type = request->typereq;
    response->typereq = request_type;
    SHA256_CTX c;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Init(&c);
    SHA256_Update(&c, request->key, strlen(request->key));
    SHA256_Final(hash, &c);
    int flag, set, data_count, i, next_index;
    char file_name[40], set_name[20];
    set = (hash[0] & 0xF0) >> 4;
    // set = 0;
    sprintf(file_name, "./data/file%x.txt", set);
    FILE* fp = fopen(file_name, "r"), * wp;
    
    response->success = 0;
    response->key = request->key;
    switch(request_type){

        case TPUT:
            // Put request
            // first check if it is there meanwhile keep writing it in a temp file 
            // if it is found, copy rest of the data from  main file to the temp file(just set the key found variable to 1) and insert at the end. 
            write_lock_acquire(ReadWritelock[set]);
            flag = 1, i = 0;
            // first check whether data is present in any file
            while(fp != NULL) {
                get_KVstore(request, &response->success, fp, set, 1);
                fclose(fp);
                if(response->success == 1) {
                    break;
                }
                if(flag) {
                    next_index = hash[i] & 0x0F;
                    ++i;
                } else {
                    next_index = (hash[i] & 0xF0) >> 4;
                }
                flag ^= 1;
                file_name[strlen(file_name) - 4] = 0;
                sprintf(set_name, "%x", next_index);
                strcat(file_name, set_name);
                strcat(file_name, ".txt");
                fp = fopen(file_name, "r");
            }

            if(response->success == 1) {
                insert_KVstore(request, response, file_name, set);
                read_write_lock_release(ReadWritelock[set]);
                return;
            }

            // data not found, so write it in any file having space
            sprintf(file_name, "./data/file%x.txt", set);
            fp = fopen(file_name, "r");
            flag = 1, i = 0;
            response->success = 0;
            while(1) {
                // this means this is new file, create it and write PUT in it
                if(fp == NULL) {
                    break;
                }
                data_count = get_KVstore(request, &response->success, fp, set, 1);
                fclose(fp);
                // data_count < DATA_LIMIT_PER_FILE means data is less, write in it
                if(data_count < DATA_LIMIT_PER_FILE) {
                    break;
                }
                if(flag) {
                    next_index = hash[i] & 0x0F;
                    ++i;
                } else {
                    next_index = (hash[i] & 0xF0) >> 4;
                }
                flag ^= 1;
                file_name[strlen(file_name) - 4] = 0;
                sprintf(set_name, "%x", next_index);
                strcat(file_name, set_name);
                strcat(file_name, ".txt");
                fp = fopen(file_name, "r");
            }
            insert_KVstore(request, response, file_name, set);
            read_write_lock_release(ReadWritelock[set]);
            
            break;
        case TGET:
            //Get request : if you find the key return success or return error
            if(fp == NULL) {
                response->val = request->val;
                return;    
            }
            flag = 1, i = 0;
            read_lock_acquire(ReadWritelock[set]);
            while (fp != NULL) {
                data_count = get_KVstore(request, &response->success, fp, set, 0);
                fclose(fp);
                if(response->success == 1) {
                    break;
                }
                if(flag) {
                    next_index = hash[i] & 0x0F;
                    ++i;
                } else {
                    next_index = (hash[i] & 0xF0) >> 4;
                }
                flag ^= 1;
                file_name[strlen(file_name) - 4] = 0;
                sprintf(set_name, "%x", next_index);
                strcat(file_name, set_name);
                strcat(file_name, ".txt");
                fp = fopen(file_name, "r");
            }
            response->val = request->val;
            read_write_lock_release(ReadWritelock[set]);
            // strcpy(request-> key, "" );
            // strcpy(request->val,"");
            break;
        case TDEL:
            //Del request:  first check if it is there meanwhile keep writing it in a temp file .
            //  if it is found, skip the key value pair and write rest of it and return.
            write_lock_acquire(ReadWritelock[set]);
            flag = 1, i = 0;
            // first check whether data is present in any file
            while(fp != NULL) {
                get_KVstore(request, &response->success, fp, set, 1);
                fclose(fp);
                if(response->success == 1) {
                    break;
                }
                if(flag) {
                    next_index = hash[i] & 0x0F;
                    ++i;
                } else {
                    next_index = (hash[i] & 0xF0) >> 4;
                }
                flag ^= 1;
                file_name[strlen(file_name) - 4] = 0;
                sprintf(set_name, "%x", next_index);
                strcat(file_name, set_name);
                strcat(file_name, ".txt");
                fp = fopen(file_name, "r");
            }
            if(response->success == 1) {
                delete_KVstore(request, response, file_name, set);
            }
            read_write_lock_release(ReadWritelock[set]);
            
            break;

        default: break;
    }
}

int get_KVstore(Request * request, int * success_pointer, FILE* fp_main, int id, int internal_use) {
    ssize_t read;
    size_t len = MAXVALUESIZE + 20;
    if(fp_main == NULL) {
        printf("file read error.\n");
        exit(1);
    }
    size_t key_found = 0;
    int data_count = 0;
    while((read = getline(&line[id], &len, fp_main)) != -1) {
        if(strncmp(line[id], "<Key>", 5) == 0) {
            ++data_count;
            if(key_found == 1) {
                continue;
            }
            if(((strstr(line[id], "</Key>") - line[id]) - 5) == strlen(request->key) && 
                    strncmp(line[id] + 5, request->key, strlen(request->key)) == 0) {
                key_found = 1;
                if(internal_use == 1) {
                    continue;
                }
                read = getline(&line[id], &len, fp_main);
                strncpy(request->val, line[id] + 7, read - 16);
                request->val[read - 16] = '\0';
            }
        }
    }
    if(key_found == 1) {
        *(success_pointer) = 1;
    }
    return data_count;
}


void delete_KVstore(Request *request, Response * response, char main_file_name[40], int id){

    char temp_file_name[40];
    sprintf(temp_file_name, main_file_name);
    temp_file_name[strlen(temp_file_name) - 4] = 0;
    strcat(temp_file_name, "-temp");
    strcat(temp_file_name, ".txt");

    FILE* fp_main = fopen(main_file_name, "r+");
	FILE* fp_temp = fopen(temp_file_name, "w+");
	if(fp_temp == NULL || fp_main == NULL) {
		printf("Unknown Error: file read error.\n");
		exit(1);
	}
	size_t key_found = 0;
    int read;
    size_t len = MAXVALUESIZE+20;
	while((read = getline(&line[id], &len, fp_main)) != -1) {
		if(key_found == 0 && strncmp(line[id], "<KVPair>", 8) == 0) {
			read = getline(&line[id], &len, fp_main);
			if(((strstr(line[id], "</Key>") - line[id]) - 5) == strlen(request->key) && 
					strncmp(line[id] + 5, request->key, strlen(request->key)) == 0) {
				key_found = 1;
				getline(&line[id], &len, fp_main);
				getline(&line[id], &len, fp_main);
				continue;
			} else {
				fprintf(fp_temp, "<KVPair>\n");
			}
		}
		fprintf(fp_temp, "%s", line[id]);
	}
	if(key_found == 1) {
		response->success = 1;
	}
	fclose(fp_temp);
	fclose(fp_main);
	if(remove(main_file_name) != 0) {
		printf("Unknown Error: main file remove error...\n");
		exit(1);
	}
	if(rename(temp_file_name, main_file_name) != 0) {
		printf("Unknown Error: temp file rename error...\n");
		exit(1);
	}
}

void insert_KVstore(Request *request, Response * response, char main_file_name[40], int id) {
    
    char temp_file_name[40];
    sprintf(temp_file_name, main_file_name);
    temp_file_name[strlen(temp_file_name) - 4] = 0;
    strcat(temp_file_name, "-temp");
    strcat(temp_file_name, ".txt");
    
    FILE* fp_temp = fopen(temp_file_name, "w+");
    FILE* fp_main = fopen(main_file_name, "r");
    // printf("%d\n", id);
    if(fp_main == NULL) {
		fp_main = fopen(main_file_name, "w+");
		fprintf(fp_main, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<KVStore>\n</KVStore>\n");
		fclose(fp_main);
        // exit(0);
		fp_main = fopen(main_file_name, "r");
	}
	if(fp_temp == NULL) {
		printf("file read error.\n");
		exit(1);
	}
	size_t key_found = 0;
    size_t len = MAXVALUESIZE+20;
    int read;
	while((read = getline(&line[id], &len, fp_main)) != -1) {
		if(key_found == 0 && strncmp(line[id], "</KVStore>", 10) == 0) {
			break;
		}
		if(key_found == 0 && strncmp(line[id], "<Key>", 5) == 0) {
			if(((strstr(line[id], "</Key>") - line[id]) - 5) == strlen(request->key) && 
					strncmp(line[id] + 5, request->key, strlen(request->key)) == 0) {
				key_found = 1;
				fprintf(fp_temp, "%s", line[id]);
				fprintf(fp_temp, "<Value>%s</Value>\n", request->val);
				getline(&line[id], &len, fp_main);
				continue;
			}
		}
		fprintf(fp_temp, "%s", line[id]);
	}
	if(key_found == 0) {
		fprintf(fp_temp, "<KVPair>\n<Key>%s</Key>\n", request->key);
		fprintf(fp_temp, "<Value>%s</Value>\n</KVPair>\n</KVStore>\n", request->val);
	}
	fclose(fp_temp);
	fclose(fp_main);
	if(remove(main_file_name) != 0) {
		printf("main file remove error...\n");
		exit(1);
	}
	if(rename(temp_file_name, main_file_name) != 0) {
		printf("temp file rename error...\n");
		exit(1);
	}
	response->success = 1;
    return;
}

void init_KVstore(void) {
    for(int i=0; i<16; ++i) {
        ReadWritelock[i] = (struct read_write_lock*)malloc(sizeof(struct read_write_lock));
        line[i] = (char*)malloc(MAXVALUESIZE+20);
        read_write_lock_init(ReadWritelock[i]);
    }
}
