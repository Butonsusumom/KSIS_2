#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#pragma comment (lib, "Ws2_32.lib")
#include <iostream>
#include <fstream>

#define PORT 8080

#define ON_RECEIVE_NAME 0
#define ON_RECEIVE_SIZE 1
#define ON_RECEIVE_DATA 2

#define BLOCK_SIZE 0xFFFF

DWORD WINAPI ProcessClient(LPVOID client_socket);

int main() {
    WSADATA wsaData;
    printf("TCP SERVER STARTED");

    //Инициализация библиотеки сокетов
    if (WSAStartup(0x0202, &wsaData)) {
        printf("Error WSAStartup %d\n",  WSAGetLastError());
        return 1;
    }

    // Создание сокета
    SOCKET serverSocket;
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Error socket %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    //Связывание сокета с локальным адресом
    sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(PORT);
    local_addr.sin_addr.s_addr = 0;

    if (bind(serverSocket, (sockaddr*) &local_addr, sizeof(local_addr))) {
        printf("Error bind %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    // Ожидание подключений
    // размер очереди – 0x100
    if (listen(serverSocket, 0x100)) {
        printf("Error listen %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    //Извлекаем сообщение из очереди
    SOCKET client_socket;    // сокет для клиента
    sockaddr_in client_addr; // адрес клиента (заполняется системой)
    // функции accept необходимо передать размер структуры
    int client_addr_size = sizeof(client_addr);

    // цикл извлечения запросов на подключение из очереди
    while ((client_socket = accept(serverSocket, (sockaddr*) &client_addr, &client_addr_size))) {
        // пытаемся получить имя хоста
        HOSTENT* hostName;
        hostName = gethostbyaddr((char*) &client_addr.sin_addr.s_addr, 4, AF_INET);

        // вывод сведений о клиенте
        printf("+%s [%s] new connect!\n", (hostName) ? hostName->h_name : "", inet_ntoa(client_addr.sin_addr));
            
        DWORD thID;
        CreateThread(NULL, NULL, ProcessClient, &client_socket, NULL, &thID);
    }
    return 0;
}

DWORD WINAPI ProcessClient(LPVOID clientSocket) {
    SOCKET socket;
    socket = ((SOCKET*)clientSocket)[0];
    
    // отправляем клиенту приветствие 
    send(socket, "0", 1, 0);

    int bytesReceived = 0;

    int toReceive = ON_RECEIVE_NAME;
    char fileName[MAX_PATH] = { 0 };
    int fileSize;
    char buffer[10] = { 0 };

    bytesReceived = recv(socket, fileName, MAX_PATH, 0);
    FILE* outFile;
    outFile = fopen(fileName, "wb");
    char* fileBuffer = new char[BLOCK_SIZE];
    int bytesCount = 0, count = 0;
    toReceive++;
    while (bytesReceived && bytesReceived != SOCKET_ERROR) {
      char buffer[200]; //time
      _itoa(count, buffer, 10);
      send(socket, buffer, strlen(buffer), 0);
      //  send(socket, "0", 3, 0);

        switch (toReceive) {
            case ON_RECEIVE_NAME:
                bytesReceived = recv(socket, fileName, MAX_PATH, 0);
                outFile = fopen(fileName, "wb");
                toReceive++;
                break;
            case ON_RECEIVE_SIZE:
                bytesReceived = recv(socket, buffer, 10, 0);
                fileSize = _atoi64(buffer);
                std::cout << "File Size: " << fileSize << "\n";
                toReceive++;
                break;
            case ON_RECEIVE_DATA:
                if (bytesCount < fileSize) {
                    bytesReceived = recv(socket, fileBuffer, BLOCK_SIZE, 0);
                    fwrite(fileBuffer, 1, bytesReceived, outFile);
                    bytesCount += bytesReceived;
                    ++count;

                }
                else {
                    fclose(outFile);
                    toReceive++;

                }
                break;
            default:
                getchar();
        }
    }
    printf("Disconnected :(");
    closesocket(socket);
    return 0;
}