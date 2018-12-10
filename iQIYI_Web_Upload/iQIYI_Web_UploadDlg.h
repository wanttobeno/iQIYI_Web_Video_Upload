// iQIYI_Web_UploadDlg.h : 头文件
//

#pragma once
#include "afxwin.h"


// CiQIYI_Web_UploadDlg 对话框
class CiQIYI_Web_UploadDlg : public CDialog
{
// 构造
public:
	CiQIYI_Web_UploadDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_IQIYI_WEB_UPLOAD_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnServer();
	afx_msg void OnBnClickedBtnUpload();
	afx_msg void OnBnClickedBtnFinish();
	afx_msg void OnBnClickedBtnSave();
	CEdit m_pEditPath;
	afx_msg void OnBnClickedBtnDel();
};
