#include <iostream>
#include <winSock2.h>

#pragma comment(lib, "ws2_32")

WORD GetWORD(int low, int high)
{
	// 부 버전(상위 비트)
	WORD minor = high;
	minor = minor << 8;
	// 주 버전(하위 비트)
	WORD major = low;
	// 부 버전, 주 버전 결합
	WORD version = minor | major;

	return version;
}

int main()
{
	// 윈속 초기화
	WSADATA wsa;
	
	if (WSAStartup(GetWORD(2, 2), &wsa) != 0)
		return 1;
	std::cout << "[알림] 윈속 초기화 성공\n" << std::endl;

	// wsa 변수 필드 출력
	std::cout << "wVersion: " << std::hex << std::showbase << wsa.wVersion << std::endl; // 16진수 출력
	std::cout << "wHighVersion: " << wsa.wHighVersion << std::endl; // 16진수 출력
	std::cout << "szDescription: " << wsa.szDescription << std::endl;
	std::cout << "szSystemStatus: " << wsa.szSystemStatus << std::endl;

	// 윈속 종료
	WSACleanup();
	return 0;
}