#include <iostream>
#include <winsock.h>

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
	SOCKET serverSocket;

	//AF_INET - указание семейства адресов 
	//в данном случае - семейство адресов протокола IPv4
	//SOCK_DGRAM - тип сокета. В данном случае - датаграммы, т.к. UDP
	//0 - транспортный протокол для указанного семейства адресов - по умолчанию (UDP)
	serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (serverSocket == INVALID_SOCKET)
	{
		printf("Error during socket creation: %d\n", WSAGetLastError());
		fatal();
	}

	//Шаг 3 - связывание сокета с локальным адресом
	//Занимаем порт 1337 и все доступные IP адреса
	SOCKADDR_IN localAdress;
	localAdress.sin_family = AF_INET;
	localAdress.sin_port = htons(MY_PORT);
	localAdress.sin_addr.s_addr = INADDR_ANY;

	//bind связывает сокет с локальным адресом
	if (bind(serverSocket, (sockaddr *)&localAdress, sizeof(localAdress)) == SOCKET_ERROR)
	{
		printf("Error during socket binding: %d\n", WSAGetLastError());
		closesocket(serverSocket);
		fatal();
	}

	//Шаг 4 - ожидание подключения - не нужен
	//т.к. протокол UDP не предполагают установку соединения

	//Шаг 5 - устанавливает соединение - не нужен по той же причине

	char cbuff[512]; //Буфер для приема сообщений
	SOCKADDR_IN clientAdress;
	int addrSize = sizeof(clientAdress);
	char exitMsg[5] = "EXIT"; 

	printf("Server succesfully initialized. Client can send files now...\n\n");

	int debug = recvfrom(serverSocket, cbuff, 512, 0, (sockaddr *)&clientAdress, &addrSize);
	while ((debug != SOCKET_ERROR) && (strcmp(cbuff, exitMsg) != 0))
	{
		//Первым сообщением приходит длина файла, сохраняем в переменную
		int fileLength = atoi(cbuff);
		int recievedBytes = 0;
		printf("File length: %d bytes\n", fileLength);
		
		//Сообщаем клиенту, что готовы принять следующее сообщение
		sendto(serverSocket, "echo-ready", 11, 0, (sockaddr *)&clientAdress, addrSize);
		recvfrom(serverSocket, cbuff, 512, 0, (sockaddr *)&clientAdress, &addrSize);
		
		//Вторым сообщением приходит имя файла, создаем и открываем для записи новый файл с этим именем
		char *fileName = (char *)malloc(strlen(cbuff) + 1);
		strcpy(fileName, cbuff);
		printf("File name: %s\n", fileName);

		FILE *recievedFile;
		recievedFile = fopen(fileName, "wb");

		//Пока не получили весь файл, сообщаем клиенту, что готовы принять следующую часть файла
		//Записываем её в новый файл
		int messageSize;
		while (recievedBytes != fileLength)
		{
			sendto(serverSocket, "echo-ready", 11, 0, (sockaddr *)&clientAdress, addrSize);
			messageSize = recvfrom(serverSocket, cbuff, 512, 0, (sockaddr *)&clientAdress, &addrSize);
			recievedBytes += messageSize;
			fwrite((void *)cbuff, 1, messageSize, recievedFile);
			//printf("Progress: %d/%d\n", recievedBytes, fileLength);
		}
		//Файл получен. Теперь его можно закрыть.
		sendto(serverSocket, "echo-recieved", 14, 0, (sockaddr *)&clientAdress, addrSize);
		fclose(recievedFile);
		printf("File succesfully recieved.\n\n");

		//Костыль, связан с тем, что если размер передаваемого файла кратный размеру буфера
		//то почему-то приходит одно лишнее сообщение нулевой длины.
		//Просто игнорим это сообщение, иначе сервак посчитает его за сообщение о следующем файле
		debug = recvfrom(serverSocket, cbuff, 512, 0, (sockaddr *)&clientAdress, &addrSize);
		if (!debug)
			debug = recvfrom(serverSocket, cbuff, 512, 0, (sockaddr *)&clientAdress, &addrSize);

	}

	printf("Shutting down server...");
	closesocket(serverSocket);
	WSACleanup();
	return 0;
}
