/*
 *
 *	Created on: 14 nov 2023
 *  Author: Michele Dellaquila
 *  Description: TCP Client
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

/* Prototype */
int communication(int socketClient);
void exitProgram(int socketClient);
void receiveWelcome(int socketClient);
void sendOperands(int socketClient, int operand1, int operand2);
int getResult(int clientSocket);
char getOperation(char inputOperation);


/* CODE */
int main(int argc, char *argv[]) {
	#if defined WIN32
		// Initialize Winsock
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			puts("Error at WSAStartup()\n");
			return 0;
		}
	#endif

    // Create client socket
    int socketClient;
    socketClient = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socketClient < 0) {
        puts("Socket creation failed!");
        system("pause");
        closesocket(socketClient);
        ClearWinSock();
        return -1;
    }

    // Create a structure to store the client socket address information
    struct sockaddr_in socClientAdr;
    socClientAdr.sin_family = AF_INET;
    socClientAdr.sin_addr.s_addr = inet_addr(PROTO_ADDRESS);
    socClientAdr.sin_port = htons(PROTO_PORT);

    // Connect to the server
    if (connect(socketClient, (struct sockaddr*) &socClientAdr, sizeof(socClientAdr)) < 0) {
        puts("Connection failed.\n");
        system("pause");
        ClearWinSock();
        return -1;
    }

    // Call the receiveWelcome function to receive and display a welcome message from the server
    receiveWelcome(socketClient);
    puts("");

    // Enter into a continuous loop to handle communication with the server
    while (1)
    	 // Call the communication function to interact with the server
    	 // If the communication function returns a non-zero value, exit the loop
        if (communication(socketClient) != 0)
            break;

    return 0;
}

// Function to get the operation from the user
char getOperation(char inputOperation) {
	int isOperationCorrect;
	char operation[50], inputString[50];

	// save value in inputString
	inputString[0] = inputOperation;


	/* get operation */
	do {
		switch (inputString[0]) {
		   case '+':
			 strcpy(operation, "Addizione");
			 isOperationCorrect = 1;
		     break;

		   case '-':
			strcpy(operation, "Sottrazione");
			isOperationCorrect = 1;
		    break;

		   case '/':
			strcpy(operation, "Divisione");
			isOperationCorrect = 1;
		    break;

		   case 'x':
			strcpy(operation, "Moltiplicazione");
			isOperationCorrect = 1;
		    break;

		   case '=':
			strcpy(operation, "=");
			isOperationCorrect = 1;
		    break;

		   default:
			printf("Carattere errato, inserire un nuovo carattere (+ - x /): ");
			fgets(inputString, sizeof(inputString), stdin);

		    isOperationCorrect = 0;
		    break;
		};
	} while(isOperationCorrect != 1);

	return operation[0];
}

// Function to handle communication with the server
int communication(int socketClient) {
    char operation[50], inputString[150];
    int operand1, operand2;
    int finished = 0;

    // Until user will insert = calculator will work
    do {
        // Read a line of input from standard input (keyboard) and store it in the inputString array and parse the inputString to extract data
        printf("Insert an operation [es: + 23 45]: ");
    	fgets(inputString, sizeof(inputString), stdin);
    	sscanf(inputString, "%s %d %d", &operation, &operand1, &operand2);
    	printf("Debug: operandi after assignment: %d\n", operand1);
     	printf("Debug: operandi after assignment: %d\n", operand2);
    	puts("");

    	// Remove newline character if present
    	if (operation[strlen(operation) - 1] == '\n') {
    		operation[strlen(operation) - 1] = '\0';
    	}

    	// Check whether the contents of operation can be converted to an integer
    	int result = atoi(operation);


    	if (result != 0 || (result == 0 && operation[0] == '0')) {
    		printf("L'operazione non è un carattere intero, inserire un nuovo carattere (+ - x /): ");
    		puts("");
    	} else {
			// Get the operation from the user using the getOperation function
			operation[0] = getOperation(operation[0]);

			// Convert the operation to lowercase using the tolower function
			operation[0] = tolower(operation[0]);

			// Calculate the length of the operation string and Send the operation to the server using the socket
			int stringLen = strlen(operation);
			if (send(socketClient, operation, stringLen, 0) != stringLen) {
				puts("Message too large");
				system("pause");
				closesocket(socketClient);
				ClearWinSock();
				finished = 1;
				return -1;
			}

			puts("");

			// Receive message function
			int bytesRcvd;
			int totalBytesRcvd = 0;
			char buf[512];

			// Receive a message from the server with a maximum length of 1023 characters
			if ((bytesRcvd = recv(socketClient, buf, 1024 - 1, 0)) <= 0) {
				puts("Connection closed or reception failed");
				system("pause");
				closesocket(socketClient);
				ClearWinSock();
				finished = 1;
				return -1;
			}

			// Update the total bytes received and null-terminate the received data
			totalBytesRcvd += bytesRcvd;
			buf[bytesRcvd] = '\0';

			// Check if the received message indicates termination
			if (strcmp(buf, "TERMINATE CLIENT PROCESS") == 0) {
				exitProgram(socketClient);
				printf("%s\n\n", buf);
				system("pause");
				finished = 1;
				return -1;
			}

			printf("Server: %s\n\n", buf);

			// Check the type of operation received from the server
			if ((strcmp(buf, "ADDITION") || strcmp(buf, "SUBTRACTION") || strcmp(buf, "DIVISION") || strcmp(buf, "MULTIPLICATION"))) {
				sendOperands(socketClient, operand1, operand2);
			} else {
				puts("Error in operation");
				exitProgram(socketClient);
				printf("TERMINATE CLIENT PROCESS\n\n");
				system("pause");
				finished = 1;
				return -1;
			}

			// Display the result received from the server and Exit the program and display the termination message
			printf("Result: %d\n\n", getResult(socketClient));
			}
    } while(finished != 1);

    return -1;
}

// Function to receive the welcome message from the server
void receiveWelcome(int socketClient) {

    // Receive message function
    int bytesRcvd;
    int totalBytesRcvd = 0;
    char buf[512];
    if ((bytesRcvd = recv(socketClient, buf, 1024 - 1, 0)) <= 0) {
        puts("Connection closed or reception failed");
        system("pause");
        closesocket(socketClient);
        ClearWinSock();
    }
    totalBytesRcvd += bytesRcvd;
    buf[bytesRcvd] = '\0';

    printf("%s\n", buf);
}

void sendOperands(int socketClient, int operand1, int operand2) {
	printf("Operand 1: %d\n", operand1);
	printf("Operand 2: %d\n", operand2);

	// save operands into msg
	MsgStruct msg;
    msg.a = operand1;
    msg.a = htonl(msg.a);
    msg.b = operand2;
    msg.b = htonl(msg.b);

    send(socketClient, &msg, sizeof(msg), 0);
}

void exitProgram(int socketClient) {
    // Close the connection
    closesocket(socketClient);
    ClearWinSock();
}

int getResult(int clientSocket) {
	int result;

	// Use ntohl instead of ntohs for a 32-bit integer
	recv(clientSocket, &result, sizeof(result), 0);
	result = ntohl(result);
	return result;
}
