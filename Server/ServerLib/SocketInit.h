#pragma once 

class SocketInit
{
public:


	SocketInit();

	// NOTE: WSACleanup�� ȣ���ϴ� ������ �־�� �ϴ� ���� �Ϲ����̴�.
	// �׷���, C++���� ���� ��ü�� �ı��� ȣ���� ������ 
	// �� �� �����Ƿ�, ������ ���μ����� �����ϴ� �����̹Ƿ� ���� WSACleanup�� ȣ������ �ʴ´�.
	void Touch();
};

extern SocketInit g_socketInit;