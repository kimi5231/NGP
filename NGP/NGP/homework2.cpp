#include <iostream>
#include <winSock2.h>

#pragma comment(lib, "ws2_32")

// ��Ʈ��ũ ����Ʈ ����(Big Endian)
u_long network = htonl(0x12345678);
// ȣ��Ʈ ����Ʈ ����
u_long host = 0x12345678;

bool IsLittleEndian()
{
	if (ntohl(network) == network)
		return false;
	
	return true;
}

bool IsBigEndian()
{
	if (ntohl(network) == network)
		return true;
	
	return false;
}

int main()
{
	if (IsLittleEndian())
		std::cout << "ȣ��Ʈ�� ����Ʈ ���� ���: Little Endian" << std::endl;

	if (IsBigEndian())
		std::cout << "ȣ��Ʈ�� ����Ʈ ���� ���: Big Endian" << std::endl;

	return 0;
}