#include <iostream>     // For input/output operations
#include <sys/socket.h> // For socket()
#include <netinet/in.h> // For AF_INET and SOCK_STREAM
#include <cstdio>       // For perror() and printf()
#include <cassert>      // For assert()
#include <cstdlib>      // This is included for exit() and other general utilities, but it is not used in the current code. It can be removed if not needed in future code additions.
#include <cstring>      // For strlen()
#include <unistd.h>     // For close()
#include <arpa/inet.h>  // For inet_ntoa() to convert IP addresses to human-readable form

// ===========================
// Helper functions and definitions
// ===========================

const size_t k_max_msg = 4096; // Define a constant for the maximum message size

void die(const char *msg) // Helper function to print an error message and exit the program
{
    perror(msg);
    exit(1);
}

// This function reads exactly 'n' bytes from the file descriptor 'fd' into the buffer 'buf'.
static int32_t read_full(int fd, char *buf, size_t n)
{
    while (n > 0)
    {
        ssize_t rv = read(fd, buf, n);
        if (rv <= 0)
        {
            return -1; // error or EOF
        }

        n -= rv;   // decrease the number of bytes left to read
        buf += rv; // move the buffer pointer forward by the number of bytes read
    }
    return 0;
}

static int32_t write_all(int fd, const char *buf, size_t n)
{
    while (n > 0)
    {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0)
        {
            return -1; // error
        }
        assert((size_t)rv <= n); // Ensure that we don't write more than 'n' bytes
        n -= rv;                 // decrease the number of bytes left to write
        buf += rv;               // move the buffer pointer forward by the number of bytes written
    }
    return 0;
}

static void dosomething(int connfd)
{
    char rbuf[64] = {};                               // Buffer to hold incoming data from the client
    ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1); // Parameters: file descriptor, buffer, size of buffer
    if (n < 0)
    {
        die("read() error");
        return;
    }
    printf("client says: %s\n", rbuf);

    // Prepare a response message to send back to the client
    char wbuf[] = "world";                          // Buffer containing the response message "world"
    ssize_t wn = write(connfd, wbuf, strlen(wbuf)); // Parameters: file descriptor, buffer, size of buffer (length of the message)
    if (wn < 0)
    {
        die("write() error");
        return;
    }
}

int32_t one_request(int connfd)
{
    // 4 byte header
    char rbuf[4 + k_max_msg];                 // Buffer to hold the incoming message, including a 4-byte header for the message length
    int32_t err = read_full(connfd, rbuf, 4); // Read the 4-byte header to get the message length
    if (err)
    {
        // handle error
        return err;
    }
    uint32_t len = 0;      // Variable to hold the length of the incoming message
    memcpy(&len, rbuf, 4); // assume little-endian, copy the 4-byte header into the 'len' variable to determine the length of the message
    if (len > k_max_msg)
    {
        // handle too long
        return -1;
    }
    // request body
    err = read_full(connfd, &rbuf[4], len); // Read the message body based on the length specified in the header
    if (err)
    {
        // handle error
        return err;
    }
    // process message
    printf("client says: %.*s\n", len, &rbuf[4]); // Print the client's message for debugging purposes
    // reply
    const char reply[] = "world";
    char wbuf[4 + sizeof(reply)];  // Buffer to hold the response message, including a 4-byte header for the message length
    len = (uint32_t)strlen(reply); // Get the length of the response message
    memcpy(wbuf, &len, 4);         // Copy the length of the response message
    memcpy(&wbuf[4], reply, len);  // Copy the response message itself into the buffer after the header
    return write_all(connfd, wbuf, 4 + len);
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

    // ===========================
    // Section: Listen for Connections
    // ===========================
    rv = listen(fd, SOMAXCONN); // Listen for incoming connections, allowing a maximum queue of SOMAXCONN (4096 on Linux)
    if (rv)
    {
        die("listen()");
    } // If listening fails, print an error message and exit

    // ===========================
    // Section: Accept Connections
    // ===========================
    while (true)
    {
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen); // Accept an incoming connection
        if (connfd < 0)
        {
            continue; // error
        }

        // ===========================
        // Debug Information: Print Client and Server Addresses
        // ===========================

        // Print the client's address and port for debugging purposes
        struct sockaddr_in local_addr, remote_addr;
        socklen_t len = sizeof(local_addr);

        // Get the local address (server side of the connection)
        if (getsockname(connfd, (struct sockaddr *)&local_addr, &len) < 0)
            perror("getsockname");
        else
            printf("Server local: %s:%d\n", inet_ntoa(local_addr.sin_addr), ntohs(local_addr.sin_port));

        // Get the remote address (client side of the connection)
        if (getpeername(connfd, (struct sockaddr *)&remote_addr, &len) < 0)
            perror("getpeername");
        else
            printf("Client remote: %s:%d\n", inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));

        while (true)
        {
            int32_t err = one_request(connfd);
            if (err)
                break;
        }

        dosomething(connfd);
        close(connfd);
    }
    return 0;
}
