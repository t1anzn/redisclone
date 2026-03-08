#include <iostream>
#include <sys/socket.h> // For socket()
#include <netinet/in.h> // For AF_INET and SOCK_STREAM
#include <cstdio>
#include <cassert>
#include <cstdlib>

void die(const char *msg) // Helper function to print an error message and exit the program
{
    perror(msg);
    exit(1);
}

int main()
{
    // ===========================
    // Section: Creating a Socket
    // ===========================
    int fd = socket(AF_INET, SOCK_STREAM, 0); // Create a socket using IPv4 (AF_INET) and TCP (SOCK_STREAM)

    if (fd < 0) // Check if socket creation was successful
    {
        die("socket()"); // If not, print an error message and exit
    }

    // Set socket options to allow reuse of the address
    int val = 1;
    // This allows the socket to be quickly reused after it is closed, which is useful for server applications that need to restart frequently.
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // ===========================
    // Section: Bind to Address
    // ===========================
    struct sockaddr_in addr = {};

    addr.sin_family = AF_INET;       // Set the address family to IPv4
    addr.sin_port = htons(1234);     // Set the port number (convert to network byte order)
    addr.sin_addr.s_addr = htonl(0); // wildcard IP 0.0.0.0

    // bind parameters:
    // int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr)); // Bind the socket to the address
    if (rv)
    {
        die("bind()"); // If binding fails, print an error message and exit
    }
    return 0;
}
