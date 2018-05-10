#include "stdafx.h"
#include "ftp_client.h"

#include <codecvt>
#include <string>

// convert UTF-8 string to wstring
std::wstring utf8_to_wstring (const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.from_bytes(str);
}

// convert wstring to UTF-8 string
std::string wstring_to_utf8 (const std::wstring& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.to_bytes(str);
}

//milisecond
bool ftp_client::readable(int wait_time)
{
	fd_set s_set;
	s_set.fd_count = 1;
	s_set.fd_array[0] = s;

	timeval time;
	time.tv_sec = wait_time / 1000;
	time.tv_usec = wait_time % 1000;
	return select(0, &s_set, NULL, NULL, &time) > 0;
}

int ftp_client::recieve_info()
{
	do {
		int byte = recv(s, buffer, BUFFER_SIZE, 0);
		if (byte < 0) {
			return RECIEVE_SOCKET_ERROR;
		}
		else if (byte == 0) {
			return DISCONECTED;
		}

		buffer[byte] = 0;

		for (int i = 0; i < byte; i++)
			u_line += (buffer[i] == '\n'); //count number of unprocessed line

		response << buffer;
	} while (readable(10));

	return 0;
}

std::wstring ftp_client::getline()
{
	if (readable(10))
		recieve_info();

	if (u_line) {
		u_line--;
		std::wstring line;
		std::getline(response, line);
		return line;
	}
	else
		return L"";
}

SOCKET ftp_client::open_port() {
	sockaddr_in add;
	socklen_t add_len = sizeof(add);
	getsockname(s, (sockaddr*)&add, &add_len);

	add.sin_port = 0; //use other port

	SOCKET data_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	bind(data_s, (sockaddr*)&add, sizeof(add));


	getsockname(data_s, (sockaddr*)&add, &add_len);

	char ipAddress[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(add.sin_addr), ipAddress, INET_ADDRSTRLEN);

	listen(data_s, 10);

	send_command(L"PORT " + std::to_wstring((add.sin_addr.s_addr >> 0) & 255) + L","
		+ std::to_wstring((add.sin_addr.s_addr >> 8) & 255) + L","
		+ std::to_wstring((add.sin_addr.s_addr >> 16) & 255) + L","
		+ std::to_wstring((add.sin_addr.s_addr >> 24) & 255) + L","
		+ std::to_wstring((add.sin_port) & 255) + L","
		+ std::to_wstring((add.sin_port >> 8) & 255));

	return data_s;
}

ftp_client::~ftp_client() {
	closesocket(s);

	FreeAddrInfoW(host);
	WSACleanup();
	free(buffer);
}

ftp_client::ftp_client(std::wstring server, int port) {
	int iResult = WSAStartup(0x0202, &SData);
	if (iResult) throw WS_START_ERROR;

	//Resolving host name
	struct addrinfoW hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = GetAddrInfoW(server.c_str(), std::to_wstring(port).c_str(), &hints, &host);

	if (iResult) {
		FreeAddrInfoW(host);
		WSACleanup();
		throw RESOLVE_HOST_ERROR;
	}

	//Connect
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (s == INVALID_SOCKET) {
		FreeAddrInfoW(host);
		WSACleanup();
		throw CREATE_SOCKET_ERROR;
	}

	iResult = connect(s, host->ai_addr, host->ai_addrlen);

	if (iResult) {
		FreeAddrInfoW(host);
		WSACleanup();
		throw CONNECT_HOST_ERROR;
	}

	buffer = (char*)malloc(BUFFER_SIZE + 1);
	recieve_info();
	print_response(std::wcout);
}

void ftp_client::print_response(std::wostream & os)
{
	while (u_line > 0)
		os << getline() << std::endl;
}

void ftp_client::dir() {
	SOCKET data_s = open_port();
	send_command(L"LIST");

	data_s = accept(data_s, NULL, NULL);

	int len;
	while ((len = recv(data_s, buffer, BUFFER_SIZE, 0)) > 0) {
		buffer[len] = 0;
		std::wcout << utf8_to_wstring(buffer);
	}

	closesocket(data_s);
}

void ftp_client::send_command(std::wstring command) {
	while (!command.empty() && (command.back() == '\n' || command.back() == '\r'))
		command.pop_back();

	if (command.empty())
		return;

	command += L"\r\n";

	send(s, wstring_to_utf8(command).c_str(), command.length(), 0);
	recieve_info();
	print_response(std::wcout);
}
