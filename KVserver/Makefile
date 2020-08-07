
CRYPTO = -lssl -lcrypto
THREAD = -lpthread
CC = gcc
CFLAGS =  -c -Wall -Wno-unused-value -g

all: server

run: all
	./server

server: KVserver_main.o read_server_config.o KVcache.o thread_pool.o server_converter_parser.o KVstore.o
	$(CC) -o $@ $^ $(THREAD) $(CRYPTO)

KVserver_main.o: ./KVserver_main/KVserver_main.c server_converter_parser.o
	$(CC) $(CFLAGS)  $< $(THREAD) $(CRYPTO)
read_server_config.o: ./read_server_config/read_server_config.c
	$(CC) $(CFLAGS)  $< $(THREAD) $(CRYPTO)
KVcache.o: ./KVcache/KVcache.c server_converter_parser.o KVstore.o
	$(CC) $(CFLAGS)  $< $(THREAD) $(CRYPTO)
thread_pool.o: ./thread_pool/thread_pool.c  server_converter_parser.o
	$(CC) $(CFLAGS)  $< $(THREAD) $(CRYPTO)
server_converter_parser.o: ./server_converter_parser/server_converter_parser.c
	$(CC) $(CFLAGS)  $< $(THREAD) $(CRYPTO)
KVstore.o: ./KVstore/KVstore.c
	mkdir -p ./data
	$(CC) $(CFLAGS)  $< $(THREAD) $(CRYPTO)

clean:
	rm -f *.o server 
