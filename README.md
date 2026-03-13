## Compiling and Running

### Compiler Flags Explained
- **-Wall**: Enable most compiler warning messages
- **-Wextra**: Enable extra warning messages
- **-Og**: Optimize for debugging (use during development)
- **-O2**: Optimize for speed (use for release/performance)
- **-g**: Include debug information (for gdb and other debuggers)
- **-o \<name\>**: Name the output executable file
- **&& ./prog**: After compiling, immediately run the `prog` executable

### Example Commands

```sh
# Compile and run the client for debugging
g++ -Wall -Wextra -Og -g client.cpp -o prog && ./prog

# Compile the client with optimizations
g++ -Wall -Wextra -O2 -g client.cpp -o client

# Compile the server with optimizations
g++ -Wall -Wextra -O2 -g server.cpp -o server