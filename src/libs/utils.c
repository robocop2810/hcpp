#include <utils.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <stdint.h>
#include <curl/curl.h>
#include <assert.h>

#define BUFFER_SIZE 1024


char *intToStr(int in){
	char *str = malloc(11 * sizeof(char)); // 11 -> 10 int 1 \0
	snprintf(str, 11,  "%d", in);
	return str;
}

char *responseBuilder(){
	char *responce = malloc(10000 * sizeof(char));
	strcpy(responce, "HTTP/1.1 200 OK\r\nServer: hcpp\r\nConnection: Keep-Alive\r\n"
                     "Keep-Alive: timeout=5, max=500\r\n"
                     "Content-Type: text/html; charset=UTF-8\r\n\r\n");
	#define html_out(x) strcat(responce, x)
	#define int(x) { \
		char *tmp = intToStr(x); \
		strcat(responce, tmp); \
		free(tmp); \
	}
	#include "../temp.h"
	#undef html_out
	#undef int
	return responce;
}

void convertTemplate(char *fileContents, char *headerName){
	uint8_t state = 0; 		    // 0 html mode 1 c mode
	FILE* temp = fopen(headerName, "w+"); // make new header file
					    
	fprintf(temp, "%s", "html_out(\"");
	do{
		if(*fileContents == '%'){
			if(state == 0){
				fprintf(temp, "\");\n");
				state = 1;
			}else{
				fprintf(temp, "\nhtml_out(\"");
				state = 0;
			}
		}else{
			if(state == 0) fprintf(temp, "\\x%x", *fileContents);
			if(state == 1) fprintf(temp, "%c", *fileContents);
		}	
		fileContents++;
	}while(*fileContents != '\0');
	fprintf(temp, "\");");
	fclose(temp);
	return;
}

char *readFileString(char *filename){
	assert(filename != NULL);
	FILE* file = fopen(filename, "r");
	assert(file != NULL && "ERROR: failed to open file");
	
	fseek(file, 0L, SEEK_END);
	long int fLength = ftell(file);
	rewind(file);
	
	printf("INFO: opened file %s, it has %ld chars. \n", filename, fLength);
	
	char *buffer = malloc(fLength * sizeof(char));
	assert(buffer != NULL && "Buy more ram dummy :3");
	
	size_t readSize = fread(buffer, 1, fLength, file);
	
	buffer[readSize] = '\0';	

	fclose(file);
	return buffer;
}


void rebuildYourself(char *executable, char *hcmlFile, int current_socket){
    if(strlen(hcmlFile) > 1000) return;

    char command[1024];
    snprintf(command, sizeof(command), "cc ./src/hcpp.c ./src/libs/*.c -o build/hcpp"); 		
    
    printf("Recompiling...\n");
    if(system(command) == 0){
        printf("INFO: Compiled successfully! Restarting...\n");
        
        close(current_socket);

        char *args[] = { executable, hcmlFile, "./src/temp.h", NULL };
        execvp(executable, args);
    } else {
        printf("ERROR: Compilation failed.\n");
    }
}


void handle_request(int client_socket) {
	char buffer[BUFFER_SIZE];
	recv(client_socket, buffer, sizeof(buffer), 0);

	char *response = responseBuilder();  
	send(client_socket, response, strlen(response), 0);
	free(response);
	close(client_socket);
}
