
#include "HttpConnect.h"
#include <iostream>
#include <memory.h>
#ifdef WIN32
#include <winsock.h>
#include <tchar.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif // WIN32
#include <sstream>
using namespace std;

#ifdef WIN32
#pragma comment(lib,"ws2_32.lib")
#endif

HttpConnect::HttpConnect()
{
#ifdef WIN32
	WSADATA wsa = { 0 };
	WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
}

HttpConnect::~HttpConnect()
{
	for (std::map<std::string,SocketData>::iterator it = m_vSocket.begin();it!=m_vSocket.end();it++)
	{
		SocketData socketData = it->second;
		closesocket(socketData.socket);
		it->second.socket =0;
	}
	m_vSocket.clear();
}

bool HttpConnect::socketHttp(std::string host, std::string request)
{
	int sockfd;
	struct sockaddr_in address;
	struct hostent *server;

	SocketData socketData = m_vSocket[host];
	if (socketData.socket==0)
	{
		sockfd = socket(AF_INET, SOCK_STREAM, 0);	
	}
	else 
	{
		sockfd = socketData.socket;
	}

	address.sin_family = AF_INET;
	address.sin_port = htons(80);
	server = gethostbyname(host.c_str());

	if (server == 0)
	{
		return false;
	}

	memcpy((char *)&address.sin_addr.s_addr, (char*)server->h_addr, server->h_length);

	// 使用循环简单处理socket超时（被服务器关闭）。
	do 
	{
		int nRet = connect(sockfd, (struct sockaddr *)&address, sizeof(address));
		if (nRet == 0)
			break;
		if (nRet == WSAEISCONN)
		{
			OutputDebugString(_T("The socket is already connected. "));
			break;
		}
		if(nRet == WSAEISCONN)
		{
			OutputDebugString(_T("A connect request was made on an already connected socket."));
		}
		else if (nRet == WSAETIMEDOUT)
		{
			// 被服务器关闭，需要重新建立socket
			OutputDebugString(_T("Attempt to connect timed out without establishing a connection."));
			sockfd = socket(AF_INET, SOCK_STREAM, 0);
			continue;
		}
		if (-1 == nRet)
		{
			TCHAR buf[256] = {0};
			DWORD dwError = WSAGetLastError();
			_stprintf(buf,_T("Connection error,Code %d"),dwError);
			OutputDebugString(buf);
			sockfd = socket(AF_INET, SOCK_STREAM, 0);
			continue;
		}
		break;
	} while (1);
	socketData.socket = sockfd;
	socketData.lastTime = GetTickCount();

	m_vSocket[host] = socketData;
#ifdef WIN32
	send(sockfd, request.c_str(), request.size(), 0);
#else
	write(sockfd, request.c_str(), request.size());
#endif
	// 注意这里会导致运行报错，缓存区溢出
	// char buf[1024 * 1024] = { 0 };
	// 修改为100K作为缓冲区
	char buf[1024 * 100] = { 0 };
	int offset = 0;
	int rc;


	bool bHttpHeadHead = false;
	bool bHttpHeadTail = false;

#ifdef WIN32
	while (rc = recv(sockfd, buf + offset, 1024, 0))
#else
	while (rc = read(sockfd, buf + offset, 1024))
#endif
	{
		offset += rc;
		// 查找http头部
		if (bHttpHeadHead == false)
		{
			const char * pHttp = strstr(buf, "HTTP/1.");
			if (pHttp)
				bHttpHeadHead = true;
		}
		// 找到http头部，找http头部结尾
		if (bHttpHeadHead == true)
		{
			// Content-Length: 294 不一定有，但是\r\n\r\n一定会有
			const char * pContent = strstr(buf, "\r\n\r\n");
			if (pContent)
			{
				// 第一次Copntent数据就接受完毕了
				if (rc < 1024)
					break;
				bHttpHeadTail = true;
			}
			// Http尾部找到了，
			if (bHttpHeadTail)
			{
				if (pContent == NULL && rc < 1023)
					break;
			}
		}	
	}
#ifdef WIN32
	//closesocket(sockfd);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     	closesocket(sockfd);
#else
	close(sockfd);
#endif
	buf[offset] = 0;
#ifdef _DEBUG
	cout << buf << std::endl;
#endif // _DEBUG
	// 查找数据 
	char *pFind = strstr(buf, "\r\n\r\n");
	if (pFind)
	{
		m_strGetResult.clear();
		if (memcmp(buf, "HTTP/1.0",8) == 0)
		{
			m_strGetResult.append(pFind+4);
		}
		else if (memcmp(buf, "HTTP/1.1",8) == 0)
		{
			char* pData = strstr(pFind+4,"\r\n"); // 定位返回数据大小所在行
			if (pData)
			{
				m_strGetResult.append(pData);
			}
			else
			{
				m_strGetResult.append(pFind+4);
			}
		}
		else
		{
			m_strGetResult.append(pFind);
		}
	}
	return true;
}

void HttpConnect::postData(std::string host, std::string path, std::string post_content)
{
	//POST请求方式
	std::stringstream stream;
	stream << "POST " << path;
	stream << " HTTP/1.1\r\n";
	stream << "Host: " << host << "\r\n";
	stream << "User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN; rv:1.9.2.3) Gecko/20100401 Firefox/3.6.3\r\n";
	stream << "Content-Type: application/x-www-form-urlencoded\r\n";
	stream << "Content-Length: " << post_content.length() << "\r\n\r\n";
	stream << post_content.c_str();
	socketHttp(host, stream.str());
}

void HttpConnect::getData1_0(std::string host, std::string path, std::string get_content)
{
	//GET请求方式
	std::stringstream stream;
	stream << "GET " << path << "?" << get_content;
	stream << " HTTP/1.0\r\n";
	stream << "Host: " << host << "\r\n";
	stream << "User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN; rv:1.9.2.3) Gecko/20100401 Firefox/3.6.3\r\n";
	stream << "Connection: close\r\n\r\n";
	socketHttp(host, stream.str());
}

bool HttpConnect::getData1_1(std::string host, std::string path, std::string get_content)
{
	//GET请求方式
	std::stringstream stream;
	stream << "GET " << path << "?" << get_content;
	stream << " HTTP/1.1\r\n";
	stream << "Accept: */*\r\n";
	stream << "Host: " << host << "\r\n";
	stream << "User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN; rv:1.9.2.3) Gecko/20100401 Firefox/3.6.3\r\n";
	stream << "Connection: Keep-Alive\r\n\r\n";
	return socketHttp(host, stream.str());
}

std::string HttpConnect::GetLastRequestResult()
{
	return m_strGetResult;
}
