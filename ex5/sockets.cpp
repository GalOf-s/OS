#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <iostream>

#define MAX_HOST_NAME 256 // TODO what number to put here
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
 * prints system error to stderr
 */
void systemError(const std::string &string) {
    std::cerr << SYSTEM_ERROR_MSG + string << std::endl;
    exit(EXIT_FAILURE);
}

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

int client(char *buffer, unsigned short portNumber) {
    int clientSocketfd, serverSocketfd; size_t writeValue;
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
    if ((clientSocketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        systemError(SOCKET_ERROR_MSG);
    }


    // connect to the server
    if (connect(clientSocketfd, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
        close(clientSocketfd);
        systemError(CONNECT_ERROR_MSG);
    }

    writeData(clientSocketfd, buffer, strlen(buffer)); // TODO what now?



//    close(clientSocketfd);
    return 0;
}

int server(char* buffer, unsigned short portNumber) {
    int serverSocketfd, clientSocket;
    size_t readValue;
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
    if ((serverSocketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        systemError(SOCKET_ERROR_MSG);
    }

    // connect the server's address to his socket
    if (bind(serverSocketfd, (struct sockaddr*) &serverAddress, sizeof(struct sockaddr_in)) < 0) {
        close(serverSocketfd);
        systemError(BIND_ERROR_MSG);
    }

    // how many clients can listen to the server's socket
    if (listen(serverSocketfd, MAX_CLIENTS) < 0) {
        close(serverSocketfd);
        systemError(LISTEN_ERROR_MSG);
    }

    fd_set clientsFDs;
    fd_set readfds;
    FD_ZERO(&clientsFDs);
    FD_SET(serverSocketfd, &clientsFDs);
    FD_SET(STDIN_FILENO, &clientsFDs);
    for( ; ; ) {
        readfds = clientsFDs;
        if (select(MAX_CLIENTS + 1, &readfds, nullptr,nullptr, nullptr) < 0) {
            close(serverSocketfd);
            systemError(SELECT_ERROR_MSG);
        }
        if (FD_ISSET(serverSocketfd, &readfds)) {

            if ((clientSocket = accept(serverSocketfd, nullptr, nullptr)) < 0) {
                close(serverSocketfd);
                systemError(ACCEPT_ERROR_MSG);
            }
            FD_SET(clientSocket, &clientsFDs); // add new client to the clientFDs set

            if((readValue = readData(clientSocket, buffer, MAX_DATA_SIZE) < 0)) {
                close(serverSocketfd);
                systemError(READ_ERROR_MSG);
            }
            system(buffer);
            close(clientSocket);
        }
    }


    close(clientSocket);
    return 0;
}

int main(int argc, char* argv[]) {
    unsigned short portNumber = std::stoi(argv[2]);
    char *buffer = new (std::nothrow) char[MAX_DATA_SIZE];
    if(buffer == nullptr) {
        systemError(MEMORY_ALLOCATION_ERROR_MSG);
    }
    if(strcmp(argv[1], "client") == 0) {


        buffer = argv[3]; // terminalCommandToWrite
        client(buffer, portNumber);
    } else {
        server(buffer, portNumber);
    }
}



