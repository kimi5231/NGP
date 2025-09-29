#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32")

#define TEST "www.naver.com"

int main(int argc, char* argv[])
{
	// 윈속 초기화
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 0;

	for (int i = 0; i< argc; i++)
		std::cout << argv[i] << std::endl;

	hostent* test = gethostbyname(argv[1]);
	std::cout << test->h_addr_list[0] << std::endl;
	in_addr addr;
	memcpy(&addr, test->h_addr_list[0], sizeof(in_addr));
	std::cout << "IP 주소: " << inet_ntoa(addr) << std::endl;

	//sockaddr_in addr;
	inet_ntop(AF_INET, test->h_addr_list[0], &addr.sin_addr);
	//std::cout << addr.sin_addr << std::endl;

	/*for(int i = 0; i< 5; i++)
		std::cout << test->h_addr_list[i] << std::endl;*/

	return 0;
}