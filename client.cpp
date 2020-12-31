//Client

#pragma comment(lib, "ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define PACKET_SIZE 1024

#include <winsock2.h>
#include <iostream>
#include <thread>

using namespace std;

int cnt = 0, cntport = 4444;

template<typename T> class _dpVec {
private:
	T* data = NULL;
	int size, maxsize;
public:
	_dpVec() : size(0), maxsize(1), data(0) { }
	void add(T t) {
		if (size + 1 >= maxsize) {
			maxsize *= 2;
			T* tmp = new T[maxsize];
			for (int i = 0; i < size; i++) tmp[i] = data[i];
			tmp[size++] = t;
			delete[] data;
			data = tmp;
		}
		else data[size++] = t;
	}
	int length() { return size; }
	T& operator[](const int index) { return data[index]; }
};

class stream {
public:
	SOCKET server;
	SOCKADDR_IN addr = { 0 };
	int port;

	stream() { }
	stream(int port) {
		this->port = port;
		server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		addr.sin_port = htons(port);

		while (connect(server, (SOCKADDR*)&addr, sizeof(addr)));
		cout << port << " Connected." << endl;
	}
}; _dpVec<stream>streams;

void recvs(SOCKET s) {
	char msg[PACKET_SIZE];

	while (true) {
		ZeroMemory(msg, PACKET_SIZE);
		recv(s, msg, PACKET_SIZE, 0);

		if (WSAGetLastError()) return;

		if (!strcmp(msg, "Create")) {
			streams.add(stream(cnt + cntport));
			cnt++;
		}
	}
}

bool crtPort(int portnum) {
	for (int i = 0; i < streams.length(); i++) if (streams[i].port == portnum) return true;
	return false;
}

int main() {
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKET server;
	SOCKADDR_IN addr = { 0 };

	server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(8080);

	while (connect(server, (SOCKADDR*)&addr, sizeof(addr)));

	thread(recvs, server).detach();

	int portnum;
	char buffer[PACKET_SIZE];

	while (!WSAGetLastError()) {
		ZeroMemory(buffer, PACKET_SIZE);
		cin >> portnum >> buffer;

		if (crtPort(portnum)) {
			send(streams[portnum - cntport].server, buffer, PACKET_SIZE, 0);
		}
		else cout << "It is not exist stream." << endl;
	}

	closesocket(server);
	for (int i = 0; i < streams.length()-1; i++) closesocket(streams[i].server);
	WSACleanup();
}
