#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

const int BUF_SIZE = 42 * 4096;

// struct client
typedef struct client
{
    int id;
    char msg[110000];
}   t_client;
t_client clients[1024];

// fds for write and read and total
fd_set fds, wfds, rfds;

// max that gonna take the fd returned from the socket
int max = 0; int next_id = 0;

// buffers for read and write
char bufRead[BUF_SIZE], bufWrite[BUF_SIZE];

// fatal error
void    fatal_error()
{
    write(2, "Fatal error\n", 12);
    exit(1);
}

// send all function that sends a message to another socket
void send_all(int s)
{
    for (int i = 0; i <= max; i++)
        if (FD_ISSET(i, &rfds) && i != s)
            send(i, bufWrite, strlen(bufWrite), 0);
}

int main(int ac, char **av)
{
    // check the number of arguments
    if(ac != 2)
    {
        write(2, "Wrong number of arguments\n", 26);
        exit(1);
    }

    //setting zeros bytes in the struct clients
    bzero(&clients, sizeof(clients));
    FD_ZERO(&fds);

    //setting the socket and checks its return
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
        fatal_error();

    //max gets the return and we set it in the FD_SET
    max = sockfd;
    FD_SET(sockfd, &fds);

    // create struct sockaddr_in and assign IP, PORT 
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
	addr.sin_family = AF_INET; 
	addr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	addr.sin_port = htons(atoi(av[1]));

    // bind sockfd with info from addr (port and ip)
    if ((bind(sockfd, (const struct sockaddr *)&addr, sizeof(addr))) < 0)
        fatal_error();

    // listen defines how many clients she can listen one time
    if(listen(sockfd, 128) < 0)
        fatal_error();

    // start of our program
    while(1)
    {
        // set the write and read fds to the fd
        wfds = rfds = fds;
        // here we check if the max fd + 1 is free
        if(select(max + 1, &wfds, &rfds, NULL, NULL) < 0)
            continue;
        
        // we loop through the fds
        for(int s = 0; s <= max; s++)
        {
            // if we find that s == sockfd and FD_ISSET
            if(FD_ISSET(s, &wfds) && s == sockfd)
            {
                // we take the client fd with the accept method
                int clientSock = accept(sockfd, (struct sockaddr *)&addr, &addr_len);
                if(clientSock < 0)
                    continue;

                //we check which fd is bigger and we replace max with it
                max = (clientSock > max) ? clientSock : max;
                
                // here's the work of when a client just arrived
                clients[clientSock].id = next_id++;
                FD_SET(clientSock, &fds);
                sprintf(bufWrite, "server: client %d just arrived\n", clients[clientSock].id);
                send_all(clientSock);
                break;
            }
            if (FD_ISSET(s, &wfds) && s != sockfd) 
            {
                int res = recv(s, bufRead, 42*4096, 0);
                if (res <= 0) 
                {
                    sprintf(bufWrite, "server: client %d just left\n", clients[s].id);
                    send_all(s);
                    FD_CLR(s, &fds);
                    close(s);
                    break ;
                }
                else 
                {
                    for (int i = 0, j = strlen(clients[s].msg); i < res; i++, j++)
                    {
                        clients[s].msg[j] = bufRead[i];
                        if (clients[s].msg[j] == '\n')
                        {
                            clients[s].msg[j] = '\0';
                            sprintf(bufWrite, "client %d: %s\n", clients[s].id, clients[s].msg);
                            send_all(s);
                            bzero(&clients[s].msg, strlen(clients[s].msg));
                            j = -1;
                        }
                    }
                    break;
                }
            }
        }
    }
    return 0;
}