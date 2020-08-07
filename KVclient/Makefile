# PORT = 8080
# SERVERIP = 127.0.0.1
CRYPTO = -lssl -lcrypto
THREAD = -lpthread
CC = gcc
CFLAGS =  -c -Wextra -Wno-unused-value 

all: client  
	 

client:  client_converter_parser.o KVclient.o
	$(CC) -o $@ $^

client_converter_parser.o:  client_converter_parser.c
	$(CC) $(CFLAGS)  $<
KVclient.o: KVclient.c
	$(CC) $(CFLAGS)  $<


clean:
	rm -f *.o client










# 4client_XML_converter.o: ./converter_parser/client_XML_converter.c 
# 	$(CC) $(CFLAGS) ./converter_parser/client_XML_converter.c
# client_parser.o: ./converter_parser/client_parser.c 
# 	$(CC) $(CFLAGS) ./converter_parser/client_parser.c

 
# server: KVserver_main.o read_server_config.o KVcache.o thread_pool.o
# 	$(CC) -o $@ $^ $(THREAD) $(CRYPTO)

# KVserver_main.o: ./KVserver_main/KVserver_main.c
# 	$(CC) $(CFLAGS)  $< $(THREAD) $(CRYPTO)
# read_server_config.o: ./read_server_config/read_server_config.c
# 	$(CC) $(CFLAGS)  $< $(THREAD) $(CRYPTO)
# KVcache.o: ./KVcache/KVcache.c
# 	$(CC) $(CFLAGS)  $< $(THREAD) $(CRYPTO)
# thread_pool.o: ./thread_pool/thread_pool.c
# 	$(CC) $(CFLAGS)  $< $(THREAD) $(CRYPTO)

# clean:
# 	rm -f *.o
    
