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
    // printf("\nReceived signal %d, closing all file descriptors...\n", signum);
    // printf("Subscriber_socket: %d", sub_socket);
    send(sub_socket, "exit", sizeof("exit"), 0);
    // for (int i = 0; i < fd_count; i++) {
    //     close(sub_socket[i]);
    //     printf("Closed fd %d\n", sub_socket[i]);
    // }
    // printf("Subscriber Socket closed from the sub code\n");
    close(sub_socket);
    exit(0);
}

// void add_fd_to_array(int fd) {
//     if (fd_count < BUFFER_SIZE) {
//         sub_socket[fd_count++] = fd;
//     } else {
//         //printf("Max file descriptors reached, cannot add more.\n");
//         int zcdasda;
//     }
// }

// void close_all_fds() {
//     for (int i = 0; i < fd_count; i++) {
//         close(sub_socket[i]);
//         printf("Closed fd %d\n", sub_socket[i]);
//     }
//     fd_count = 0;
// }

int main(int argc, char *argv[])
{

    signal(SIGINT, signal_handler);
    // close_all_fds();
    int sock;
    struct sockaddr_in server_addr;
    // char topic[BUFFER_SIZE];

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
    // add_fd_to_array(sock);
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
