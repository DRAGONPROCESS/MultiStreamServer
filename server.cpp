//Server

#pragma comment(lib, "ws2_32.lib")

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
	_dpVec() : size(0), maxsize(1) { }
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
	SOCKET server, _stream;
	SOCKADDR_IN addr = { 0 }, client_sock = { 0 };
	int client_size, port;
	stream() { }
	stream(int port) {
		this->port = port;
		server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(port);

		bind(server, (SOCKADDR*)&addr, sizeof(addr));
		listen(server, SOMAXCONN);

		client_size = sizeof(client_sock);
		_stream = accept(server, (SOCKADDR*)&client_sock, &client_size);
	}
}; _dpVec<stream>streams;

void recvs(SOCKET s, int p) {
	char buffer[PACKET_SIZE];

	while (true) {
		ZeroMemory(buffer, PACKET_SIZE);
		recv(s, buffer, PACKET_SIZE, 0);

		if (WSAGetLastError()) break;
		cout << p << " ::: " << buffer << endl;
	}
}

int main() {
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKET server, _stream;
	SOCKADDR_IN addr = { 0 }, client_sock = { 0 };
	int client_size;

	server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(8080);

	bind(server, (SOCKADDR*)&addr, sizeof(addr));
	listen(server, SOMAXCONN);

	client_size = sizeof(client_sock);
	_stream = accept(server, (SOCKADDR*)&client_sock, &client_size);

	char msg[PACKET_SIZE];

	while (!WSAGetLastError()) {
		ZeroMemory(msg, PACKET_SIZE);
		cin >> msg;

		if (!strcmp(msg, "create")) {
			send(_stream, "Create", PACKET_SIZE, 0);
			streams.add(stream(cnt + cntport));
			thread(recvs, streams[cnt]._stream, streams[cnt].port).detach();
			cnt++;
		}
	}

	closesocket(_stream);
	closesocket(server);
	for (int i = 0; i < streams.length() - 1; i++) {
		closesocket(streams[i]._stream);
		closesocket(streams[i].server);
	}
	WSACleanup();
}
