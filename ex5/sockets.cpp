#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <iostream>

#define MAX_HOST_NAME 256
#define MAX_DATA_SIZE 256
#define MAX_CLIENTS 5
#define FAILURE -1

#define SYSTEM_ERROR_MSG "system error: "
#define SOCKET_ERROR_MSG "error on socket"
#define BIND_ERROR_MSG "error on bind"
#define LISTEN_ERROR_MSG "error on listen"
#define ACCEPT_ERROR_MSG "error on accept"
#define READ_ERROR_MSG "error on read data"
#define CONNECT_ERROR_MSG "error on read data"
#define MEMORY_ALLOCATION_ERROR_MSG "failed to allocate memory."
#define HOST_NAME_ERROR_MSG "error on get host by name"
#define SELECT_ERROR_MSG "error on select"

/**
 * Prints system error to stderr.
 */
void systemError(const std::string &string) {
    std::cerr << SYSTEM_ERROR_MSG + string << std::endl;
    exit(EXIT_FAILURE);
}

/**
 * Writes a data into a buffer.
 */
size_t writeData(int socketFD, char *buffer, size_t sizeOfData){
    size_t bytesTotalCount = 0; // counts bytes read
    size_t bytesThisRound = 0; // bytes read this pass
    while (bytesTotalCount < sizeOfData) { // loop until full buffer
        bytesThisRound = write(socketFD, buffer, sizeOfData - bytesTotalCount);
        if (bytesThisRound > 0) {
            bytesTotalCount += bytesThisRound;
            buffer += bytesThisRound;
        }
        if (bytesThisRound < 1) {
            return FAILURE;
        }
    }
    return bytesTotalCount;
}

/**
 * Reads the data from a buffer.
 */
size_t readData(int socketFD, char *buffer, size_t sizeOfData) {
    size_t bytesTotalCount = 0; // counts bytes read
    size_t bytesThisRound = 0; // bytes read this pass
    while (bytesTotalCount < sizeOfData) { // loop until full buffer
        bytesThisRound = read(socketFD, buffer, sizeOfData - bytesTotalCount);
        if (bytesThisRound > 0) {
            bytesTotalCount += bytesThisRound;
            buffer += bytesThisRound;
        }
        if (bytesThisRound < 1) {
            return FAILURE;
        }
    }
    return bytesTotalCount;
}

/**
 * The clients side. Opening a socket and writing terminal commands to run into a servers socket.
 */
void client(char *buffer, unsigned short portNumber) {
    int clientSocketFD;
    struct sockaddr_in serverAddress{}; // address of server to connect to
    char serverName[MAX_HOST_NAME + 1];
    struct hostent *hostIp{};

    // host name to connect to initialization
    gethostname(serverName, MAX_HOST_NAME);
    hostIp = gethostbyname(serverName);
    if (hostIp == nullptr) {
        systemError(HOST_NAME_ERROR_MSG);
    }

    // host initialization to connect to
    memset(&serverAddress, 0, sizeof(struct sockaddr_in));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    memcpy(&serverAddress.sin_addr, hostIp->h_addr, hostIp->h_length); // TODO do we need to use inet_aton?

    // creating client socket file descriptor
    if ((clientSocketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        systemError(SOCKET_ERROR_MSG);
    }

    // connect to the server
    if (connect(clientSocketFD, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
        close(clientSocketFD);
        systemError(CONNECT_ERROR_MSG);
    }
    writeData(clientSocketFD, buffer, strlen(buffer)); // writes to the clients socket

    close(clientSocketFD);
}


/**
 * The servers size. Opening a sockets and waiting for clients to connect.
 * Reading clients data when and running terminal commands.
 */
void server(char* buffer, unsigned short portNumber) {
    int serverSocketFD, clientSocket;
    struct sockaddr_in serverAddress{};
    char serverName[MAX_HOST_NAME + 1];
    struct hostent *hostIp{};

    // servers host name initialization
    gethostname(serverName, MAX_HOST_NAME);
    hostIp = gethostbyname(serverName);
    if (hostIp == nullptr) {
        systemError(HOST_NAME_ERROR_MSG);
    }

    // host initialization
    memset(&serverAddress, 0, sizeof(struct sockaddr_in));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    memcpy(&serverAddress.sin_addr, hostIp->h_addr, hostIp->h_length); // TODO do we need to use inet_aton?

    // creating server socket file descriptor
    if ((serverSocketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        systemError(SOCKET_ERROR_MSG);
    }

    // connect the server's address to his socket
    if (bind(serverSocketFD, (struct sockaddr*) &serverAddress, sizeof(struct sockaddr_in)) < 0) {
        close(serverSocketFD);
        systemError(BIND_ERROR_MSG);
    }

    // how many clients can listen to the server's socket
    if (listen(serverSocketFD, MAX_CLIENTS) < 0) {
        close(serverSocketFD);
        systemError(LISTEN_ERROR_MSG);
    }

    fd_set clientsFDs;
    fd_set readFDs;
    FD_ZERO(&clientsFDs);
    FD_SET(serverSocketFD, &clientsFDs);
    FD_SET(STDIN_FILENO, &clientsFDs);
    for( ; ; ) {
        readFDs = clientsFDs;
        if (select(MAX_CLIENTS + 1, &readFDs, nullptr,nullptr, nullptr) < 0) {
            close(serverSocketFD);
            systemError(SELECT_ERROR_MSG);
        }
        if (FD_ISSET(serverSocketFD, &readFDs)) {
            if ((clientSocket = accept(serverSocketFD, nullptr, nullptr)) < 0) {
                close(serverSocketFD);
                systemError(ACCEPT_ERROR_MSG);
            }

            if(readData(clientSocket, buffer, MAX_DATA_SIZE) < 0) { // reads from the client socket
                close(serverSocketFD);
                systemError(READ_ERROR_MSG);
            }
            system(buffer);
            close(clientSocket);
        }
    }
}

/**
 * Runs server-client.
 */
int main(int argc, char* argv[]) {
    unsigned short portNumber = std::stoi(argv[2]);
    char *buffer = new (std::nothrow) char[MAX_DATA_SIZE];
    if(buffer == nullptr) {
        systemError(MEMORY_ALLOCATION_ERROR_MSG);
    }
    if(strcmp(argv[1], "client") == 0) {
        buffer = argv[3];
        client(buffer, portNumber);
    } else {
        server(buffer, portNumber);
    }
}



