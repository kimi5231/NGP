#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32")

int main(int argc, char* argv[])
{
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 0;

	hostent* host = gethostbyname(argv[1]);

	std::cout << "[��� ����]" << std::endl;
	for (int i = 0; host->h_aliases[i]; i++)
		std::cout << host->h_aliases[i] << std::endl;

	std::cout << std::endl;
	
	std::cout << "[��� IPv4]" << std::endl;
	for (int i = 0; host->h_addr_list[i]; i++)
	{
		char ip[INET_ADDRSTRLEN]{};
		inet_ntop(host->h_addrtype, host->h_addr_list[i], ip, sizeof(ip));
		std::cout << ip << std::endl;
	}

	// ���� ����
	WSACleanup();
	return 0;
}