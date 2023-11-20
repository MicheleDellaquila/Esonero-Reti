/*
 *
 *
 * 	Created on: 14 nov 2023
 *  Author: Michele Dellaquila
 *  Description: TCP Server
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "protocol.h"

#if defined WIN32
#include <winsock.h>
#else
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif


void ClearWinSock() {
#if defined WIN32
    WSACleanup();
#endif
}

// Prototype
int communication(int clientSocket, int socketServer);
void welcomeMessage(int clientSocket, int socketServer);
void exitProgram(int socketServer);
int sendMessage(int clientSocket, int socketServer, char *inputServer);
MsgStruct getOperands(int clientSocket, int socketServer);
int performOperation(int a, int b, char operation);
void sendResult(int socketClient, int result);

// Math operations functions
int addition(int a, int b);
int subtraction(int a, int b);
int multiplication(int a, int b);
int division(int a, int b);

int main(int argc, char *argv[]) {
#if defined WIN32
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    int wsastartup;
    wsastartup = WSAStartup(wVersionRequested, &wsaData);
    if (wsastartup != 0) {
        printf("Error with WSAStartup()\n");
    }
#endif

    int socketServer;
    socketServer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socketServer < 0) {
        printf("Socket creation failed!");
        closesocket(socketServer);
        ClearWinSock();
        return -1;
    } else {
        puts("Socket creation successful");
    }

    // BIND FUNCTION
    struct sockaddr_in sockAdd;
    sockAdd.sin_family = AF_INET;
    sockAdd.sin_addr.s_addr = inet_addr(PROTO_ADDRESS);
    sockAdd.sin_port = htons(PROTO_PORT);

    if (bind(socketServer, (struct sockaddr*) &sockAdd, sizeof(sockAdd)) < 0) {
        puts("Binding failed!\n");
        closesocket(socketServer);
        ClearWinSock();
        return -1;
    } else {
        puts("Binding started!");
        puts(" ");
    }
    // ----------------------------------------------------

    // LISTEN FUNCTION
    int qlen = 5;
    if (listen(socketServer, qlen) < 0) {
        puts("Listening failed\n");
        closesocket(socketServer);
        ClearWinSock();
        return -1;
    } else {
        puts("Waiting for a client");
    }
    // ----------------------------------------------------

    // ACCEPT CLIENT FUNCTION
    struct sockaddr_in cad;
    int clientSocket;
    int clientLen;
    while (1) {
        puts("Waiting for client connection...");
        clientLen = sizeof(cad);
        if ((clientSocket = accept(socketServer, (struct sockaddr*) &cad, &clientLen)) < 0) {
            puts("Acceptance failed\n");
            closesocket(socketServer);
            ClearWinSock();
            return -1;
        }

        welcomeMessage(clientSocket, socketServer);

        while (1) {
            if (communication(clientSocket, socketServer) != 0)
                break;
        }
    } // end of the while loop
}

int communication(int clientSocket, int socketServer) {

    // RECEIVE MESSAGE FUNCTION
    int bytesRcvd;
    char buf[512];

    if ((bytesRcvd = recv(clientSocket, buf, 1024 - 1, 0)) <= 0) {
        puts("Connection closed by the client or reception failed");
        return -1;
    } else {
        switch (buf[0]) {
			case 'a':
				sendMessage(clientSocket, socketServer, "ADDITION");
				break;
			case 's':
				sendMessage(clientSocket, socketServer, "SUBTRACTION");
				break;
			case 'd':
				sendMessage(clientSocket, socketServer, "DIVISION");
				break;
			case 'm':
				sendMessage(clientSocket, socketServer, "MULTIPLICATION");
				break;
			default:
				sendMessage(clientSocket, socketServer, "TERMINATE CLIENT PROCESS");
        }
    }
    // ------------------------------------------------------------

    MsgStruct operands = getOperands(clientSocket, socketServer);

    int result = performOperation(operands.a, operands.b, buf[0]);
    result = htons(result);

    sendResult(clientSocket, result);
    return 0;
}

MsgStruct getOperands(int clientSocket, int socketServer) {
	MsgStruct msg;
    recv(clientSocket, &msg, sizeof(msg), 0);
    msg.a = ntohl(msg.a);
    msg.b = ntohl(msg.b);
    return msg;
}

int sendMessage(int clientSocket, int socketServer, char *inputServer) {
    // SEND MESSAGE FUNCTION
    int stringLen = strlen(inputServer);
    if (send(clientSocket, inputServer, stringLen, 0) != stringLen) {
        puts("Message too large");
        system("pause");
        closesocket(socketServer);
        ClearWinSock();
        return -1;
    }
    if (strcmp(inputServer, "exit") == 0) {
        exitProgram(socketServer);
    }
    puts("");
    // ------------------------------------------------------------
    return 0;
}

void welcomeMessage(int clientSocket, int socketServer) {
    char *inputServer = "Connection established";

    int stringLen = strlen(inputServer);
    send(clientSocket, inputServer, stringLen, 0);

    puts("");
    // ------------------------------------------------------------
}

void exitProgram(int socketServer) {
    // CLOSE THE CONNECTION
    closesocket(socketServer);
    ClearWinSock();
}

// Addition function
int addition(int a, int b) {
    return a + b;
}

// Subtraction function
int subtraction(int a, int b) {
    return a - b;
}

// Multiplication function
int multiplication(int a, int b) {
    return a * b;
}

// Division function
int division(int a, int b) {
    if (b == 0) {
        return 0;
    } else {
        return a / b;
    }
}

int performOperation(int a, int b, char operation) {
    switch (operation) {
		case 'a':
			return addition(a, b);
			break;

		case 's':
			return subtraction(a, b);
			break;

		case 'd':
			return division(a, b);
			break;

		case 'm':
			return multiplication(a, b);
			break;

		default:
			return -1;
    }
}

void sendResult(int socketClient, int result) {
    send(socketClient, &result, sizeof(result), 0);
}
