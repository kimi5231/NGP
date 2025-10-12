#include <iostream>
#include <vector>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "ws2_32")

int main(int argc, char* argv[])
{
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 0;

	// clientSocket 생성
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
		return 0;

	// connect
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	addr.sin_port = htons(7777);
	if (connect(clientSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
		return 0;

	// 파일 열기
	std::ifstream file(argv[1], std::ios::binary);
	if (!file)
	{
		std::cout << "파일을 열 수 없습니다." << std::endl;
		return 0;
	}

	// 커서를 파일의 끝으로 이동시켜서 전체 파일 크기 구하기
	file.seekg(0, std::ios::end);
	size_t fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	// 파일 전체 크기 보내기
	send(clientSocket, (char*)&fileSize, sizeof(size_t), 0);

	std::vector<char> buffer(512);

	while (!file.eof())
	{
		file.read(buffer.data(), buffer.size());
		// 실제로 읽은 바이트 수 확인
		std::streamsize readByte = file.gcount();

		// 고정 길이 송신
		int len = static_cast<int>(readByte);
		send(clientSocket, (char*)&len, sizeof(int), 0);

		// 가변 데이터 송신
		send(clientSocket, buffer.data(), len, 0);
	}

	std::cout << "전송 완료" << std::endl;

	// 파일 닫기
	file.close();

	// 소켓 닫기
	closesocket(clientSocket);

	// 윈속 종료
	WSACleanup();
	return 0;
}