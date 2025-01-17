#include <iostream>
#include <winsock.h>
#include <sys/stat.h>

//Прекратить работу библиотеки Winsock.dll и выйти с кодом -1
#define fatal() { WSACleanup(); return -1; }  

#define MY_PORT 1337 //Порт, который будет слушать сервер


int main()
{
	int errorCode;
	WSADATA WSAData;

	//Шаг 1 - инициализация библиотеки Winsock.dll
	errorCode = WSAStartup(0x0202, (LPWSADATA)&WSAData);
	if (errorCode)
	{
		printf("Error during WSA startup: %d\n", errorCode);
		return -1;
	}

	//Шаг 2 - создание сокета сервера
	SOCKET clientSocket;

	//AF_INET - указание семейства адресов 
	//в данном случае - семейство адресов протокола IPv4
	//SOCK_DGRAM - тип сокета. В данном случае - датаграммы, т.к. UDP
	//0 - транспортный протокол для указанного семейства адресов - по умолчанию (UDP)
	clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (clientSocket == INVALID_SOCKET)
	{
		printf("Error during socket creation: %d\n", WSAGetLastError());
		fatal();
	}

	//Указываем IP адрес и порт сервера, на который будем отправлять файлы
	SOCKADDR_IN serverAdress;
	serverAdress.sin_family = AF_INET;
	serverAdress.sin_port = htons(MY_PORT);
	serverAdress.sin_addr.s_addr = inet_addr("127.0.0.1");
	int addrSize = sizeof(serverAdress);


	char cbuff[512]; //Буфер для отправки сообщений
	char exitMsg[5] = "EXIT";  //Команда для корректного закрытия (с освобождением системных ресурсов)

	//Файл должен лежать в папке с экзешником (или в папке Client\Client если запускаете из-под отладчика)
	printf("Specify the name of the file you want to send (with extension). Type EXIT to close the client\n");
	scanf("%s", cbuff);
	int i = strcmp(cbuff, exitMsg);

	while (i != 0)
	{
		//Достали имя файла из буфера
		char *fileName = (char *)malloc(strlen(cbuff) + 1);
		strcpy(fileName, cbuff);
		//Создали и открыли для чтения соответствующий файл
		FILE *file;
		file = fopen(fileName, "rb");
		//Получили размер файла и засунули его строковое представление в буфер
		struct stat fileInfo;
		stat(fileName, &fileInfo);
		int fileSize = fileInfo.st_size;
		printf("File size: %d bytes\n", fileSize);
		_itoa(fileSize, cbuff, 10);

		//Отправляем размер файла серверу, затем ждём подтверждения, что сообщение дошло
		//Потом отправляем имя файла
		sendto(clientSocket, cbuff, strlen(cbuff), 0, (sockaddr *)&serverAdress, sizeof(serverAdress));
		recvfrom(clientSocket, cbuff, 512, 0, (sockaddr *)&serverAdress, &addrSize);
		sendto(clientSocket, fileName, strlen(fileName) + 1, 0, (sockaddr *)&serverAdress, sizeof(serverAdress));

		//Пока не конец файла, ждем сообщение от сервера, что он готов принять сообщение
		//Берём следующий кусок файла и отправляем на сервер
		while (not feof(file))
		{
			recvfrom(clientSocket, cbuff, 512, 0, (sockaddr *)&serverAdress, &addrSize);
			int bytesRead = fread((void *)cbuff, 1, 512, file);
			sendto(clientSocket, cbuff, bytesRead, 0, (sockaddr *)&serverAdress, sizeof(serverAdress));
		}

		//Отправить ещё файл? 
		printf("File sent succesfully.\n\n");
		printf("Specify the name of the file you want to send (with extension). Type EXIT to close the client\n");
		scanf("%s", cbuff);
		i = strcmp(cbuff, exitMsg);
	}

	//Послать серверу сообщение о закрытии, закрыть сокет и библиотеку winsock.dll
	printf("Shutting down client...");
	sendto(clientSocket, exitMsg, 5, 0, (sockaddr *)&serverAdress, sizeof(serverAdress));
	closesocket(clientSocket);
	WSACleanup();
	return 0;
}