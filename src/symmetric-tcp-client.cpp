#include "symmetric-tcp-client.hpp"

SymmetricTcpClient::SymmetricTcpClient(std::string ip_address, uint16_t port, std::string keyfile)
:SimpleTcpClient(ip_address, port),
encryptor(keyfile){}

bool SymmetricTcpClient::reconnect(){
	if(close(this->fd) < 0){
		perror("symmetric tcp client reconnect closing socket");
	}
	this->writes = 0;
	this->reads = 0;
	return SimpleTcpClient::reconnect();
}

std::string SymmetricTcpClient::communicate(std::string request){
	return this->communicate(request.c_str(), request.length());
}

std::string SymmetricTcpClient::communicate(const char* request, size_t length){
	char response[PACKET_LIMIT];
	std::string response_string;
	ssize_t len;
	
	int i = 0;
	while(!this->connected || this->encryptor.send(this->fd, request, length, &this->writes)){
		this->connected = this->reconnect();
		if(i++ >= 5){
			return std::string();
		}
	}

	std::function<ssize_t(int, const char*, size_t)> set_response_callback = [&](int, const char* data, size_t data_length)->ssize_t{
		response_string = std::string(data);
		return static_cast<ssize_t>(data_length);
	};

	do{
		if((len = this->encryptor.recv(this->fd, response, PACKET_LIMIT, set_response_callback, &this->reads)) < 0){
			return std::string();
		}
	}while(len == 0);

	return response_string;
}
