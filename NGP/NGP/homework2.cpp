#include <iostream>
#include <winSock2.h>

#pragma comment(lib, "ws2_32")

// 네트워크 바이트 정렬(Big Endian)
u_long network = htonl(0x12345678);
// 호스트 바이트 정렬
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
		std::cout << "호스트의 바이트 정렬 방식: Little Endian" << std::endl;

	if (IsBigEndian())
		std::cout << "호스트의 바이트 정렬 방식: Big Endian" << std::endl;

	return 0;
}