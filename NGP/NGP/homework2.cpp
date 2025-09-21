#include <iostream>
#include <winSock2.h>

#pragma comment(lib, "ws2_32")

// Big Endian(네트워크 바이트)
u_long Big = 0x12345678;
// Little Endian
u_long Little = 0x78563412;

bool IsLittleEndian()
{
	if (ntohl(Big) == Big)
		return false;
	else if (ntohl(Big) == Little)
		return true;
}

bool IsBigEndian()
{
	if (ntohl(Big) == Big)
		return true;
	else if (ntohl(Big) == Little)
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