#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define PORT 8090


#include "./libs/utils.h"

struct stat pageStat;

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input.hcml> <output.h>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* initialContent = readFileString(argv[1]);
    if (initialContent) {
        printf("INFO: Initial build of %s\n", argv[1]);
        convertTemplate(initialContent, argv[2]);
        free(initialContent);
    }

    if (stat(argv[1], &pageStat) < 0) {
        perror("Initial stat failed");
        exit(EXIT_FAILURE);
    }

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

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

    while (1) {
        struct stat pageStatCheck;
        
        if (stat(argv[1], &pageStatCheck) == 0) {
            if (pageStat.st_mtime != pageStatCheck.st_mtime) {
                printf("Detected page change. Recompiling...\n");
                rebuildYourself(argv[0], argv[1], server_socket); 
            }
        }

        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0) {
            continue; 
        }

        handle_request(client_socket);
    }

    close(server_socket);
    return 0;
}
