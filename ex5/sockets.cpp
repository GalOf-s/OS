#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <iostream>

#define MAXHOSTNAME 8; // TODO what number to put here
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

/**
 * prints system error to stderr
 */
void systemError(const std::string &string) {
    std::cerr << SYSTEM_ERROR_MSG + string << std::endl;
    exit(EXIT_FAILURE);
}

size_t writeData(int socketFD, char *buffer, int sizeOfData){
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

size_t readData(int socketFD, char *buffer, int sizeOfData) {
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

int client(char *terminalCommandsToWrite, unsigned short portNumber) {
    int clientSocketFD; size_t writeValue;

    // address of server to connect
    struct sockaddr_in serverAddress{};
    char *buffer = new char[MAX_DATA_SIZE];
    buffer = terminalCommandsToWrite; // TODO check if ok

    // creating client socket file descriptor
    if ((clientSocketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        systemError(SOCKET_ERROR_MSG);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    inet_aton("127.0.0.1", &(serverAddress.sin_addr)); // TODO check if this is ok or we need the function gethostbyname

    // connect to the server
    if ((clientSocketFD = connect(portNumber, (struct sockaddr*) &serverAddress,sizeof(serverAddress)) < 0)) {
        close(clientSocketFD);
        systemError(CONNECT_ERROR_MSG);
    }

    writeValue = writeData(clientSocketFD, buffer, MAX_DATA_SIZE); // TODO what now?

    close(clientSocketFD);
    return 0;
}

int server(unsigned short portNumber) {
    int serverSocketFD, clientSocket;
    size_t readValue;
    struct sockaddr_in serverAddress{};
    char* buffer = new char[MAX_DATA_SIZE];

    // creating server socket file descriptor
    if ((serverSocketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        systemError(SOCKET_ERROR_MSG);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    inet_aton("127.0.0.1", &(serverAddress.sin_addr)); // TODO check if this is ok or do we need the function gethostname
    memset(&serverAddress, 0, sizeof(struct sockaddr_in));

    // connect the server's address to his socket
    if (bind(serverSocketFD, (struct sockaddr*) &serverAddress,sizeof(struct sockaddr_in)) < 0) {
        close(serverSocketFD);
        systemError(BIND_ERROR_MSG);
    }

    // how many clients can listen to the server's socket
    if (listen(serverSocketFD, MAX_CLIENTS) < 0) {
        close(serverSocketFD);
        systemError(LISTEN_ERROR_MSG);
    }

    // waiting for connections
    if ((clientSocket = accept(serverSocketFD, nullptr, nullptr)) < 0) {
        close(serverSocketFD);
        systemError(ACCEPT_ERROR_MSG);
    }

    if((readValue = readData(clientSocket, buffer, 256) < 0)) { // TODO how to run command? should it run? what to do (:
        close(serverSocketFD);
        systemError(READ_ERROR_MSG);
    }

    close(clientSocket);
    return 0;
}

int main(int argc, char* argv[]) {
    unsigned short portNumber = std::stoi(argv[2]);
    if(stpcpy(argv[1],"client")) {
        char* terminalCommandsToWrite = argv[3];
        client(terminalCommandsToWrite, portNumber);
    } else {
        server(portNumber);
    }
}



