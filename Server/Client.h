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
	std::string getAllData()
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
			tm* time = gmtime(&lastOnline);
			std::cout << "; last online: " << asctime(time);
		}
	}
	SOCKET getSocket()const{
		return clientConnect;
	}
	void leaves()
	{
		if (lastOnline == 0){
			lastOnline = time(0);
			shutdown(clientConnect, SD_SEND);
		}
		
	}
	bool operator==(Client &right)
	{
		if (this->getAllData() == right.getAllData()) {
			clientConnect = right.getSocket();
			this->lastOnline = 0;
			return 1;
		}
		return 0;
	}
};