
// MFC_ffmpeg_sdl_playerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MFC_ffmpeg_sdl_player.h"
#include "MFC_ffmpeg_sdl_playerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
extern "C"
{
#include "libavcodec\avcodec.h"
#include "libavformat\avformat.h"
#include "libswscale\swscale.h"
#include "SDL2\SDL.h"
}



//Thread Loop constant
#define SFM_REFRESH_EVENT (SDL_USEREVENT + 1)
#define SFM_BREAK_EVENT (SDL_USEREVENT + 2)
//信息
int thread_exit = 0;
int thread_pause = 0;
int sfp_refresh_thread(void * opaque)
{
	thread_exit = 0;
	thread_pause = 0;

	while (thread_exit == 0)
	{
		if (!thread_pause)
		{
			//事件
			SDL_Event event;
			//设置事件
			event.type = SFM_REFRESH_EVENT;

			//推送事件
			SDL_PushEvent(&event);
		}
		//设置等待事件
		SDL_Delay(40);
	}

	
	//Break;
	SDL_Event event;
	//退出的
	event.type = SFM_BREAK_EVENT;

	SDL_PushEvent(&event);
	//收到退出事件的处理
	thread_exit = 0;
	thread_pause = 0;

	return 0;
}




UINT ffmpeg_sdl_palyer(LPVOID lpParam)
{
	//================ffmpeg================
	AVFormatContext *pFormatCtx;
	int				i, videoindex;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;
	AVFrame			*pFrame, *pFrameYUV;
	uint8_t			*out_buffer;  //视频解码的缓冲区
	AVPacket		*packet;
	int				ret, got_picture;
	int				sum;

	//===============SDL=====================
	int screen_w, screen_h;
	SDL_Window	*screen;
	SDL_Renderer	*sdlRenderer; //视频的
	SDL_Texture		*sdlTexture;
	SDL_Rect		sdlRect;
	SDL_Thread		*video_tid;
	SDL_Event		event;

	struct SwsContext *img_convert_ctx;
	//==================filepath===============================
	//文件路径
	CMFC_ffmpeg_sdl_playerDlg *dlg = (CMFC_ffmpeg_sdl_playerDlg *)lpParam;
	char filepath[250] = { 0 };
	GetWindowTextA(dlg->m_url, (LPSTR)filepath, 250);
	//char filepath[] = "屌丝男士.mov";

	//注册ffmpeg所有的组件
	av_register_all();

	
	avformat_network_init();

	//获取封装格式的上下文
	pFormatCtx = avformat_alloc_context();

	//打开视频文件
	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0)
	{
		printf("Couldn't open input stream.\n");
		return -1;
	}
	//查找封装格式
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		printf("Couldn't find stream information.\n");
		return -1;
	}

	//获取视频流索引 
	videoindex = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
	{
		//判断是否视频的索引
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
			break;
		}
	}

	if (videoindex == -1)
	{
		printf("Didn't find a video stream.\n");
		return -1;
	}


	//获取视频解码器
	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	//查找视频解码器
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		printf("Codec not found.\n");
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL)<0) {
		printf("Could not open codec.\n");
		return -1;
	}

	//h264视频帧注册
	pFrame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();

	//开辟内存
	out_buffer = (uint8_t *)av_malloc(avpicture_get_size(
		AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height
	));

	//fill开辟内存
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P,
		pCodecCtx->width, pCodecCtx->height);

	//转yuv格式
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
		pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height,
		AV_PIX_FMT_YUV420P,
		SWS_BICUBIC, NULL, NULL, NULL);


	//初始化SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER))
	{
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}

	//SDL 2.0 Support for multiple windows

	//设置宽度 高度
	screen_w = pCodecCtx->width;
	screen_h = pCodecCtx->height;
	//创建窗口
	/*screen = SDL_CreateWindow("songli ffmpeg player sdl 1.0",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h, SDL_WINDOW_OPENGL);*/

	//获取播放窗口id -----IDC_SDL
	screen = SDL_CreateWindowFrom(dlg->GetDlgItem(IDC_SDL)->GetSafeHwnd());
	if (!screen)
	{
		printf("SDL: could not create window -exiting:%s\n", SDL_GetError());
		return -1;
	}

	//创建Renderer
	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);


	//IYUV : Y + U + V (3 planes);
	//YV12: Y + U + V (3 planes);
	sdlTexture = SDL_CreateTexture(sdlRenderer,
		SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,
		pCodecCtx->width, pCodecCtx->height);


	//设置屏幕宽度和高度
	sdlRect.x = 0;
	sdlRect.y = 0;
	sdlRect.w = screen_w;
	sdlRect.h = screen_h;

	//文件information
	av_dump_format(pFormatCtx, 0, filepath, 0);
	//给视频包开辟内存
	packet = (AVPacket *)av_malloc(sizeof(AVPacket));

	//创建Thread
	video_tid = SDL_CreateThread(sfp_refresh_thread, NULL, NULL);

	//FILE *fp_h264 = fopen("songli.h264", "wb+");
	//事件的Loop
	sum = 0;
	for (;;)
	{
		//Wait
		SDL_WaitEvent(&event);
		//判断事件
		if (event.type == SFM_REFRESH_EVENT)
		{
			if (av_read_frame(pFormatCtx, packet) >= 0)
			{

				//找到视频流索引
				if (packet->stream_index == videoindex)
				{
					//写入h264 文件中
					//fwrite(packet->data, 1, packet->size, fp_h264);

					ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
					if (ret < 0)
					{
						printf("Decode Error.\n");
						return -1;
					}
					if (got_picture)
					{
						sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data,
							pFrame->linesize, 0,
							pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

						printf("Decoded frame index:%d\n", sum);
						sum++;
						//=============SDL=================
						SDL_UpdateTexture(sdlTexture, NULL, pFrameYUV->data[0],
							pFrameYUV->linesize[0]);

						SDL_RenderClear(sdlRenderer);

						SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
						SDL_RenderPresent(sdlRenderer);
					}
				}
				av_free_packet(packet);
			}
			else
			{
				//Exit Thread;
				thread_exit = 1;
			}

		}
		else if (event.type == SDL_QUIT)
		{
			thread_exit = 1;
		}
		else if (event.type == SFM_BREAK_EVENT)
		{
			break;
		}
	}

	//fclose(fp_h264);
	sws_freeContext(img_convert_ctx);
	SDL_Quit();
	//FIX Small Bug
	//SDL Hide Window When it finished
	dlg->GetDlgItem(IDC_SDL)->ShowWindow(SW_SHOWNORMAL);
	av_frame_free(&pFrame);
	av_frame_free(&pFrameYUV);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);
	//system("pause");
	return EXIT_SUCCESS;
}

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFC_ffmpeg_sdl_playerDlg 对话框



CMFC_ffmpeg_sdl_playerDlg::CMFC_ffmpeg_sdl_playerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MFC_FFMPEG_SDL_PLAYER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFC_ffmpeg_sdl_playerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_URL, m_url);
}

BEGIN_MESSAGE_MAP(CMFC_ffmpeg_sdl_playerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_PLAYER, &CMFC_ffmpeg_sdl_playerDlg::OnBnClickedPlayer)
	ON_BN_CLICKED(IDC_ABOUT, &CMFC_ffmpeg_sdl_playerDlg::OnBnClickedAbout)
	ON_EN_CHANGE(IDC_URL, &CMFC_ffmpeg_sdl_playerDlg::OnEnChangeUrl)
	ON_BN_CLICKED(IDC_FILE, &CMFC_ffmpeg_sdl_playerDlg::OnBnClickedFile)
	ON_BN_CLICKED(IDC_PUASE, &CMFC_ffmpeg_sdl_playerDlg::OnBnClickedPuase)
	ON_BN_CLICKED(IDC_STOP, &CMFC_ffmpeg_sdl_playerDlg::OnBnClickedStop)
END_MESSAGE_MAP()


// CMFC_ffmpeg_sdl_playerDlg 消息处理程序

BOOL CMFC_ffmpeg_sdl_playerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFC_ffmpeg_sdl_playerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFC_ffmpeg_sdl_playerDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFC_ffmpeg_sdl_playerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}






void CMFC_ffmpeg_sdl_playerDlg::OnBnClickedPlayer()
{
	//AfxMessageBox("Hello world!");
	// TODO: Add your control notification handler code here
	
	//char buf[1024];
	//CString str;
	//str.Format(_T("%s"), avcodec_configuration());
	////strcpy(buf, avcodec_configuration());
	//AfxMessageBox(str);

	AfxBeginThread(ffmpeg_sdl_palyer, this);
}


void CMFC_ffmpeg_sdl_playerDlg::OnBnClickedAbout()
{
	// TODO: Add your control notification handler code here

	CAboutDlg Dlg;
	Dlg.DoModal();
}

//文件的路径显示
void CMFC_ffmpeg_sdl_playerDlg::OnEnChangeUrl()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

//选择文件
void CMFC_ffmpeg_sdl_playerDlg::OnBnClickedFile()
{
	// TODO: Add your control notification handler code here
	//选择文件的路径
	CString FilePathName;
	CFileDialog dlg(TRUE, NULL, NULL, NULL, NULL);///TRUE为OPEN对话框，FALSE为SAVE AS对话框 
	if (dlg.DoModal() == IDOK) {
		FilePathName = dlg.GetPathName();
		m_url.SetWindowText(FilePathName);
	}
}


void CMFC_ffmpeg_sdl_playerDlg::OnBnClickedPuase()
{
	// TODO: Add your control notification handler code here
	thread_pause = !thread_pause;
}


void CMFC_ffmpeg_sdl_playerDlg::OnBnClickedStop()
{
	// TODO: Add your control notification handler code here
	thread_exit = 1;
}
