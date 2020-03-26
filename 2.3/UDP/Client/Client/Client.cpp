#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#include <winsock2.h>
#include <fstream>
#include <cstdlib>
#include <time.h>
#pragma comment(lib,"ws2_32.lib")

int main()
{
    const int bufNum = 1024; //размер буфера
    int iResult;
    WSAData wsaData;
    iResult = WSAStartup(MAKEWORD(2, 1), &wsaData); //настраиваем библиотеку
    if (iResult != NO_ERROR) {
        std::cout << "Error 1";
        return 1;
    }
    SOCKADDR_IN serverAdress;  //указываем IP-адрес и порт сервера
    int sizeaddr = sizeof(serverAdress);
    serverAdress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAdress.sin_port = htons(3228);
    serverAdress.sin_family = AF_INET;

    SOCKET ClientSocket = socket(AF_INET, SOCK_DGRAM, NULL); //создаем сокет клиента
    if (ClientSocket == INVALID_SOCKET) {
        std::cout << "Error 2";
        WSACleanup();
    }
    char buf[bufNum]; //буфер 
    ZeroMemory(buf, bufNum);

    sendto(ClientSocket, "Successful client connection", 28, 0, (SOCKADDR*)&serverAdress, sizeaddr);//проверяем подключение 
    recvfrom(ClientSocket, buf, bufNum, 0, (sockaddr*)&serverAdress, &sizeaddr);                                 
    std::cout << buf <<std::endl;

    FILE* file; //трафик, который будем передавать 
    char filename[] = "test1.txt";
    file = fopen(filename, "rb");
    struct stat fileInfo;
    stat(filename, &fileInfo);
    int fileSize = fileInfo.st_size; //находим его размеры 
    int packages; //и количество пакетов
    std::cout << "File size: "<< fileSize <<" bytes" << std::endl;
    if (fileSize % bufNum != 0) {
        packages = fileSize / bufNum + 1;
    }
    else
    {
        packages = fileSize / bufNum;
    }
    std::cout << "Number of packages: " << packages << std::endl;

    clock_t start, stop;
    start = clock(); //начало время отсчета 
    while (not feof(file)) 
    {
        recvfrom(ClientSocket, buf, bufNum, 0, (sockaddr*)&serverAdress, &sizeaddr);
        int bytesRead = fread((void*)buf, 1, bufNum, file);
        sendto(ClientSocket, buf, bytesRead, 0, (sockaddr*)&serverAdress, sizeaddr);
    };
    sendto(ClientSocket, "ThisIsEnd", 9, 0, (sockaddr*)&serverAdress, sizeaddr);
    recvfrom(ClientSocket, buf, bufNum, 0, (sockaddr*)&serverAdress, &sizeaddr); //ожидание ответа о сервере о конце
    stop = clock(); //конец 

    double time = (stop - start+0.0) / CLK_TCK; //время передачи
    std::cout << "Time: " <<time<<" sec"<<std::endl;

    double speed = fileSize  / time; //скорость передачи
    std::cout << "Speed: " << speed <<" bytes per sec"<<std::endl;
    fclose(file);
    iResult = closesocket(ClientSocket);
    if (iResult == SOCKET_ERROR) {
        std::cout << "Error 3";
        return 1;
    }
    WSACleanup();
}