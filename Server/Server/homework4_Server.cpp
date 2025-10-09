#include <iostream>
#include <vector>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "ws2_32")

int main(void)
{
	// 콘솔 다시 그릴 때 필요한 변수 선언
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD written;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 0;

	// listenSocket 생성
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
		return 0;

	// bind
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(7777);
	if (bind(listenSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
		return 0;

	// listen
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
		return 0;
	
	// accept
	SOCKET clientSocket;
	sockaddr_in clientAddr;
	int addrLen = sizeof(clientAddr);
	clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &addrLen);
	if (clientSocket == INVALID_SOCKET)
		return 0;

	// 파일 전체 크기 받기
	size_t fileSize;
	recv(clientSocket, (char*)&fileSize, sizeof(size_t), MSG_WAITALL);
	
	// 파일의 모든 바이트를 담아둘 버퍼
	std::vector<char> fileBuffer{};

	int len;
	std::vector<char> buffer(512);

	while (true)
	{
		// 고정 길이 수신
		if (recv(clientSocket, (char*)&len, sizeof(int), MSG_WAITALL) == 0)
			break;

		// 가변 데이터 수신
		if (recv(clientSocket, buffer.data(), len, MSG_WAITALL) == 0)
			break;

		// 받은 가변 데이터 기록
		fileBuffer.insert(fileBuffer.end(), buffer.begin(), buffer.begin() + len);
	
		// 콘솔 다시 그리고, 수신률 표시
		GetConsoleScreenBufferInfo(hConsole, &csbi);
		DWORD cells = csbi.dwSize.X * csbi.dwSize.Y;
		FillConsoleOutputCharacter(hConsole, ' ', cells, { 0, 0 }, &written);
		FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cells, { 0, 0 }, &written);
		SetConsoleCursorPosition(hConsole, { 0, 0 });

		std::cout << "수신률: " << (static_cast<double>(fileBuffer.size()) / fileSize) * 100 << "%" << std::endl;
	}

	// 받은 파일 저장
	std::ofstream file("homeworkFile.3pg", std::ios::binary);
	if (file) 
	{
		file.write(fileBuffer.data(), fileBuffer.size());
		file.close();
	}

	// 소켓 닫기
	closesocket(clientSocket);
	closesocket(listenSocket);

	// 윈속 종료
	WSACleanup();
	return 0;
}