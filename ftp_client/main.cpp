// ftp_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ftp_client.h"


int main()
{
	_setmode(_fileno(stdout), _O_U16TEXT);
	//SetConsoleOutputCP(CP_UTF8);

	std::wstring ftp_server;
	int port;
	std::wcout << "Nhap server: ";
	std::getline(std::wcin, ftp_server);

	std::wcout << "Nhap port: ";
	std::wcin >> port;

	std::wcin.ignore(INT_MAX, '\n');
	//std::cin.clear();


	try {
		ftp_client client(ftp_server, port);
		std::wcout << "Successfully connect to host" << std::endl;

		client.send_command(L"OPTS UTF8 ON");

		while (1) {
			std::wstring command;
			std::wcout << "> ";
			std::getline(std::wcin, command);

			if (command == L"DIR") {
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
			std::wcout << "Cannot start winsock service" << std::endl;
			return 1;
		case RESOLVE_HOST_ERROR:
			std::wcout << "Cannot resolve the given hostname" << std::endl;
			return 1;
		case CREATE_SOCKET_ERROR:
			std::wcout << "Cannot create socket" << std::endl;
			return 1;
		case CONNECT_HOST_ERROR:
			std::wcout << "Cannot connect to host" << std::endl;
			return 1;
		}
	}


	return 0;
}

