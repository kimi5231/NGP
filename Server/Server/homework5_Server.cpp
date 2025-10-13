#include <iostream>
#include <vector>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "ws2_32")

// Thread한테 넘길 구조체
struct ClientInfo
{
	SOCKET clientSocket{};
	size_t fileSize{};
	int fileNameSize{};
	std::vector<char> fileName{};
};

std::vector<HANDLE> threads;
std::vector<DWORD> threadIDs(2);

HANDLE threadEvent1;
HANDLE threadEvent2;

int thread1Rate = 0;
int thread2Rate = 0;

DWORD WINAPI ProcessClinet(LPVOID clientInfo)
{
	// 콘솔 다시 그릴 때 필요한 변수
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD written;

	ClientInfo* info = static_cast<ClientInfo*>(clientInfo);

	// 주소 정보 얻기
	sockaddr addr;
	int addrLen = sizeof(addr);
	getpeername(info->clientSocket, &addr, &addrLen);
	sockaddr_in* addrIn = reinterpret_cast<sockaddr_in*>(&addr);
	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(addrIn->sin_addr), ip, INET_ADDRSTRLEN);

	// 파일의 모든 바이트를 담아둘 버퍼
	std::vector<char> fileBuffer{};

	while (true)
	{
		int len;
		std::vector<char> buffer(4096);

		// 고정 길이 수신
		if (recv(info->clientSocket, (char*)&len, sizeof(int), MSG_WAITALL) == 0)
			break;

		// 가변 데이터 수신
		if (recv(info->clientSocket, buffer.data(), len, MSG_WAITALL) == 0)
			break;

		// 받은 가변 데이터 기록
		fileBuffer.insert(fileBuffer.end(), buffer.begin(), buffer.begin() + len);

		if (GetCurrentThreadId() == threadIDs[0])
		{
			// 수신률 기록
			thread1Rate = (static_cast<double>(fileBuffer.size()) / info->fileSize) * 100;

			WaitForSingleObject(threadEvent1, INFINITE);
			
			// 콘솔 다시 그리기
			GetConsoleScreenBufferInfo(hConsole, &csbi);
			DWORD cells = csbi.dwSize.X * csbi.dwSize.Y;
			FillConsoleOutputCharacter(hConsole, ' ', cells, { 0, 0 }, &written);
			FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cells, { 0, 0 }, &written);
			SetConsoleCursorPosition(hConsole, { 0, 0 });

			// 출력
			std::cout << "[" << info->fileName.data() << "]" << std::endl;
			std::cout << "IP: " << ip << std::endl;
			std::cout << "수신률: " << thread1Rate << "%" << std::endl;

			// 2번 쓰레드 깨우기
			SetEvent(threadEvent2);
		}
		else
		{
			//
			thread2Rate = (static_cast<double>(fileBuffer.size()) / info->fileSize) * 100;

			WaitForSingleObject(threadEvent2, INFINITE);

			// 출력
			std::cout << "[" << info->fileName.data() << "]" << std::endl;
			std::cout << "IP: " << ip << std::endl;
			std::cout << "수신률: " << thread2Rate << "%" << std::endl;

			// 1번 쓰레드 깨우기
			SetEvent(threadEvent1);
		}
	}

	// 받은 파일 저장
	std::ofstream file(info->fileName.data(), std::ios::binary);
	if (file)
	{
		file.write(fileBuffer.data(), fileBuffer.size());
		file.close();
	}

	while (thread1Rate != 100 || thread2Rate != 100)
	{
		if (GetCurrentThreadId() == threadIDs[0])
		{
			WaitForSingleObject(threadEvent1, INFINITE);
			if (thread1Rate != 100 || thread2Rate != 100)
			{
				// 콘솔 다시 그리기
				GetConsoleScreenBufferInfo(hConsole, &csbi);
				DWORD cells = csbi.dwSize.X * csbi.dwSize.Y;
				FillConsoleOutputCharacter(hConsole, ' ', cells, { 0, 0 }, &written);
				FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cells, { 0, 0 }, &written);
				SetConsoleCursorPosition(hConsole, { 0, 0 });

				// 출력
				std::cout << "[" << info->fileName.data() << "]" << std::endl;
				std::cout << "IP: " << ip << std::endl;
				std::cout << "수신률: " << thread1Rate << "%" << std::endl;
			}
			SetEvent(threadEvent2);
		}
		else
		{
			WaitForSingleObject(threadEvent2, INFINITE);

			// 출력
			std::cout << "[" << info->fileName.data() << "]" << std::endl;
			std::cout << "IP: " << ip << std::endl;
			std::cout << "수신률: " << thread2Rate << "%" << std::endl;
		
			SetEvent(threadEvent1);
		}
	}

	// 소켓 닫기
	closesocket(info->clientSocket);

	delete info;

	return 0;
}

int main(void)
{
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 0;

	// 이벤트 생성
	// thread1을 깨우는 이벤트
	threadEvent1 = CreateEvent(NULL, FALSE, TRUE, NULL);
	// thread2를 깨우는 이벤트
	threadEvent2 = CreateEvent(NULL, FALSE, FALSE, NULL);

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

	int clientCount = 0;
	
	while (clientCount != 2)
	{
		// accept
		SOCKET clientSocket;
		sockaddr_in clientAddr;
		int addrLen = sizeof(clientAddr);
		clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
			break;

		clientCount++;

		// 파일 전체 크기 받기
		size_t fileSize{};
		recv(clientSocket, (char*)&fileSize, sizeof(size_t), MSG_WAITALL);
			
		// 파일 이름 받기
		int fileNameSize{};
		std::vector<char> fileName(50);
		// 고정 길이 수신
		recv(clientSocket, (char*)&fileNameSize, sizeof(int), MSG_WAITALL);
		// 가변 데이터 수신
		recv(clientSocket, fileName.data(), fileNameSize, MSG_WAITALL);

		// Thread에 넘길 데이터 작성
		ClientInfo* info = new ClientInfo();
		info->clientSocket = clientSocket;
		info->fileSize = fileSize;
		info->fileNameSize = fileNameSize;
		info->fileName = fileName;
		
		threads.push_back(CreateThread(NULL, 0, ProcessClinet, info, 0, &threadIDs[clientCount - 1]));
	}
	
	// 쓰레드가 종료될 때까지 대기
	WaitForMultipleObjects(2, threads.data(), TRUE, INFINITE);

	// 소켓 닫기
	closesocket(listenSocket);

	// 핸들 닫기
	CloseHandle(threadEvent1);
	CloseHandle(threadEvent2);

	// 윈속 종료
	WSACleanup();

	return 0;
}