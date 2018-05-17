// ftp_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ftp_client.h"


int main()
{
	_setmode(_fileno(stdout), _O_U16TEXT);

	std::wstring ftp_server;
	int port;
	std::wcout << "Nhap server: ";
	std::getline(std::wcin, ftp_server);

	std::wcout << "Nhap port: ";
	std::wcin >> port;

	std::wcin.ignore(INT_MAX, '\n');

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
			else if (command == L"PASV") {
				client.to_mode(PASSIVE_MODE);
			}
			else if (command == L"ACTV") {
				client.to_mode(ACTIVE_MODE);
			}
			else if (command == L"DOWN") {
				std::wstring source;
				std::wstring dest;
				std::wcout << "Enter file on host (source file): ";
				std::getline(std::wcin, source);
				std::wcout << "Enter file on client (dest file): ";
				std::getline(std::wcin, dest);
				client.download(source, dest);
			}
			else if (command == L"UP") {
				std::wstring source;
				std::wstring dest;
				std::wcout << "Enter file to client (source file): ";
				std::getline(std::wcin, source);
				std::wcout << "Enter file on host (dest file): ";
				std::getline(std::wcin, dest);
				client.upload(source, dest);
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

