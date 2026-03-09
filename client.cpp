#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

void die(const char *msg)
{
    perror(msg);
    exit(1);
}

int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        die("socket()");
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1
    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv)
    { // connect() returns 0 on success, -1 on error
        die("connect()");
    }

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
