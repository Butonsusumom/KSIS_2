// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <winsock.h>
#pragma comment( lib, "ws2_32.lib" )
#define _CRT_SECURE_NO_WARNINGS
#define MY_PORT 1337  //номер прослушиваемого порта
#define SERVERADDR "127.0.0.1"  //локальный IP-адрес

int main()
{
	std::cout << "CLIENT\n";
	int errorCode;  //эта переменная хранит коды всех возможных ошибок во время выполнения
	int serverSize;  //размер серверного адреса
	char Buf[1024];  //буфер для наших сообщений
	int BufSize;  //размер буфера
	WORD versionRequested;  //номер требуемой версии (старший байт - версия, маладший - подверсия)
	WSADATA wsaData;  //информация о спецификации сокета Windows
	SOCKET clientSocket;  //созаваемый сокет, дескриптор клиента
	SOCKADDR_IN saServer;  //информация о серверном адресе
	versionRequested = MAKEWORD(2, 2);  ////соединяет два отдельных байта в двухбайтное слово
	//Шаг 1. Инициализируем библиотеку Winsock
	errorCode = WSAStartup(versionRequested, &wsaData);
	if (errorCode != 0)
	{
		printf("Sorry, it's not available to find a usable Winsock DLL\n");
		return 1;
	}
	if ((LOBYTE(wsaData.wVersion) != 2) || (HIBYTE(wsaData.wVersion) != 2))
	{
		printf("Sorry, it's not available to find a usable Winsock DLL\n");
		return 1;
	}
	//Шаг 2. Создаем сокет
	//AF-INET - сокет интернета
	//SOCK-DGRAM - дейтаграммный сокет, не ориентированный на соединение
	//IPROTO-UDP - прокол UDP
	clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (clientSocket == INVALID_SOCKET)
	{
		printf("socket failed with error = %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	else
	{
		//Шаг 3. Устанавливаем соединение с сервером
		saServer.sin_family = AF_INET;
		saServer.sin_port = htons(MY_PORT);
		saServer.sin_addr.s_addr = inet_addr(SERVERADDR);  //преобразуем формат из строкового в сетевой
		serverSize = sizeof(saServer);
		while (1)
		{
			//отправляем сообщение
			printf("User:");
			fgets(&Buf[0], sizeof(Buf) - 1, stdin);
			if (!strcmp(&Buf[0], "quit\n")) break;  //команда завершения работы чата
			sendto(clientSocket, &Buf[0], strlen(&Buf[0]), 0, (sockaddr *)&saServer, sizeof(saServer));

		}
	}
	//Шаг 4. Закрываем сокет и деинициализируем Winsock
	errorCode = closesocket(clientSocket);
	if (errorCode == SOCKET_ERROR)
	{
		printf("closesocket failed with error = %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	WSACleanup();
	return 0;
}