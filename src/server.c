#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <pthread.h>

#define MAX_USERS 2
#define MESSAGE_OF_THE_DAY "Welcome!\n" // must have trailing newline
#define LOCKS 2

int clients[MAX_USERS];
int users = 0;
int locks[LOCKS]; // Thread safety. 0 -> unlocked
pthread_t sockets[MAX_USERS];

void *slave(void *args);
int getlock(int *lock);
void unlock(int *lock);

int
main(int argc, char *argv[])
{
	int server_fd, client;
	int opt = 1;
	int port = (argc > 1) ? atoi(argv[1]) : 8199;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	int i;

	for (i = 0; i < LOCKS; i++) {
		locks[i] = 0; // init the locks
	}

	if (!(server_fd = socket(AF_INET, SOCK_STREAM, 0))) {
		perror("socket failed!");
	}

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
		perror("setsockopt"); 
		return EXIT_FAILURE;
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
		perror("bind fail!");
		return EXIT_FAILURE;
	}

	if (listen(server_fd, MAX_USERS) < 0) {
		perror("listen fail!");
		return EXIT_FAILURE;
	}

	while (1) {
		client = accept(server_fd, (struct sockaddr *) &address, (socklen_t *) &addrlen);
		while (getlock(&locks[0]));
		users++;
		unlock(&locks[0]);
		pthread_create(malloc(sizeof(pthread_t)), NULL, slave, &client); // scuffed mem leak, TODO must fix...
		printf("Current connected users: %d\n", users);
	}
	return EXIT_SUCCESS;
}

int
getlock(int *lock)
{
	if (!*lock) {
		*lock = 1;
		return 0;
	}
	return 1;
}

void
unlock(int *lock)
{
	*lock = 0;
}

void * 
slave(void *args)
{
	int client = *((int *) args); // the socket for this slave
	char *s = "Type: ";
	char buff[256];

	printf("%sServicing CID=%d\n", s, client);
	send(client, MESSAGE_OF_THE_DAY, strlen(MESSAGE_OF_THE_DAY), 0);
	send(client, s, strlen(s), 0);
	while (read(client, buff, 256)) {
		strtok(buff, "\n"); // remove trailing newline:
		printf("Mesg from client: %s\n", buff);
		strcat(buff, " <- message sent\n");
		send(client, buff, strlen(buff), 0);
		send(client, s, strlen(s), 0);
	}

	close(client);
	return 0;
}
