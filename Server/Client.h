#pragma once
#include <iostream>
#include "windows.h"
#include "WinSock2.h"


class Client
{
private:
	SOCKET clientConnect;
	std::string username, system, ip;
	time_t lastOnline = 0;
public:
	Client(SOCKET clientSocket) : clientConnect(clientSocket) {}
	void setUserName(std::string name){
		username = name;
	}
	void setSystemInfo(std::string info) {
		system = info;
	}
	void setIp(std::string ip) {
		this->ip = ip;
	}
	std::string getAllData()	//для сравнивания клиентов
	{
		return username + "|" + system + "|" + ip;
	}
	std::string getUserName()
	{
		return username;
	}
	void print() const
	{
		std::cout << "Username: " << username << "; " << system << " - " << ip;
		if (lastOnline == 0)
			std::cout << ", online!\n";
		else{
			tm* time = gmtime(&lastOnline);			//преобразуем время в секундах в удобный для восприятия вид
			std::cout << "; last online: " << asctime(time);
		}
	}
	SOCKET getSocket()const{
		return clientConnect;
	}
	void leaves()		//при отключении пользователя
	{
		if (lastOnline == 0){			//если он online
			lastOnline = time(0);			//последнее время в сети - текущий момент
			shutdown(clientConnect, SD_SEND);	//отключаем сокет
		}
		
	}
	bool operator==(Client &right)		//при проверке не являются ли 2 клиента одним и тем же
	{
		if (this->getAllData() == right.getAllData()) {
			clientConnect = right.getSocket();		//переносим сокет нового в старый
			this->lastOnline = 0;				//устанавливаем статус "online"
			return 1;
		}
		return 0;
	}
};
