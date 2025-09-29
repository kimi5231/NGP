#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32")

int main(int argc, char* argv[])
{
	// 윈속 초기화
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 0;

	hostent* test = gethostbyname(argv[1]);
	//hostent* test = gethostbyname("www.example.com");
	
	std::cout << test->h_name << std::endl;

	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	inet_pton(test->h_addrtype, test->h_addr_list[0], &addr.sin_addr);
	std::cout << addr.sin_addr << std::endl;

	for (int i = 0; test->h_aliases[i]; i++)
		std::cout << test->h_aliases[i] << std::endl;
	
	return 0;
}