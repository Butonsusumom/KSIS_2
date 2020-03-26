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

#define PORT 8080
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

int generateFile(char filePath[]) {
    const int fileSize = ( 5 + rand() % 25 ) * 10000; 
    std::ofstream out;         
    out.open(filePath); 
    if (out.is_open())
    {

        for (int i = 0; i < fileSize; i++) {
            out << char(32 + rand()% 125);
        }
    
        out.close();
        return 0;
    }
    return 1; 
}

int main() {
    srand(static_cast<unsigned int>(time(0)));
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
    char filePath[MAX_PATH];

    do {
        std::cout << "Name of file: ";
        std::cin >> filePath;
    } while ( generateFile(filePath) );

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

    fileBuffer = (char*) malloc(BLOCK_SIZE);
    int blocks = 0;
    int recivedSyze = 0;

    char dontCare[1024] ;
    int time_start = clock();
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
                std::cout << std::endl; 
                std::cout << "Speed " << fileSize / ((clock() - time_start + 0.0000001) / 1000) << 
                    std::cout.precision(4) << " Byte/sec" << std::endl;
                recivedSyze = atoi(dontCare);
                std::cout << std::endl;
                std::cout << "Received " << recivedSyze << " package of " << blocks << " " << std::endl;
                if (recivedSyze == blocks) std::cout << "All package recived." << std::endl;
                else  std::cout << "Not all package recived." << std::endl;
            }
            break;
        default: {
            getchar();
        }
        }
    }
    

    closesocket(clientSocket);
    WSACleanup();
   
	return 0;
}