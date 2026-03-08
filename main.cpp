#include <iostream>
#include <sys/socket.h> // For socket()
#include <netinet/in.h> // For AF_INET and SOCK_STREAM
#include <cstdio>
#include <cassert>

int main()
{
    // Create a socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0) // Check if socket creation was successful
    {
        perror("socket"); // Print error message
        exit(1);          // Exit with error code
    }

    // Set socket options to allow reuse of the address
    int val = 1;
    // This allows the socket to be quickly reused after it is closed, which is useful for server applications that need to restart frequently.
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    return 0;
}
