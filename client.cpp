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
    const char msg[] = "hello";
    uint32_t msg_lenlen = (uint32_t)strlen(msg);
    char wbuf[4 + sizeof(msg)];
    memcpy(wbuf, &msg_lenlen, 4); // little-endian
    memcpy(&wbuf[4], msg, msg_lenlen);
    write_all(fd, wbuf, 4 + msg_lenlen);

    const size_t k_max_msg = 4096;
    char rbuf[4 + k_max_msg];

    // Step 1: Read 4 bytes for response length
    int32_t err = read_full(fd, rbuf, 4);
    if (err)
    { /* handle error */
    }

    uint32_t resp_len = 0;
    memcpy(&resp_len, rbuf, 4); // little-endian
    if (resp_len > k_max_msg)
    { /* handle error */
    }

    // Step 2: Read response payload
    err = read_full(fd, &rbuf[4], resp_len);
    if (err)
    { /* handle error */
    }

    // Step 3: Print response
    printf("server says: %.*s\n", resp_len, &rbuf[4]);
    close(fd); // Close the socket connection
    return 0;
}
