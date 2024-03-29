/*
*  Materials downloaded from the web.
*  Collected and modified for teaching purpose only.
*/

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#define ARRAY_SIZE 30 /* Size of array to receive */

#define BACKLOG 10 /* how many pending connections queue will hold */

#define RETURNED_ERROR -1

void *Send_Array_Data(void *socket_fd)
{
    int socket_id = (long int)socket_fd;
    int i = 0;
    /* Create an array of squares of first 30 whole numbers */
    int simpleArray[ARRAY_SIZE] = {0};
    for (i = 0; i < ARRAY_SIZE; i++)
    {
        simpleArray[i] = i * i;
    }

    uint16_t statistics;
    for (i = 0; i < ARRAY_SIZE; i++)
    {
        statistics = htons(simpleArray[i]);
        send(socket_id, &statistics, sizeof(uint16_t), 0);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    /* Thread */
    pthread_t client_thread;

    int sockfd, new_fd;            /* listen on sock_fd, new connection on new_fd */
    struct sockaddr_in my_addr;    /* my address information */
    struct sockaddr_in their_addr; /* connector's address information */
    socklen_t sin_size;

    /* Get port number for server to listen on */
    if (argc != 2)
    {
        fprintf(stderr, "usage: port_number\n");
        exit(1);
    }

    /* generate the socket */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    /* Enable address/port reuse, useful for server development */
    int opt_enable = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt_enable, sizeof(opt_enable));
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt_enable, sizeof(opt_enable));

    /* clear address struct */
    memset(&my_addr, 0, sizeof(my_addr));

    /* generate the end point */
    my_addr.sin_family = AF_INET;            /* host byte order */
    my_addr.sin_port = htons(atoi(argv[1])); /* short, network byte order */
    my_addr.sin_addr.s_addr = INADDR_ANY;    /* auto-fill with my IP */

    /* bind the socket to the end point */
    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    /* start listening */
    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    printf("server starts listnening ...\n");

    /* repeat: accept, send, close the connection */
    /* for every accepted connection, use a sepetate process or thread to serve it */
    while (1)
    { /* main accept() loop */
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr,
                             &sin_size)) == -1)
        {
            perror("accept");
            continue;
        }
        printf("server: got connection from %s\n",
               inet_ntoa(their_addr.sin_addr));

        //Create a thread to accept client

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&client_thread, &attr, Send_Array_Data, &new_fd);

        pthread_join(client_thread, NULL);

        if (send(new_fd, "All of array data sent by server\n", 40, 0) == -1)
            perror("send");

        close(new_fd);
    }
}
