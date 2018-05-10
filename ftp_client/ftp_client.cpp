// ftp_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <sstream>

#define WS_START_ERROR 1
#define RESOLVE_HOST_ERROR 2
#define CREATE_SOCKET_ERROR 3
#define CONNECT_HOST_ERROR 4

#define BUFFER_SIZE 100

class ftp_client
{
private:
	WSADATA SData;
	SOCKET s;
	addrinfo *host = NULL;

	std::vector<std::thread> thread_pool;

	char *buffer;

	void send_command(std::string com, std::string arg)
	{

	}

	bool check_readability()
	{
		fd_set s_set;
		s_set.fd_count = 1;
		s_set.fd_array[0] = s;

		timeval time;
		time.tv_sec = 0;
		time.tv_usec = 100;
		return select(0, &s_set, NULL, NULL, &time) > 0;
	}

	void recieve_new_line(std::string line)
	{
		std::cout << "Meomeomeo" << std::endl;
		std::cout << line;
	}

	void recieve_info()
	{
		while (check_readability()) {
			int byte = recv(s, buffer, BUFFER_SIZE, 0);
			if (byte < 0) {
				std::cout << "Socket error" << std::endl;
				return;
			}
			else if (byte == 0) {
				std::cout << "Disconnect" << std::endl;
				return;
			}

			buffer[byte] = 0;
			std::cout << buffer;
		}
	}

	void recieve_thread() {
		std::stringstream stream;
		int result = 0;
		while ((result = recv(s, buffer, BUFFER_SIZE, 0)) > 0) {
			buffer[result] = 0;
			stream << buffer;
			for (int i = 0; i < result; i++)
				if (buffer[i] == '\n') {
					std::string str;
					std::getline(stream, str);
					recieve_new_line(str);
				}
		}
	}



public:
	~ftp_client() {
		closesocket(s);
		//thread_pool[0].join();

		freeaddrinfo(host);
		WSACleanup();
		free(buffer);
	}

	ftp_client(std::string server, int port = 21) {
		int iResult = WSAStartup(0x0202, &SData);
		if (iResult)
			throw WS_START_ERROR;

		//Resolving host name
		struct addrinfo hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		iResult = getaddrinfo(server.c_str(), std::to_string(port).c_str(), &hints, &host);

		if (iResult) {
			freeaddrinfo(host);
			WSACleanup();
			throw RESOLVE_HOST_ERROR;
		}

		//Connect
		s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (s == INVALID_SOCKET) {
			freeaddrinfo(host);
			WSACleanup();
			throw CREATE_SOCKET_ERROR;
		}

		iResult = connect(s, host->ai_addr, host->ai_addrlen);

		if (iResult) {
			freeaddrinfo(host);
			WSACleanup();
			throw CONNECT_HOST_ERROR;
		}

		buffer = (char*)malloc(BUFFER_SIZE + 1);
		recieve_info();
		//thread_pool.push_back(std::thread(&ftp_client::recieve_thread, this));
	}

	SOCKET open_port() {
		sockaddr_in add;
		socklen_t add_len = sizeof(add);
		getsockname(s, (sockaddr*)&add, &add_len);

		add.sin_port = 0; //use other port

		SOCKET data_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		bind(data_s, (sockaddr*)&add, sizeof(add));


		getsockname(data_s, (sockaddr*)&add, &add_len);

		char ipAddress[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(add.sin_addr), ipAddress, INET_ADDRSTRLEN);

		//add.sin_addr.s_addr = INADDR_LOOPBACK;
		listen(data_s, 10);

		send_command("PORT " + std::to_string((add.sin_addr.s_addr >> 0) & 255) + ","
			+ std::to_string((add.sin_addr.s_addr >> 8) & 255) + ","
			+ std::to_string((add.sin_addr.s_addr >> 16) & 255) + ","
			+ std::to_string((add.sin_addr.s_addr >> 24) & 255) + ","
			+ std::to_string((add.sin_port) & 255) + ","
			+ std::to_string((add.sin_port >> 8) & 255));

		return data_s;
	}

	void dir() {
		SOCKET data_s = open_port();
		send_command("LIST");

		data_s = accept(data_s, NULL, NULL);

		int len;
		while ((len = recv(data_s, buffer, BUFFER_SIZE, 0)) > 0) {
			buffer[len] = 0;
			std::cout << buffer;
		}

		closesocket(data_s);
	}

	void login(std::string username, std::string password)
	{
		send_command("USER " + username);
		send_command("PASS " + password);
	}

	void send_command(std::string command) {
		command += "\r\n";
		send(s, command.c_str(), command.length(), 0);
		recieve_info();
	}
};

int main()
{
	std::string ftp_server;
	int port;
	std::cout << "Nhap server: ";
	std::getline(std::cin, ftp_server);

	std::cout << "Nhap port: ";
	std::cin >> port;

	try {
		ftp_client client = ftp_client(ftp_server, port);
		std::cout << "Successfully connect to host" << std::endl;

		while (1) {
			std::string command;

			std::cout << "> ";
			std::getline(std::cin, command);

			if (command == "DIR") {
				client.dir();
			}
			else {
				client.send_command(command);
			}
		}
	}
	catch (int e) {
		switch (e) {
		case WS_START_ERROR:
			std::cout << "Cannot start winsock service" << std::endl;
			return 1;
		case RESOLVE_HOST_ERROR:
			std::cout << "Cannot resolve the given hostname" << std::endl;
			return 1;
		case CREATE_SOCKET_ERROR:
			std::cout << "Cannot create socket" << std::endl;
			return 1;
		case CONNECT_HOST_ERROR:
			std::cout << "Cannot connect to host" << std::endl;
			return 1;
		}
	}


	return 0;
}

