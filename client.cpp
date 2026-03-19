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

const size_t k_max_msg = 4096;

void die(const char *msg)
{
    perror(msg);
    exit(1);
}

// Helper function to print a message to the console
void msg(const char *s)
{
    printf("%s\n", s);
}

static int32_t read_full(int fd, char *buf, size_t n)
{
    while (n > 0)
    {
        ssize_t rv = read(fd, buf, n);
        if (rv <= 0)
            return -1; // error or EOF
        n -= rv;
        buf += rv;
    }
    return 0;
}

static int32_t write_all(int fd, const char *buf, size_t n)
{
    while (n > 0)
    {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0)
            return -1; // error
        n -= rv;
        buf += rv;
    }
    return 0;
}

static int32_t query(int fd, const char *text)
{
    // ===========================
    // Prepare the query message to send to the server
    // ===========================
    uint32_t len = (uint32_t)strlen(text); // Get the length of the query text
    if (len > k_max_msg)
        return -1;               // Check if the length exceeds the maximum allowed message size
    char wbuf[4 + k_max_msg];    // Buffer to hold the query message. First 4 bytes is the length, followed by the query text
    memcpy(wbuf, &len, 4);       // Copy the length of the query text
    memcpy(&wbuf[4], text, len); // Copy the query text into the buffer.

    // ===========================
    // Send the query message to the server
    // ===========================
    if (int32_t err = write_all(fd, wbuf, 4 + len)) // Send the query message to the server, including the length header and the query text
    {
        return err;
    }

    // ===========================
    // Read the response from the server
    // ===========================
    char rbuf[4 + k_max_msg]; // Buffer to hold the response from the server
    int32_t err = read_full(fd, rbuf, 4);
    if (err)
    {
        msg("read() error or EOF");
        return err;
    }

    // ===========================
    // Process the response from the server
    // ===========================

    memcpy(&len, rbuf, 4); // Extract the length of the response message from the first 4 bytes of the response buffer
    if (len > k_max_msg)   // Check if the length of the response message exceeds the maximum allowed message size
    {
        msg("too long");
        return -1;
    }

    err = read_full(fd, &rbuf[4], len); // Read the response message body based on the length specified in the header
    if (err)
    {
        msg("read() error");
        return err;
    }
    printf("server says: %.*s\n", len, &rbuf[4]); // Print the server's response for debugging purposes
    return 0;
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
    // Section: Send a Query to the Server and Process the Response
    // ===========================
    int32_t err = query(fd, "hello1"); // Send a query message "hello1" to the server and process the response
    if (err)
    {
        goto L_DONE;
    }
    err = query(fd, "hello2"); // Send another query message "hello2" to the server and process the response
    if (err)
    {
        goto L_DONE;
    }

L_DONE:
    close(fd); // Close the socket connection
    return 0;
}
