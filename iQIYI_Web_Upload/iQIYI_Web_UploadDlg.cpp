// iQIYI_Web_UploadDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "iQIYI_Web_Upload.h"
#include "iQIYI_Web_UploadDlg.h"
#include "resource.h"
#include "HttpConnect.h"
#include "jsoncpp/json.h"
#include "Transcode.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CiQIYI_Web_UploadDlg 对话框




CiQIYI_Web_UploadDlg::CiQIYI_Web_UploadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CiQIYI_Web_UploadDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	srand(time(NULL));
}

void CiQIYI_Web_UploadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PATH, m_pEditPath);
}

BEGIN_MESSAGE_MAP(CiQIYI_Web_UploadDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_SERVER, &CiQIYI_Web_UploadDlg::OnBnClickedBtnServer)
	ON_BN_CLICKED(IDC_BTN_UPLOAD, &CiQIYI_Web_UploadDlg::OnBnClickedBtnUpload)
	ON_BN_CLICKED(IDC_BTN_SAVE, &CiQIYI_Web_UploadDlg::OnBnClickedBtnSave)
	ON_BN_CLICKED(IDC_BTN_FINISH, &CiQIYI_Web_UploadDlg::OnBnClickedBtnFinish)
	ON_BN_CLICKED(IDC_BTN_DEL, &CiQIYI_Web_UploadDlg::OnBnClickedBtnDel)
END_MESSAGE_MAP()


// CiQIYI_Web_UploadDlg 消息处理程序

BOOL CiQIYI_Web_UploadDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CiQIYI_Web_UploadDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CiQIYI_Web_UploadDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

std::string g_cookie = "";

std::string g_token = "";


std::string g_upload_url;
std::string g_file_id;


HttpConnect http;
long g_filesize;
std::string g_strfile;
std::string g_strFileName;

char * ReadFileBuf(const char* pFilePath,long * nFileSize)
{

	FILE* pFile = fopen(pFilePath, "rb");
	if (pFile == NULL)
		return NULL;

	_fseeki64(pFile, 0, SEEK_END);
	long nLen = ftell(pFile);
	*nFileSize = nLen;
	_fseeki64(pFile, 0, SEEK_SET);
	// 预留10空间作为越界保护
	char * pbuf = (char*)malloc(nLen + 10);
	if (pbuf)
	{
		memset(pbuf, 0, nLen+10);
		nLen = fread(pbuf, 1, nLen, pFile);
	} while (0);
	fclose(pFile);
	return pbuf;
}


// 定义64位整形
#if defined(_WIN32) && !defined(CYGWIN)
typedef __int64 int64_t;
#else
typedef long long int64t;
#endif  // _WIN32

// 获取系统的当前时间，单位微秒(us)
int64_t GetSysTimeMicros()
{
#ifdef _WIN32
	// 从1601年1月1日0:0:0:000到1970年1月1日0:0:0:000的时间(单位100ns)
#define EPOCHFILETIME   (116444736000000000UL)
	FILETIME ft;
	LARGE_INTEGER li;
	int64_t tt = 0;
	GetSystemTimeAsFileTime(&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;
	// 从1970年1月1日0:0:0:000到现在的微秒数(UTC时间)
	tt = (li.QuadPart - EPOCHFILETIME) / 10;
	return tt;
#else
	timeval tv;
	gettimeofday(&tv, 0);
	return (int64_t)tv.tv_sec * 1000000 + (int64_t)tv.tv_usec;
#endif // _WIN32
	return 0;
}

#define  Buf_Size MAX_PATH*10

void CiQIYI_Web_UploadDlg::OnBnClickedBtnServer()
{
	OutputDebugStringA(__FUNCTION__);
	char buf[Buf_Size+1] ={0};
	GetDlgItemTextA(m_hWnd,IDC_EDIT_PATH,buf,Buf_Size);
	g_strfile = buf;
	std::string strFileName;
	std::string::size_type nPos = g_strfile.find_last_of("\\");
	if (nPos != std::string::npos)
	{
		strFileName = g_strfile.c_str()+nPos+1;
	}
	else
	{
		strFileName = "video.mp4";
	}
	g_strFileName = strFileName;

	memset(buf,0,Buf_Size+1);
	GetDlgItemTextA(m_hWnd,IDC_EDIT_COOKIE,buf,Buf_Size);
	g_cookie = buf;

	memset(buf,0,Buf_Size+1);
	GetDlgItemTextA(m_hWnd,IDC_EDIT_TOKEN,buf,Buf_Size);
	g_token = buf;


	long nFileSize =0;
	char * pFileBuf = ReadFileBuf(g_strfile.c_str(),&nFileSize);
	free(pFileBuf);
	g_filesize = nFileSize;

	std::string host = "upload.iqiyi.com";
	std::string path;

	std::stringstream stream;
	stream << "GET /openupload HTTP/1.1\r\n";
	stream << "Accept: */*\r\n";
	stream << "access_token: " << g_token << "\r\n";
	stream << "device-id: openapi\r\n";
	stream << "role: openapi\r\n";
	stream << "filesize: " << nFileSize << "\r\n";
	stream << "filetype: mp4\r\n";
	stream << "User-Agent: Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; WOW64; Trident/6.0)\r\n";

	stream << "Host: upload.iqiyi.com\r\n";
	stream << g_cookie << "\r\n";
	stream << "Referer: http://www.iqiyi.com/upload?type=0&ufrm=web_iqiyi_uc\r\n";
	stream << "Connection: Keep-Alive\r\n\r\n";

	std::string request = stream.str();
	int nSize = request.size();
	if(http.socketHttp(host,request) == false)
	{	
		AfxMessageBox(_T("发包失败！"));
		return;
	}

	std::string strGetResult = http.GetLastRequestResult();

	std::wstring wStr;
	Transcode::UTF8_to_Unicode(strGetResult.c_str(), strGetResult.size(), wStr);
	const wchar_t* pData = wStr.c_str();
	OutputDebugString(pData);


	std::string upload_url;
	std::string file_id;
	std::string code;

	Json::Reader reader;
	Json::Value root;
	if (reader.parse(strGetResult, root) == false)
		return ;

	if (!root["data"].isNull())
	{
		if (!root["data"]["upload_url"].isNull())
			upload_url = root["data"]["upload_url"].asString();
		if (!root["data"]["file_id"].isNull())
			file_id = root["data"]["file_id"].asString();
		if (!root["code"].isNull())
			code = root["code"].asString();
	}
	if (upload_url.empty() || file_id.empty())
	{
		AfxMessageBox(_T("初始化数据失败！"));
		return ;
	}

	g_upload_url = upload_url;
	g_file_id = file_id;
}

unsigned int Rand32(unsigned int min, unsigned int max)
{
	return (unsigned int)((double)rand() / (double)RAND_MAX*(max - min + 1) + min);
}

std::string GetRamdomBoundary()
{
	std::string str = "-------------------------7";
	for (int i=0;i<11;i++)
	{
		char ch[2] = {0};
		ch[0] = (char)Rand32('a','z');
		str.append(ch);
	}
	return str;
}

void CiQIYI_Web_UploadDlg::OnBnClickedBtnUpload()
{
	char file[MAX_PATH+1] ={0};
	GetDlgItemTextA(m_hWnd,IDC_EDIT_PATH,file,MAX_PATH);
	g_strfile = file;
	std::string strFileName;
	std::string::size_type nPos = g_strfile.find_last_of("\\");
	if (nPos != std::string::npos)
	{
		strFileName = g_strfile.c_str()+nPos+1;
	}

	if (g_file_id.empty())
	{
		AfxMessageBox(_T("数据未初始化"));
		return;
	}

	std::string host;
	std::string path;
	std::size_t nPoslast = g_upload_url.find_last_of("/");
	std::size_t nHttp = g_upload_url.find_first_of("//");
	
	host.append(g_upload_url.c_str() + nHttp + 2, nPoslast - nHttp - 2);
	path.append(g_upload_url.c_str() + nPoslast);


	long nFileSize = 0;
	char * pFileBuf = ReadFileBuf(g_strfile.c_str(),&nFileSize);
	g_filesize = nFileSize;

	int nCount = g_filesize/65536;
	int nLeave = g_filesize%65536;

	for(int i =0;i< nCount+1;i++)
	{
		__int64 dwTime = GetSysTimeMicros()/1000;

		if (0)
		{
			std::string host = "upload0401.if.iqiyi.com";
			std::stringstream stream;
			stream << "OPTIONS /upload?t=" << dwTime << " HTTP/1.1\r\n";
			stream << "Host: "<< host << "\r\n";
			stream << "Connection: keep-alive\r\n";
			stream << "Access-Control-Request-Method: POST\r\n";
			stream << "User-Agent: Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; WOW64; Trident/6.0)\r\n";
			stream << g_cookie << "\r\n";
			stream << "Connection: Keep-Alive\r\n";
			stream << "Cache-Control: no-cache\r\n\r\n";
			std::string strOption = stream.str();

			if (http.socketHttp(host,strOption) == false)
			{
				AfxMessageBox(_T("发包失败！"));
				return;
			}

			std::string strGetResult = http.GetLastRequestResult();
			const char* pData = strGetResult.c_str();
		}

		/////////////////////////////////////////////////////////////////
		std::string strBoundary = GetRamdomBoundary();

		std::string strConBoundary = "--" + strBoundary;

		std::stringstream  contentStram;
		contentStram << strConBoundary  << "\r\n";
		contentStram << "Content-Disposition: form-data; name=\"file_id\"\r\n\r\n";
		contentStram << g_file_id << "\r\n";

		contentStram << strConBoundary << "\r\n";
		contentStram << "Content-Disposition: form-data; name=\"file_size\"\r\n\r\n";
		contentStram << g_filesize << "\r\n";

		contentStram << strConBoundary << "\r\n";
		contentStram << "Content-Disposition: form-data; name=\"range\"\r\n\r\n";

		int nStart = 0;
		nStart = i* 65536;
		int nLast = 0;
		int nSendSize = 0;
		if (i != nCount)
		{
			nLast = nStart + 65535;
			nSendSize = 65536;
		}
		else
		{
			if (nLeave == 0)
				break;
			nLast = nStart + nLeave - 1;
			nSendSize = nLeave;
		}
		
		//contentStram << "0-65535" << "\r\n";
		contentStram << nStart << "-" << nLast << "\r\n";

		contentStram << strConBoundary << "\r\n";
		contentStram << "Content-Disposition: form-data; name=\"file\"; filename=\"blob\"\r\n";
		//contentStram << "Content-Type: application/octet-stream\r\n\r\n";
		contentStram << "Content-Type: video/mp4\r\n\r\n";

		std::string strContentData = contentStram.str();
		// 添加文件数据
		// ....
		strContentData.append(pFileBuf+nStart,nSendSize);
		strContentData.append("\r\n");
		strContentData.append(strConBoundary);
		strContentData.append("--\r\n\r\n");
		int nContentLen = strContentData.size();


		//////////////////////////////////////////////////////////////
		// C++ 模拟POST，multipart/form-data格式上传
		std::stringstream streamPost;
		streamPost << "POST /upload?t=" << dwTime << " HTTP/1.1\r\n";
		streamPost << "Accept: */*\r\n";
		streamPost << "Content-Type: multipart/form-data; boundary="<<strBoundary<<"\r\n";
		streamPost << "User-Agent: Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; WOW64; Trident/6.0)\r\n";
		streamPost << "Host: "<< host << "\r\n";
		streamPost << "DNT: 1\r\n";
		streamPost << "Connection: keep-alive\r\n";
		streamPost << "Referer: http://www.iqiyi.com/upload?type=0&ufrm=web_iqiyi_uc\r\n";
		streamPost << "Cache-Control: no-cache\r\n";
		streamPost << g_cookie << "\r\n";
		streamPost << "Content-Length: " << nContentLen << "\r\n\r\n";

		std::string PostContent = streamPost.str();
		int nHeadSize = PostContent.size();
		PostContent.append(strContentData.c_str(),nContentLen);
		int nPostSize = PostContent.size();
		if (http.socketHttp(host,PostContent) == false)
		{
			AfxMessageBox(_T("发包失败！"));
			break;
		}
		else
		{
			std::string strGetResult = http.GetLastRequestResult();

			std::wstring wStr;
			Transcode::UTF8_to_Unicode(strGetResult.c_str(), strGetResult.size(), wStr);
			const wchar_t* pData = wStr.c_str();
			OutputDebugString(pData);
		}
		// 避免上传过快
		Sleep(10);
	}
	if (pFileBuf)
	{
		free(pFileBuf);
		pFileBuf = NULL;
	}
}

void CiQIYI_Web_UploadDlg::OnBnClickedBtnFinish()
{
	if (g_file_id.empty())
	{
		AfxMessageBox(_T("数据未初始化"));
		return;
	}

	std::string host = "upload.iqiyi.com";
	std::string path = "//uploadfinish";
	std::string content = "access_token=";
	content.append(g_token);
	content.append("&file_id=");
	content.append(g_file_id);
	if(http.getData1_1(host,path,content))
	{
		std::string strGetResult = http.GetLastRequestResult();

		std::wstring wStr;
		Transcode::UTF8_to_Unicode(strGetResult.c_str(), strGetResult.size(), wStr);
		const wchar_t* pData = wStr.c_str();
		OutputDebugString(pData);

		Json::Reader reader;
		Json::Value root;
		if (reader.parse(strGetResult,root))
		{
			bool bSuccess = false;
			if (!root["code"].isNull())
			{
				std::string  code = root["code"].asCString();
				if (code.find("A00000")!= std::string::npos)
					bSuccess = true;
			}
			if (bSuccess)
			{
				AfxMessageBox(_T("上传成功！"));
			}
			else
			{
					AfxMessageBox(__T("文件上传失败！（黑名单文件，无法上传到服务器）"));
			}
		}
	}
	else
	{	
		AfxMessageBox(_T("发包失败！"));
	}
}

void CiQIYI_Web_UploadDlg::OnBnClickedBtnSave()
{
	if (g_file_id.empty())
	{
		AfxMessageBox(_T("数据未初始化"));
		return;
	}

	std::string host = "www.iqiyi.com";
	std::string path = "/u/api/video/save_video_info";
	std::string content;
	std::string strPost ="flag=upload&tags=娱乐&video_title=bc77f185366dc19bddc7216272107388&behavior_type=1&channel_id=7&video_intro=小猪佩琪游泳的视频短片&private_policy=2&video_type=1&authCookie=34d02f7a06b05c996c33d9b224679a96&file_id=";
	
	Transcode::ANSI_to_UTF8(strPost.c_str(),strPost.size(),content);
	
	content.append(g_file_id);


	std::stringstream stream;
	stream << "POST " << path << " HTTP/1.1\r\n";
	stream << "Host: " << host << "\r\n";
	stream << "Accept: */*\r\n";
	stream << "Content-Type: application/x-www-form-urlencoded\r\n";
	stream << "User-Agent: Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; WOW64; Trident/6.0)\r\n";
	stream << "Accept-Encoding: gzip, deflate\r\n";
	stream << g_cookie << "\r\n";
	stream << "Connection: Keep-Alive\r\n";
	stream << "Referer: http://www.iqiyi.com/upload?type=0&ufrm=web_iqiyi_uc\r\n";
	stream << "Content-Length: " << content.size() << "\r\n\r\n";

	
	std::string request = stream.str();

	request.append(content.c_str(),content.size());

	if(http.socketHttp(host,request))
	{
		std::string strGetResult = http.GetLastRequestResult();

		std::wstring wStr;
		Transcode::UTF8_to_Unicode(strGetResult.c_str(), strGetResult.size(), wStr);
		const wchar_t* pData = wStr.c_str();
		OutputDebugString(pData);

		Json::Reader reader;
		Json::Value root;
		if (reader.parse(strGetResult,root))
		{
			bool bSuccess = false;
			if (!root["code"].isNull())
			{
				std::string  code = root["code"].asCString();
				if (code.find("A00000")!= std::string::npos)
					bSuccess = true;
			}
			if (bSuccess)
			{
				AfxMessageBox(_T("更新成功！"));
			}
			else
			{
				::MessageBox(m_hWnd,pData,_T("更新失败"),MB_OK|MB_ICONERROR);
			}
		}
	}
	else
	{	
		AfxMessageBox(_T("发包失败！"));
	}
}


void CiQIYI_Web_UploadDlg::OnBnClickedBtnDel()
{
	if (g_file_id.empty())
	{
		AfxMessageBox(_T("数据未初始化！"));
		return;
	}

	std::string host = "openapi.iqiyi.com"; 
	std::string path ="/api/file/delete";
	std::string content = "access_token=";
	content.append(g_token);
	content.append("&file_ids=");
	content.append(g_file_id);
	http.getData1_1(host,path,content);
	std::string strGetResult = http.GetLastRequestResult();
	std::wstring wStr;
	Transcode::UTF8_to_Unicode(strGetResult.c_str(), strGetResult.size(), wStr);
	const wchar_t* pData = wStr.c_str();

	std::wstring::size_type nFind = wStr.find(L"A00000");
	if(nFind != std::wstring::npos)
	{
		AfxMessageBox(_T("删除视频成功！"));
	}
	else
	{
		nFind = wStr.find(L"Q00001");
		if(nFind != std::wstring::npos)
			AfxMessageBox(_T("操作失败！"));
	}
	OutputDebugString(pData);
}

