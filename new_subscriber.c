#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#define BUFFER_SIZE 1024
#define PORT 8080

int sub_socket;

void signal_handler(int signum)
{
    send(sub_socket, "exit", sizeof("exit"), 0);

    close(sub_socket);
    exit(0);
}

int main(int argc, char *argv[])
{

    signal(SIGINT, signal_handler);

    int sock;
    struct sockaddr_in server_addr;


    sock = socket(AF_INET, SOCK_STREAM, 0);
    // printf("Socket %d\n",sock);
    // add_fd_to_array(sock);
    sub_socket = sock;
    if (sock < 0)
    {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Failed to connect to server");
        close(sock);
        exit(EXIT_FAILURE);
    }
    send(sock, "subscriber", strlen("subscriber"), 0);

    // printf("Enter topic to subscribe: ");
    // fgets(topic, sizeof(topic), stdin);
    srand(15);
    sleep(rand() % 10);

    send(sock, argv[1], strlen(argv[1]), 0);

    // pid_t pid = getpid(); // Get the process ID
    // send(sock, &pid, sizeof(pid), 0);
    
    char buffer[BUFFER_SIZE];
    while (1)
    {   
        // pause();
        int n = recv(sock, buffer, sizeof(buffer), 0);
        if (n <= 0)
            break;
        buffer[n] = '\0';
        printf("S-%d : Received message: %s\n", getpid(), buffer);
    }

    close(sock);
    return 0;
}
