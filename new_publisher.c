#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define PORT 8080

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_addr;
    char topic[BUFFER_SIZE];
    char message[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to connect to server");
        close(sock);
        exit(EXIT_FAILURE);
    }

    send(sock, "publisher", strlen("publisher"), 0);

    printf("Enter topic to publish: ");
    //fgets(topic, sizeof(topic), stdin);
    send(sock, argv[], strlen(topic), 0);

    printf("Enter message to publish: ");
    //fgets(message, sizeof(message), stdin);
    send(sock, message, strlen(message), 0);

    close(sock);
    return 0;
}
