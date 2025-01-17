// Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <winsock.h>
#pragma comment( lib, "ws2_32.lib" )
#define _CRT_SECURE_NO_WARNINGS
#define MY_PORT 1337  //номер прослушиваемого порта

int main()
{
	std::cout << "SERVER";
	int errorCode;    //эта переменная хранит коды всех возможных ошибок во время выполнения
	int clientSize;  //размер клиентского адреса
	int localSize;   //размер локального адреса
	char Buf[1024];   //буфер для наших сообщений
 	int BufSize;       //размер буфера
	WORD versionRequested; //номер требуемой версии (старший байт - версия, маладший - подверсия)
	WSADATA wsaData;  //информация о спецификации сокета Windows
	SOCKET serverSocket;  //созаваемый сокет, дескриптор сервера
	SOCKADDR_IN saClient, saLocal;  //информация о локальном и клиентском адресах
	HOSTENT *hostent;  //информация о хосте
	versionRequested = MAKEWORD(2, 2);  //соединяет два отдельных байта в двухбайтное слово
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
	serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (serverSocket == INVALID_SOCKET)
	{
		printf("socket failed with error = %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	else
	{
		//Шаг 3. Связываем сокет с локальным адресом
		saLocal.sin_family = AF_INET;
		saLocal.sin_port = htons(MY_PORT);
		saLocal.sin_addr.s_addr = INADDR_ANY; //сервер принимает подключения  со всех IP-адресов
		localSize = sizeof(saLocal);
		//вызываем bind для связывания
		errorCode = bind(serverSocket, (sockaddr*)&saLocal, sizeof(saLocal));
		if (errorCode != 0)
		{
			printf("bind failed with error = %d\n", WSAGetLastError());
			closesocket(serverSocket);
			WSACleanup();
			return 1;
		}
		while (1)
		{
			//Шаг 4. Ожидаем подключения от клиента и получаем сообщения
			clientSize = sizeof(saClient);
			BufSize = recvfrom(serverSocket, &Buf[0], sizeof(Buf) - 1, 0, (sockaddr *)&saClient, &clientSize);
			if (BufSize == SOCKET_ERROR)
			{
				printf("recvfrom failed with error = %d\n", WSAGetLastError());
				closesocket(serverSocket);
				break;
			}
			else
			{
				//выводим сведения о подключившемся клиенте
				hostent = gethostbyaddr((char *)&saClient.sin_addr, 4, AF_INET);
				printf("accepted connection from %s, port %d\n", inet_ntoa(saClient.sin_addr), ntohs(saClient.sin_port));
				Buf[BufSize] = 0;  //добавляем завершающий ноль в конце для вывода на экран
				printf("User:%s\n", &Buf[0]);
			}
		}
	}
	//Шаг 5. Закрываем сокет и деинициализируем Winsock
	errorCode = closesocket(serverSocket);
	if (errorCode == SOCKET_ERROR)
	{
		printf("closesocket failed with error = %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	WSACleanup();
	return 0;
}
