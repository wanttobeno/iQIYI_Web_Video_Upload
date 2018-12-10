#include <string>
#include <map>


#pragma once

typedef struct SocketData
{
	unsigned int socket;
	unsigned long lastTime;
	SocketData()
	{
		socket = 0;
		lastTime = 0;
	}
};

class HttpConnect
{
private:
	std::string m_strGetResult;
public:
	HttpConnect();
	~HttpConnect();
	bool socketHttp(std::string host, std::string request);
	void postData(std::string host, std::string path, std::string post_content);
	void getData1_0(std::string host, std::string path, std::string get_content);
	bool getData1_1(std::string host, std::string path, std::string get_content);

	// 获取请求的结果
	std::string GetLastRequestResult();
	// socket缓冲池
	std::map<std::string,SocketData> m_vSocket;
};

