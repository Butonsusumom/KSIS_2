// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>

#pragma comment (lib, "Ws2_32.lib")

#define PORT 7
#define SERVER_ADDRESS "127.0.0.1"

#define ON_SEND_NAME 0
#define ON_SEND_SIZE 1
#define ON_SEND_DATA 2

#define BLOCK_SIZE 0xFFFF

char* nameByPath(char* filePath) {
	int i = strlen(filePath) - 1;
	bool found = false;
	while (i >= 0 && !found) {
		if (filePath[i] == '/' || filePath[i] == '\\') {
			found = true;
			i++;
		}
		i--;
	}
	i++;
	return &filePath[i];
}

int main() {
	printf("CLIENT\n");
	WSADATA wsaData;

	//Инициализация библиотеки Winsock
	if (WSAStartup(0x202, &wsaData)) {
		printf("WSAStart error");
		return 1;
	}

	//Создание сокета
	SOCKET clientSocket;
	clientSocket = socket(AF_INET, SOCK_STREAM, 0); //TODO: IPPROTO_TCP
	if (clientSocket < 0) {
		printf("Error calling socket() function");
		return 1;
	}

	//Установка соединения
	sockaddr_in destAddress;
	destAddress.sin_family = AF_INET;
	destAddress.sin_port = htons(PORT);
	HOSTENT* hostent;

	// преобразование IP адреса из символьного в сетевой формат
	if (inet_addr(SERVER_ADDRESS) != INADDR_NONE) {
		destAddress.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
	}
	else {
		// попытка получить IP адрес по доменному имени сервера
		if (hostent = gethostbyname(SERVER_ADDRESS)) {
			// hst->h_addr_list содержит не массив адресов,
			// а массив указателей на адреса
			((unsigned long*)&destAddress.sin_addr)[0] = ((unsigned long**)hostent->h_addr_list)[0][0];
		}
		else {
			printf("Invalid address %s\n", SERVER_ADDRESS);
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}
	}

	//Адрес сервера получен – пытаемся установить соединение 
	if (connect(clientSocket, (sockaddr*)&destAddress, sizeof(destAddress))) {
		printf("Connect error");
		return 1;
	}

	printf("Connected successfully\n");

	//Чтение и передача сообщений    
	int answerSize;
	int toSend = ON_SEND_NAME;

	std::cout << "Name of file: ";
	char filePath[MAX_PATH];
	std::cin >> filePath;

	char* fileName = nameByPath(filePath);

	FILE* inFile;
	char* fileBuffer;
	long fileSize;

	inFile = fopen(filePath, "rb");
	fseek(inFile, 0, SEEK_END);
	fileSize = ftell(inFile);
	rewind(inFile);

	char sizeInChar[10] = { 0 };
	_itoa(fileSize, sizeInChar, 10);

	fileBuffer = (char*)malloc(BLOCK_SIZE);
	int blocks = 0;

	char dontCare[1024];
	while ((answerSize = recv(clientSocket, dontCare, 1024, 0)) != SOCKET_ERROR) {
		switch (toSend) {
		case ON_SEND_NAME:
			send(clientSocket, fileName, strlen(fileName) + 1, 0);
			toSend++;
			break;
		case ON_SEND_SIZE:
			send(clientSocket, sizeInChar, strlen(sizeInChar) + 1, 0);
			toSend++;
			break;
		case ON_SEND_DATA:
			if (blocks * BLOCK_SIZE <= fileSize) {
				int realBytes = fread(fileBuffer, 1, BLOCK_SIZE, inFile);
				//std::cout << "Real bytes" << realBytes << "\n";
				send(clientSocket, fileBuffer, realBytes, 0);
				blocks++;

			}
			else {
				fclose(inFile);
				toSend++;
			}
			break;
		default:
			getchar();
		}
	}

	closesocket(clientSocket);
	WSACleanup();

	return 0;
}
