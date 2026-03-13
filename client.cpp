#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <arpa/inet.h>

// ===========================
// Helper functions and definitions
// ===========================
void die(const char *msg)
{
    perror(msg);
    exit(1);
}

// ===========================
// Main function
// ===========================

int main()
{
    // ===========================
    // Section: Creating a Socket and Connecting to the Server
    // ===========================
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        die("socket()");
    }

    struct sockaddr_in addr = {};                  // Initialize the sockaddr_in structure to zero
    addr.sin_family = AF_INET;                     // Set the address family to AF_INET (IPv4)
    addr.sin_port = htons(1234);                   // Set the port number to 1234, converting it to network byte order using htons()
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1 is the loopback address,
                                                   // which allows the client to connect to a server running on the same
                                                   // machine. htonl() converts it to network byte order.

    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr)); // Attempt to connect to the server
    if (rv)
    { // connect() returns 0 on success, -1 on error
        die("connect()");
    }

    // Print address information for debugging purposes
    struct sockaddr_in local_addr, remote_addr; // Structures to hold local and remote address information
    socklen_t len = sizeof(local_addr);         // Initialize len to the size of local_addr

    getsockname(fd, (struct sockaddr *)&local_addr, &len);
    printf("Local: %s:%d\n", inet_ntoa(local_addr.sin_addr), ntohs(local_addr.sin_port));

    getpeername(fd, (struct sockaddr *)&remote_addr, &len);
    printf("Remote: %s:%d\n", inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));

    // ===========================
    // Section: Sending a Message to the Server and Receiving a Response
    // ===========================
    char msg[] = "hello";
    ssize_t wn = write(fd, msg, strlen(msg)); // Send the message "hello" to the server
    if (wn < 0)
    {
        die("write() error");
    }

    char rbuf[64] = {};
    ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1); // Read the response from the server into the buffer rbuf
    if (n < 0)
    {
        die("read() error");
    }
    printf("server says: %s\n", rbuf); // Print the response from the server
    close(fd);                         // Close the socket connection
    return 0;
}
