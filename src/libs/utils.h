#ifndef UTILS_H
#define UTILS_H



char *readFileString(char*);
void convertTemplate(char *fileContents, char *headerName);
char *responseBuilder();
char *intToStr(int in);
void rebuildYourself(char *executable, char *hcmlFile, int current_socket);
void handle_request(int client_socket);

#endif
