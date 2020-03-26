// ConsoleApplication2.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
int main()
{
    FILE* file;
    file = fopen("test1.txt", "w");
    unsigned char buffer[10240];
    srand(time(0));
    int kol = rand() % 8000+3000;
    std::cout << kol;
    for (int i = 1; i <= kol; ++i) {
        for (int j = 0; j < 10240; ++j) {
            buffer[j] = rand() % 256;
        }
        fwrite(buffer, 1, 10240, file);
    }
    fclose(file); 
}
