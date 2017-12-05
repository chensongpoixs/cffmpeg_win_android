
// MFC_ffmpeg_sdl_playerDlg.h : 头文件
//

#pragma once
#include "afxwin.h"


// CMFC_ffmpeg_sdl_playerDlg 对话框
class CMFC_ffmpeg_sdl_playerDlg : public CDialogEx
{
// 构造
public:
	CMFC_ffmpeg_sdl_playerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFC_FFMPEG_SDL_PLAYER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedPlayer();
	afx_msg void OnBnClickedAbout();
	afx_msg void OnEnChangeUrl();
	afx_msg void OnBnClickedFile();
	CEdit m_url;
	afx_msg void OnBnClickedPuase();
	afx_msg void OnBnClickedStop();
};
