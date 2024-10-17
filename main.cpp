/*
 * * Socket Server Example, based on this video: https://www.youtube.com/watch?v=esXw4bdaZkc&list=PL9IEJIKnBJjH_zM5LnovnoaKlXML5qh17&index=2
 * * Note: What's the point of using c++, if like 75% of the code ends up being C, i feel kinda scammed :v
 * * But oh well, at least im learning stuff
 * * PD: Install bettercomments for vscode and enable single line parsing, it really helps
 */
#include <sys/socket.h> // ? Basic socket headers
#include <sys/types.h>  // ? In order: Functions to create and manage sockets, network data types, inet protocols definition
#include <poll.h>

#include <netinet/in.h> // ? Needed for sockaddr_in struct, htons()
#include <arpa/inet.h>  // ? Needed fot inet_pton()
#include <unistd.h>     // ? Needed for write()

#include <cerrno>  // ? Needed to access c's errno, error codes from the socket.h functions get stored there
#include <cstdarg> // ? Needed to pass a variable argument list to a function just like in c (don't quote me on that one)
#include <cstdio>  // ? In this case needed to print formated strings like c
#include <cstring>

#include <iostream>
#include <stdlib.h>

#define PORT 18000           // Port that the client sends data to
#define DATABUFFER_SIZE 4096 // Size of the buffer where the recieved data goes

void err_n_die(const char *fmt, ...);         /** @param fmt Format for the cstring */
char *bin2hex(const char *input, size_t len); /**Str and len, convers chars to hexdecimal */

int main(int argc, char const *argv[])
{

    int listenfd, connfd; // ? File describtors for the lissening socket, and the connection socket
    int n;
    struct sockaddr_in servaddr;

    // ? See client code amd coments for explanation...
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        err_n_die("Could not create socket");
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;                // IPv4 Protocol
    servaddr.sin_port = htons(PORT);              // Port 18000
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // Set server address to any since we're lissening and not sending, and we don't wanna discriminate

    uint8_t recieved_data[DATABUFFER_SIZE + 1];   // Array of 4096 bytes (+ 1 extra for the eof "char")
    uint8_t response_buffer[DATABUFFER_SIZE + 1]; // Array of 4096 bytes (+ 1 extra for the eof "char")

    // ? With the server we have to bind the address and port we're listening to the socket.
    // The socket goes like "brrr, i know who's coming now >:3"
    // I repeat, don't quote me on anything for the love of god
    if ((bind(listenfd, (sockaddr *)&servaddr, sizeof(servaddr))) < 0)
    {
        err_n_die("Could not bind");
    }
    // ? After binding, "start" the socket
    // ? Now it's waiting for somebody to connect() like we do in the client
    // Param 2 is the max number of pending connections allowed. If more than 10 then go "boom"
    if (listen(listenfd, 10) > 0)
    {
        err_n_die("Could not lissen");
    }

    // Time for an infinite loop!
    // Loop until a client tryes to connect
    while (true)
    {
        printf("Waiting for a connection on port%d\n", PORT);
        fflush(stdout);

        // ? Accept returns a socket that holds the connection with the client
        // ? Now to comunicate with the client in particular we write/read to this one, while the other one keeps lissening
        // Worry about the last 2 args later...
        connfd = accept(listenfd, (sockaddr *)NULL, NULL);
        int open = 1;
        while (open == 1)
        {
            // Note to self: The -1 is bc recieved_data[-1] holds and eof char, and we do not want that no no ( i think )
            while ((n = read(connfd, recieved_data, DATABUFFER_SIZE - 1)) > 0)
            {
                // Print recieved data in text and hex formats
                printf("\n%s\n\n%s", bin2hex((const char *)recieved_data, n), recieved_data);

                // Check for custom close
                if (recieved_data[0] == 'C' && recieved_data[1] == 'L' && recieved_data[2] == 'O' && recieved_data[3] == 'S' && recieved_data[4] == 'E' )
                {
                    open = 0;
                }

                // If whatever we read ends in \n stop reading
                if (recieved_data[n - 1] == '\n')
                {
                    break;
                }
                std::cin;
                // Empty buffer to be able to read it again
                memset(recieved_data, 0, DATABUFFER_SIZE);
            }

            if (n < 0)
            {
                err_n_die("Could not read");
            }

            pollfd conn;
            conn.fd = connfd;
            conn.events = POLLOUT;

            
            
            // Write response to buffer
            sprintf((char *)response_buffer, "HTTP/1.0 200 OK\r\n\r\nRESPONSE");

            // SEND IT!
            if (write(connfd, (char *)response_buffer, strlen((char *)response_buffer)) < 0)
            {
                err_n_die("could not write");
            }
            memset(response_buffer, 0, DATABUFFER_SIZE);

            
            

        }
        close(connfd);
    }

    return 0;
}

void err_n_die(const char *fmt, ...)
{

    int error_number; // Error number, well set to whatever "errno" is
    error_number = errno;

    va_list arguments_pointer; // Pointer to the arguments array

    // Print Error to standard out
    va_start(arguments_pointer, fmt);         // Needed to make the args array accessible, //**  @params: args pointer, last fixed argument
    vfprintf(stdout, fmt, arguments_pointer); // Pointer to an c output stream (internally a FILE pointer), format for the string (c again), args pointer
    fprintf(stdout, "\n");
    fflush(stdout);

    // Pretty c printing stuff
    if (errno != 0)
    {
        fprintf(stdout, "(errno = %d) : %s\n", error_number, strerror(error_number));
        fprintf(stdout, "\n");
        fflush(stdout);
    }
    // Cleans the object created by va_start()
    va_end(arguments_pointer);
    exit(1);
}
char *bin2hex(const char *input, size_t len)
{

    char *result;
    char *hexits = new char[17];
    strcpy(hexits, "0123456789ABCDEF");
    if (input == NULL || len == 0)
    {
        return NULL;
    }

    int resultlength = (len * 3) + 1;

    result = new char[resultlength];
    bzero(result, resultlength);

    for (size_t i = 0; i < len; i++)
    {
        result[i * 3] = hexits[input[i] >> 4];
        result[(i * 3) + 1] = hexits[input[i] & 0x0F];
        result[(i * 3) + 2] = ' ';
    }
    return result;
}