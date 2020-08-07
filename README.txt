We will be submitting two folders.


1) KVClient:
	The folder has a makefile that will form an executable "client".

To start running the client give the following command:
	Make all
	./client <server ip> <server port> (here server IP is "127.0.0.1" and server port is "8080" by default, it can be changed from server’s configuration file)

2)KVServer:
	The folder that has a makefile that will form an executable "server".
We are reading cache sets, a number of entries, thread pool size, and server port number from config file "server.conf" present in the "config_file" folder.
Give the following command on terminal:
	Make all
	./server
3)All the disk data created by KVstore is in data directory.



Note:
	- To delete all executables run command: "make clean"
	
	- All the default server configuration parameters are in server.conf file. Their default values are:
	SETS=8 (respresents no. of sets in KVcache)
	ENTRIES=16 (respresents no. of entries in each set)
	PORT_NUMBER=8080
	THREAD_POOL_SIZE=10
	IP=127.0.0.1 (server's ip address)
	
	- There is a folder IO_Files in KVclient which contains the script to generate batchRun.txt and its corresponding expected Response file. Please keep your batchRun.txt in this folder to run and test KVclient.

	- MAX_CLIENT_CONN = 20 (max no. of KVclients which can be connected at a time. You may change it as per your expectations)
	  DATA_LIMIT_PER_FILE = 10 (We are creating multiple files using extendable hashing depending on the hash of the key to increase the concurreny among the threads and increase the read/write performance of the system. DATA_LIMIT_PER_FILE represets the max. no. of key-value pairs inside one file. It can be changed accordingly.)


References:

https://www.ibm.com/support/knowledgecenter/en/ssw_ibm_i_71/rzab6/poll.htm
