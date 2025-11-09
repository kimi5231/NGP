#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include <shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Ws2_32.lib")


// 대화상자 프로시저
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

HWND hIp;
HWND hLabel;
HWND hProgress;
HWND hSelectButton;
HWND hInputButton;
HWND hCancelButton;

OPENFILENAME ofn;
TCHAR path[MAX_PATH];
char fileName[50];

DWORD Ip;
char* Ip2;

DWORD WINAPI ClientMain(LPVOID arg)
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
	inet_pton(AF_INET, Ip2, &addr.sin_addr);
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
	int fileNameSize = strlen(fileName);
	send(clientSocket, (char*)&fileNameSize, sizeof(int), 0);

	// 가변 데이터 송신
	send(clientSocket, fileName, fileNameSize, 0);

	std::vector<char> buffer(4096);

	SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

	int totalReadByte = 0;
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

		SendMessage(hProgress, PBM_SETPOS, (static_cast<double>(totalReadByte)/fileSize)*100, 0);
	}

	//SetDlgItemText(hDlg, IDC_LABEL, L"전송 완료");

	// 파일 닫기
	file.close();



	// 소켓 닫기
	closesocket(clientSocket);

	// 윈속 종료
	WSACleanup();
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// 대화상자 생성
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, DlgProc);

	return 0;
}

// 대화상자 프로시저
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) 
	{
	case WM_INITDIALOG:
		hIp = GetDlgItem(hDlg, IDC_IPADDRESS);
		hLabel = GetDlgItem(hDlg, IDC_LABEL);
		hProgress = GetDlgItem(hDlg, IDC_PROGRESS);
		hInputButton = GetDlgItem(hDlg, IDB_INPUT);
		hSelectButton = GetDlgItem(hDlg, IDB_SELECT);
		hCancelButton = GetDlgItem(hDlg, IDCANCEL);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{
		case IDB_INPUT:
			SendMessage(hIp, IPM_GETADDRESS, 0, (LPARAM)&Ip);
			in_addr addr;
			addr.S_un.S_addr = htonl(Ip);
			Ip2 = inet_ntoa(addr);
			CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);
			return true;
		case IDB_SELECT:
			{
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.lpstrFile = path;
				ofn.nMaxFile = MAX_PATH;
				ofn.lpstrFilter = L"모든 파일\0*.*\0";
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

				if (GetOpenFileName(&ofn))
				{
					SetDlgItemText(hDlg, IDC_LABEL, ofn.lpstrFile);
					WideCharToMultiByte(CP_ACP, 0, PathFindFileName(ofn.lpstrFile), -1, fileName, sizeof(fileName), NULL, NULL);
					
				}
				
			}
			return true;
		case IDB_CANCEL:
			EndDialog(hDlg, true);
			return true;
		}
		return FALSE;
	}
	return FALSE;
}