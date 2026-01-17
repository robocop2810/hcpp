#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <pthread.h>

#define PORT 8090

#include "./libs/utils.h"

struct stat pageStat;

typedef struct {
	int server_socket;
	struct sockaddr_in client_addr;
	socklen_t addr_len;
} ServerArgs;

typedef struct {
	char **argv;
	int server_socket;
} FileArgs;

void* server(void* arg) {
	ServerArgs* args = (ServerArgs*)arg;
	int client_socket;

	printf("Started thread for http server\n");
	for(;;) {
		client_socket = accept(args->server_socket, (struct sockaddr *)&args->client_addr, &args->addr_len);
		if (client_socket < 0) {
			continue; 
		}
		handle_request(client_socket);
	}
	return NULL;
}

void* checkFileChange(void* arg) {
	FileArgs* args = (FileArgs*)arg;
	printf("Started thread for file change detection\n");

	for(;;) {
		struct stat pageStatCheck;
		if (stat(args->argv[1], &pageStatCheck) == 0) {
			if (pageStat.st_mtime != pageStatCheck.st_mtime) {
				printf("Detected page change. Recompiling...\n");
				rebuildYourself(args->argv[0], args->argv[1], args->server_socket);
				//update pageStat to the new modification time to prevent infinite rebuild
				pageStat = pageStatCheck;
			}
		}
		usleep(500000); //Sleep for 500ms to prevent 100% cpu usage
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	printf("==========================================================\n\n");
	if (argc < 3) {
		fprintf(stderr, "Usage: %s <input.hcml> [on/off]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	bool runServer = (strncmp(argv[2], "on", 2) == 0);

	char* initialContent = readFileString(argv[1]);
	if (initialContent) {
		printf("INFO: Initial build of %s\n", argv[1]);
		convertTemplate(initialContent, "./src/temp.h");
		free(initialContent);
	}

	if (!runServer) {
		printf("Server mode is 'off'. Exiting.\n");
		return 0;
	}

	if (stat(argv[1], &pageStat) < 0) {
		perror("Initial stat failed");
		exit(EXIT_FAILURE);
	}

	int server_socket;
	struct sockaddr_in server_addr;

	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1) {
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}

	int opt = 1;
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		perror("setsockopt failed");
		exit(EXIT_FAILURE);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);

	if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}

	if (listen(server_socket, 10) < 0) {
		perror("Listen failed");
		exit(EXIT_FAILURE);
	}

	printf("Server listening on port %d\n", PORT);

	//prepare arguments for threads
	FileArgs *fArgs = malloc(sizeof(FileArgs));
	fArgs->argv = argv;
	fArgs->server_socket = server_socket;

	ServerArgs *sArgs = malloc(sizeof(ServerArgs));
	sArgs->server_socket = server_socket;
	sArgs->addr_len = sizeof(struct sockaddr_in);

	pthread_t FileThread, ServerThread;

	if (pthread_create(&FileThread, NULL, checkFileChange, fArgs) != 0) {
		perror("Failed to create file thread");
		return 1;
	}

	if (pthread_create(&ServerThread, NULL, server, sArgs) != 0) {
		perror("Failed to create server thread");
		return 1;
	}

	//wait for threads to finish
	pthread_join(FileThread, NULL);
	pthread_join(ServerThread, NULL);

	return 0;
}
