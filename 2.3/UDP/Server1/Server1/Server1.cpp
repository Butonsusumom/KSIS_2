// Server1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#pragma comment(lib,"ws2_32.lib")
#include <winsock2.h>
#include <ctime>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
int main()
{
	std::cout << "SERVER\n";
	const int bufNum = 1024; //размер буфера
	int iResult;
	WSAData wsaData;
	iResult = WSAStartup(MAKEWORD(2, 1), &wsaData);//настраиваем библиотеку
	if (iResult != NO_ERROR) {
		std::cout << "Error 1";
		return 1;
	}
	SOCKADDR_IN localAdress; //занимаем все IP-адреса и порт 
	int sizeaddr = sizeof(localAdress);
	localAdress.sin_addr.s_addr = INADDR_ANY;
	localAdress.sin_port = htons(3228);
	localAdress.sin_family = AF_INET;

	SOCKET ServerSocket = socket(AF_INET, SOCK_DGRAM, NULL); //создаем сокет сервера
	if (ServerSocket == INVALID_SOCKET) {
		std::cout << "Error 2";
		WSACleanup();
	}

	iResult = bind(ServerSocket, (SOCKADDR*)&localAdress, sizeaddr); // связываем сокет с локальным адресом
	if (iResult == SOCKET_ERROR) {
		closesocket(ServerSocket);
		WSACleanup();
		std::cout << "Error 3";
		return 1;
	}
	SOCKADDR_IN ClientAdress; //адрес клиента 
	int ClientAdresssize = sizeof(ClientAdress);

	char buf[bufNum];
	ZeroMemory(buf, bufNum);

	recvfrom(ServerSocket, buf, bufNum, 0, (SOCKADDR*)&ClientAdress, &ClientAdresssize);//проверяем подключение
	sendto(ServerSocket, "Successful server connection", 28, 0, (SOCKADDR*)&ClientAdress, ClientAdresssize);
	std::cout << buf << std::endl;

	FILE* file;  //эталонный трафик с которым будем сравнивать полученный
	char filename[] = "test1.txt";
	file = fopen(filename, "rb");
	struct stat fileInfo;
	stat(filename, &fileInfo);
	int fileSize = fileInfo.st_size; //находим его размеры 
	int packages; //и количество пакетов
	std::cout << "Standard file size: " << fileSize << " bytes" << std::endl;
	if (fileSize % bufNum != 0) {
		packages = fileSize / bufNum + 1;
	}
	else
	{
		packages = fileSize / bufNum;
	}
	std::cout << "Standard number of packages: " << packages << std::endl;
	int packagesNew = 0;
	int messageSize, newFileSize = 0;
	while (newFileSize != fileSize)
	{
		ZeroMemory(buf, bufNum);
		sendto(ServerSocket, "go", 2, 0, (sockaddr*)&ClientAdress, ClientAdresssize);
		messageSize = recvfrom(ServerSocket, buf, bufNum, 0, (sockaddr*)&ClientAdress, &ClientAdresssize);
		if (strcmp("ThisIsEnd", buf) == 0) {
			break;
		}
		newFileSize += messageSize;
		packagesNew += 1;
	}
	sendto(ServerSocket, "end", 3, 0, (sockaddr*)&ClientAdress, ClientAdresssize);
	std::cout << "New file size: " << newFileSize << " bytes" << std::endl;
	std::cout << "New number of packages: " << packagesNew << std::endl;
	std::cout << "Packages lost: " << packages - packagesNew << std::endl;
	iResult = closesocket(ServerSocket);
	if (iResult == SOCKET_ERROR) {
		std::cout << "Error 4";
		return 1;
	}
	WSACleanup();
	getchar();
}
