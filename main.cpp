#include <iostream>
#include <sys/socket.h> // For socket()
#include <netinet/in.h> // For AF_INET and SOCK_STREAM
#include <cstdio>
#include <cassert>

int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("socket");
        exit(1);
    }

    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    return 0;
}
