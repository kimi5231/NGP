#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include "resource.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Shlwapi.lib")

HWND hIp;
HWND hLabel;
HWND hProgress;
HWND hSelectButton;
HWND hInputButton;
HWND hCancelButton;

OPENFILENAME fileDlg;
TCHAR path[MAX_PATH];

DWORD WINAPI ProcessNetwork(LPVOID arg)
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
	std::ifstream file(path, std::ios::binary);
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

	// 파일 이름 보내기
	// 고정 길이 송신
	char fileName[50];
	WideCharToMultiByte(CP_ACP, 0, PathFindFileName(path), -1, fileName, sizeof(fileName), NULL, NULL);
	int fileNameSize = strlen(fileName);
	send(clientSocket, (char*)&fileNameSize, sizeof(int), 0);

	// 가변 데이터 송신
	send(clientSocket, fileName, fileNameSize, 0);

	int totalReadByte = 0;
	std::vector<char> buffer(4096);
	while (!file.eof())
	{
		file.read(buffer.data(), buffer.size());
		// 실제로 읽은 바이트 수 확인
		std::streamsize readByte = file.gcount();
		totalReadByte += readByte;

		// 고정 길이 송신
		int len = static_cast<int>(readByte);
		send(clientSocket, (char*)&len, sizeof(int), 0);

		// 가변 데이터 송신
		send(clientSocket, buffer.data(), len, 0);

		// Procress Bar Udate
		SendMessage(hProgress, PBM_SETPOS, (static_cast<double>(totalReadByte)/fileSize)*100, 0);
	}

	// 파일 닫기
	file.close();

	// 소켓 닫기
	closesocket(clientSocket);

	// 윈속 종료
	WSACleanup();
	return 0;
}

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		hLabel = GetDlgItem(hDlg, IDC_LABEL);
		hProgress = GetDlgItem(hDlg, IDC_PROGRESS);
		hSelectButton = GetDlgItem(hDlg, IDB_SELECT);
		hCancelButton = GetDlgItem(hDlg, IDCANCEL);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDB_SELECT:
			ZeroMemory(&fileDlg, sizeof(fileDlg));
			fileDlg.lStructSize = sizeof(fileDlg);
			fileDlg.lpstrFile = path;
			fileDlg.nMaxFile = MAX_PATH;
			fileDlg.lpstrFilter = L"모든 파일\0*.*\0";
			fileDlg.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			
			if (GetOpenFileName(&fileDlg))
				SetDlgItemText(hDlg, IDC_LABEL, path);

			// Thread 생성
			CreateThread(NULL, 0, ProcessNetwork, NULL, 0, NULL);

			// Select Button 비활성화
			EnableWindow(hSelectButton, FALSE);
			return TRUE;
		case IDB_CANCEL:
			EndDialog(hDlg, IDB_CANCEL);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// DialogBox 생성
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, DlgProc);

	// Progress Bar 설정
	SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

	return 0;
}