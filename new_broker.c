#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sqlite3.h>

#define MAX_CLIENTS 4096
#define BUFFER_SIZE 1024
#define PORT 8080

// Define a structure to hold subscriber information
typedef struct
{
    int socket;
    char topic[BUFFER_SIZE];
} Subscriber;

Subscriber subscribers[MAX_CLIENTS];
int subscriber_count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

sqlite3 *db; // SQLite database connection

// Function to insert new subscribers into the database
void reset_subscribers(){
    memset(subscribers , 0 ,sizeof(subscribers));
    printf("Subscriber list Reset! \n ");
}
void remove_subscriber(int socket)
{
    close(socket);
    pthread_mutex_lock(&mutex);
    subscriber_count--;
    if(subscriber_count==0){
        reset_subscribers();
    }
    pthread_mutex_unlock(&mutex);
}

void insert_subscriber(int socket, const char *topic)
{
    char *err_msg = 0;
    char sql[BUFFER_SIZE];
    snprintf(sql, sizeof(sql), "INSERT INTO subscribers (socket, topic) VALUES (%d, '%s');", socket, topic);

    if (sqlite3_exec(db, sql, 0, 0, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
}
// Function to send messages to subscribers
void send_message_to_subscribers(const char *topic, const char *message)
{
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < subscriber_count; i++)
    {
        if (strcmp(subscribers[i].topic, topic) == 0)
        {
            send(subscribers[i].socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&mutex);
}

// Thread to handle subscriber connections
void *handle_subscriber(void *arg)
{
    int new_socket = *(int *)arg;
    char topic[BUFFER_SIZE] = {0};

    if (recv(new_socket, topic, sizeof(topic), 0) <= 0)
    {
        perror("Failed to receive topic from subscriber");
        close(new_socket);
        return NULL;
    }
    topic[strcspn(topic, "\n")] = 0; // Remove newline character

    // Register subscriber in the database
    insert_subscriber(new_socket, topic);

    pthread_mutex_lock(&mutex);
    if (subscriber_count < MAX_CLIENTS)
    {
        subscribers[subscriber_count].socket = new_socket;
        strncpy(subscribers[subscriber_count].topic, topic, sizeof(subscribers[subscriber_count].topic) - 1);
        subscriber_count++;
    }
    else
    {
        printf("Max subscribers reached. Connection refused.\n");
        close(new_socket);
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    pthread_mutex_unlock(&mutex);

    char confirmation[] = "Subscribed successfully\n";
    send(new_socket, confirmation, strlen(confirmation), 0);

    char buffer[BUFFER_SIZE];
    // printf("Before");
    while (recv(new_socket, buffer, sizeof(buffer), 0) != sizeof("exit"))
        ;
    printf("Closing Subscriber Socket: %d\n", new_socket);
    remove_subscriber(new_socket);
    return NULL;
}

// Thread to handle publisher connections
void *handle_publisher(void *arg)
{
    clock_t start_time, end_time; ///////////////////////////////to check the runtime of this function
    double time_taken= 0;
    start_time = clock();

    int new_socket = *(int *)arg;
    char topic[BUFFER_SIZE] = {0};
    char message[BUFFER_SIZE] = {0};

    if (recv(new_socket, topic, sizeof(topic), 0) <= 0)
    {
        perror("Failed to receive topic from publisher");
        close(new_socket);
        return NULL;
    }
    topic[strcspn(topic, "\n")] = 0; // Remove newline character

    if (recv(new_socket, message, sizeof(message), 0) <= 0)
    {
        perror("Failed to receive message from publisher");
        close(new_socket);
        return NULL;
    }
    message[strcspn(message, "\n")] = 0; // Remove newline character

    printf("Message published to topic '%s': %s\n", topic, message);
    send_message_to_subscribers(topic, message);

    end_time = clock();
    time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Function runtime: %f seconds\n", time_taken);
    
    close(new_socket);
    return NULL;
}

void *handle_connection(void *sfd)
{
    int sockfd = *(int *)sfd;
    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int new_socket = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
        printf("New socket connecion in broker: %d\n", new_socket);

        if (new_socket < 0)
        {
            perror("Failed to accept connection");
            continue;
        }

        char role[BUFFER_SIZE] = {0};
        recv(new_socket, role, sizeof(role), 0);

        if (strcmp(role, "subscriber") == 0)
        {
            pthread_t thread;
            pthread_create(&thread, NULL, handle_subscriber, (void *)&new_socket);
        }
        else if (strcmp(role, "publisher") == 0)
        {
            pthread_t thread;
            pthread_create(&thread, NULL, handle_publisher, (void *)&new_socket);
        }
        else
        {
            printf("Unknown role: %s\n", role);
            close(new_socket);
        }
    }
}

int main()
{
    sqlite3_open("broker.db", &db); // Open the SQLite database

    // Create tables if not exist
    char *err_msg = 0;
    char *create_sql = "CREATE TABLE IF NOT EXISTS subscribers (id INTEGER PRIMARY KEY, socket INTEGER, topic TEXT);";
    sqlite3_exec(db, create_sql, 0, 0, &err_msg);

    int server_socket;
    struct sockaddr_in server_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Failed to bind socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) < 0)
    {
        perror("Failed to listen on socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Broker running on port %d. Waiting for connections...\n", PORT);

    int num_threads = 100;
    pthread_t th[num_threads];

    for (int x = 0; x < num_threads; x++)
    {
        pthread_create(&th[x], NULL, handle_connection, (void *)&server_socket);
    }
    printf("Connection - ThreadPoolCreated\n");

    // while (1) {
    //     struct sockaddr_in client_addr;
    //     socklen_t client_len = sizeof(client_addr);
    //     int new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);

    //     if (new_socket < 0) {
    //         perror("Failed to accept connection");
    //         continue;
    //     }

    //     char role[BUFFER_SIZE] = {0};
    //     recv(new_socket, role, sizeof(role), 0);

    //     if (strcmp(role, "subscriber") == 0) {
    //         pthread_t thread;
    //         pthread_create(&thread, NULL, handle_subscriber, (void *)&new_socket);
    //     } else if (strcmp(role, "publisher") == 0) {
    //         pthread_t thread;
    //         pthread_create(&thread, NULL, handle_publisher, (void *)&new_socket);
    //     } else {
    //         printf("Unknown role: %s\n", role);
    //         close(new_socket);
    //     }
    // }

    for (int x = 0; x < num_threads; x++)
    {
        pthread_join(th[x], NULL);
    }
    printf("Connection - ThreadPoolDestroyed\n");

    close(server_socket);
    sqlite3_close(db); // Close the SQLite database
    return 0;
}
