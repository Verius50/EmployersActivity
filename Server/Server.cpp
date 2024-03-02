#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <iostream>
#include "windows.h"
#include "WinSock2.h"
#include "WS2tcpip.h"
#include "thread"
#include "SFML/Graphics.hpp"
#include "Client.h"
#include "vector"
#include <conio.h>

#pragma comment (lib, "ws2_32.lib")

std::vector<Client> clients;

void newClient(SOCKET client)	//метод для обработки подключения нового клиента
{
    if (client == INVALID_SOCKET){
        std::cout << "accepting socket error";
        return;
    }
    Client newClient(client);
    char* buff = new char[256];		//для данныз о клиенте
    memset(buff, 0, 256);
    int result = 0;

    if (recv(client, buff, 256, 0) > 0) {	//в случае успешного получения данных
        newClient.setUserName(std::string(strtok(buff, "/")));
        newClient.setIp(std::string(strtok(NULL, "/")));
        newClient.setSystemInfo(std::string(strtok(NULL, "/")));
        bool alreadyExist = 0;
        for (int i = 0; i < clients.size(); ++i)	//проверяем не подключался ли ранее этот же клиент
              if (clients[i] == newClient) {		//если да, то возвращаем ему статус online (см. client.h operator==(...))
                  alreadyExist = 1;
                  break;
              }
          if (!alreadyExist)
              clients.push_back(newClient);	//если нет, создаём нового
    }
}


void getScreenShot(Client source)	//получает скриншот от указанного клиента
{

    char* screenInfo = new char[15];	//получаем размеры изображения
    if (recv(source.getSocket(), screenInfo, 15, 0) <= 0) {
        std::cout << "Getting screenshot info failed!";
        return;
    }
    char* output;	//для функции strtol
    int screenWidth = strtol(strtok(screenInfo, " "), &output, 10),	//делим полученную строку на значения
		screenHeight = strtol(strtok(NULL, " "), &output, 10);
   
    char* screen = new char[3 * screenWidth * screenHeight];
    if (recv(source.getSocket(), screen, 3 * screenWidth * screenHeight, 0) > 0) {	//если учпешно получили само изображение

        sf::Image screenbuffer;		//создаём изображение (см. документацию SFML)
        screenbuffer.create(screenWidth, screenHeight);
        sf::Color pixel;

        for (int i = 0; i < 3 * screenWidth * screenHeight; i += 3){	//перебираем его пиксели (у каждого 3 компонента)
            pixel = sf::Color(screen[i], screen[i + 1], screen[i + 2]);
            screenbuffer.setPixel((i / 3) % screenWidth, (i / 3) / screenWidth, pixel);
        }
        screenbuffer.saveToFile("Screen from " + source.getUserName() + ".png");	//созраняем изображение
    } else
    {
        std::cout << "Cant load screenshot!";
    }
}


void userInterface()	
{
    time_t start = time(0), now = time(0) + 1;		//обновление пользовательского интерфейса раз в секунду
    int answ = 0;
    do {
        now = time(0) + 1;
        if (now > start)
        {
            start = now;
            system("cls");
            std::cout << "\n\t\tServer is working!"
                << "\nConnected clients: \n";
            for (int i = 0; i < clients.size(); ++i) {
                std::cout << i << ". ";
                clients[i].print();
                if (send(clients[i].getSocket(), "m", 1, 0) <= 0)	//проверяем подключен ли клиент
                    clients[i].leaves();				//если нет, отключаем его
            }
            std::cout << "\n1) Make a screenshot"
                << "\n2) Quit\n";
        }
        if (_kbhit())	//при нажатии клавиши
        {
            answ = _getch();
            switch (answ)
            {
            case '1': {
                int n = 0;
                std::cout << "Enter client number: ";
                std::cin >> n;
                if (n < clients.size() && send(clients[n].getSocket(), "s", 1, 0) >= 0) {	//отправляем сигнал для получения скриншота
                    std::thread screenShot(getScreenShot, clients[n]);				//и в отдельном потоке ожилаем его получения
                    screenShot.detach();
                }
                break; }
            case '2':

                break;
            }
        }
    } while (answ != '2');

}


int main()
{
    WSAData wsaData;
    ADDRINFO hints, * addrResult = NULL;
    SOCKET listenSocket = INVALID_SOCKET, client = INVALID_SOCKET;	//сокет для прослушивания порта
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0){
        std::cout << "StartUp failed with error " << result;
        return 1;
    }
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;

    result = getaddrinfo(NULL, "12", &hints, &addrResult);

    listenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);

    if (listenSocket == INVALID_SOCKET){
        std::cout << "Creating listening socket failed with error: " << result;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }
    result = bind(listenSocket, addrResult->ai_addr, int(addrResult->ai_addrlen));
    if (result != 0){
        std::cout << "Binding socket error:" << result;
        closesocket(listenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }
    result = listen(listenSocket, SOMAXCONN);	
    if (result == SOCKET_ERROR){
        std::cout << "Listening failed with error: " << result;
        closesocket(listenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
    }
    std::thread user(userInterface);	//инициализируем интерфейс в отдельном потоке
    user.detach();
    while (1) {		//ждём подключений
        SOCKET client = accept(listenSocket, NULL, NULL);	//при подключении

        std::thread thr(newClient, client);		//в отдельном потоке его обрабатываем
        thr.detach();
    }

    closesocket(listenSocket);

}

