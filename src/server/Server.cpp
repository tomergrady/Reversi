/*
 * Tomer Grady 205660863
 * Raz Shenkman 311130777
 */

#include "Server.h"
#include "../client/Pair.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <cstdlib>


using namespace std;
#define MAX_CONNECTED_CLIENTS 10

Server::Server(int port) : port(port), serverSocket(0) {
    cout << "Server" << endl;
}


void Server::start() {
    // Create a socket point
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        throw "Error opening socket";
    }
    // Assign a local address to the socket
    struct sockaddr_in serverAddress = {};
    bzero((void *) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);
    if (bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1) {
        this->stop();
        throw "Error on binding";
    }
    // Start listening to incoming connections
    listen(serverSocket, MAX_CONNECTED_CLIENTS);
    // Define the client socket's structures
    struct sockaddr_in playerAddress1;
    struct sockaddr_in playerAddress2;
    socklen_t playerAddressLen1 = 0;
    socklen_t playerAddressLen2 = 0;
    int player1, player2;
    while (true) {
        cout << "Waiting for connections..." << endl;
        // Accept a new client connection
        player1 = accept(serverSocket, (struct sockaddr *) &playerAddress1, &playerAddressLen1);
        if (player1 == -1) {
            this->stop();
            throw "Error on accept";
        }
        cout << "Player X connected." << endl;

        initializingPlayer(player1, 1);
        // Accept a new client connection
        player2 = accept(serverSocket, (struct sockaddr *) &playerAddress2, &playerAddressLen2);
        if (player2 == -1) {
            this->stop();
            throw "Error on accept";
        }
        cout << "Player O connected." << endl;

        initializingPlayer(player2, 2);
        initializingPlayer(player1, 3);
        handleClients(player1, player2);
    }
}

void Server::initializingPlayer(int playerSocket, int playerNum) {
    ssize_t x = write(playerSocket, &playerNum, sizeof(playerNum));
    if (x == -1) {
        cout << "Error writing to socket" << endl;
        exit(1);
    }

}

void Server::handleClients(int player1, int player2) {
    int sender = player1;
    int receiver = player2;
    int temp;
    while (transferMessage(sender, receiver)) {
        temp = sender;
        sender = receiver;
        receiver = temp;
    }
    // Close communication with the client
    close(player1);
    close(player2);
}

bool Server::transferMessage(int sender, int receiver) {
    Pair pair;
    try {
        ssize_t checkTransfer = read(sender, &pair, sizeof(pair));
        if (checkTransfer <= 0) {
            return false;
        }
        // end of game
        if (pair.getCol() == -6 && pair.getCol() == -6) {
            return false;
        }
        checkTransfer = write(receiver, &pair, sizeof(pair));
        if (checkTransfer <= 0) {
            return false;
        }
    } catch(exception e) {
        return false;
    }
    return true;
}

void Server::stop() {
    close(serverSocket);
}

