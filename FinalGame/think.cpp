#include <windows.h>
#include <stdio.h>

class Action
{
public:

	int frameAll; //总帧数
	int frameNow; //当前帧数
	bool isPlaying; //是否正在播放

	Action() {
		frameAll = 0;
		frameNow = 0;
		isPlaying = false;
	}

	Action(int frameAll) {
		this->frameAll = frameAll;
		frameNow = 0;
		isPlaying = false;
	}


	~Action() {

	}

private:

};


class Character
{
public:
	Character() {
		x = 0;
		y = 600;
		init_x = 0;
		init_y = 600;
		dir = 0;
		dir_way = 0;
		num = 0;
		vx = 0;
		vy = 0;

		//设定人物动作帧数
		action[0].frameAll = 6;
		action[1].frameAll = 8;
		action[2].frameAll = 6;
		action[3].frameAll = 12;

		//播放待机动作
		action[0].isPlaying = true;

		frame_action_num[0] = 6;
		frame_action_num[1] = 8;
		frame_action_num[2] = 6;
		frame_action_num[3] = 12;
	}

	~Character() {}


	int		num, dir, x, y;       //x，y变量为人物贴图坐标，dir为人物移动方向，以0，1，2，3代表人物上，下，左，右方向上的移动：num为连续贴图中的小图编号
	int		init_x, init_y;
	int     dir_way;              //dir_way为人物移动方向，1右，-1左
	int     vx, vy;               //vx，vy为人物移动速度
	Action  action[10];          //action为人物动作，0为静止，1为移动，2为攻击,3为跳跃
	int     frame_action_num[10];     //frame_action_num为人物动作帧数


	int		hp;					  //hp为人物生命值


	HBITMAP Hb_walk, Hb_idle, Hb_attack, Hb_jump;
};


//全局变量声明
HDC		hdc, mdc, bufdc;
HWND	hWnd;
DWORD	tPre, tNow;
int window_width = 1024, window_height = 768;
int scene_width = 1229, scene_height = 922;
int camera_x = 0, camera_y = 0;

Character p[2];
HBITMAP Hb_bg,Hb_bgBack;

POINT   cow_pos[2];

//全局函数声明
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
void				MyPaint(HDC hdc);
void MyTransparentBlt(HDC hdcDest,      // 目标DC
	int nXOriginDest,   // 目标X偏移
	int nYOriginDest,   // 目标Y偏移
	int nWidthDest,     // 目标宽度
	int nHeightDest,    // 目标高度
	HDC hdcSrc,         // 源DC
	int nXOriginSrc,    // 源X起点
	int nYOriginSrc,    // 源Y起点
	int nWidthSrc,      // 源宽度
	int nHeightSrc,     // 源高度
	UINT crTransparent  // 透明色,COLORREF类型
);

//****WinMain函数，程序入口点函数***********************
int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{
	MSG msg;

	MyRegisterClass(hInstance);

	//初始化
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	GetMessage(&msg, NULL, NULL, NULL);//初始化msg              
	//游戏循环
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			tNow = GetTickCount();
			if (tNow - tPre >= 40)
			{
				MyPaint(hdc);
			}
		}
	}
	return msg.wParam;
}

//****设计一个窗口类****
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "canvas";
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex);
}

//****初始化函数****
// 加载位图并设定各种初始值
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hWnd = CreateWindow("canvas", "绘图窗口", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{
		return FALSE;
	}

	MoveWindow(hWnd, 10, 10, window_width, window_height, true);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	////////////////////////////////////////
	HBITMAP fullmap;
	hdc = GetDC(hWnd);
	mdc = CreateCompatibleDC(hdc);
	bufdc = CreateCompatibleDC(hdc);

	//建立空的位图并置入mdc中
	fullmap = CreateCompatibleBitmap(hdc, window_width, window_height);
	SelectObject(mdc, fullmap);

	cow_pos[0].x = 100;
	cow_pos[0].y = 300 - 80;
	cow_pos[1].x = 270;
	cow_pos[1].y = 350 - 80;

	//载入各连续移动位图及背景图
	p[0].Hb_walk = (HBITMAP)LoadImage(NULL, "Actor/walk.bmp", IMAGE_BITMAP, 1024, 128, LR_LOADFROMFILE);
	p[0].Hb_idle = (HBITMAP)LoadImage(NULL, "Actor/idle.bmp", IMAGE_BITMAP, 768, 128, LR_LOADFROMFILE);
	p[0].Hb_attack = (HBITMAP)LoadImage(NULL, "Actor/attack_1.bmp", IMAGE_BITMAP, 768, 128, LR_LOADFROMFILE);
	p[0].Hb_jump = (HBITMAP)LoadImage(NULL, "Actor/jump.bmp", IMAGE_BITMAP, 1536, 128, LR_LOADFROMFILE);
	Hb_bg = (HBITMAP)LoadImage(NULL, "BackGroundFarm.bmp", IMAGE_BITMAP, scene_width, scene_height, LR_LOADFROMFILE);
	Hb_bgBack = (HBITMAP)LoadImage(NULL, "Cloud.bmp", IMAGE_BITMAP, 1366, 768, LR_LOADFROMFILE);
	MyPaint(hdc);

	return TRUE;
}

//****自定义绘图函数****
// 
// 
void actionEnd()
{
	p[0].action[0].frameNow %= 2 * p[0].action[0].frameAll;
	p[0].action[1].frameNow %= 2 * p[0].action[1].frameAll;
	for (int i = 2; i < 10; i++)
	{
		if (p[0].action[i].frameNow == 2 * p[0].action[i].frameAll)
		{
			p[0].action[i].frameNow = 0;
			p[0].action[i].isPlaying = false;
		}
	}
}

// 人物贴图坐标修正及窗口贴图
void MyPaint(HDC hdc)
{
	if (p[0].action[1].isPlaying == true)
	{
		if (p[0].x + p[0].vx < 0)
		{
			p[0].vx = 0;
		}
		else if (p[0].x + p[0].vx > scene_width - 128)
		{
			p[0].vx = 0;
		}
		else
			p[0].x += p[0].vx;
		if (p[0].y + p[0].vy < 200) //到顶了
		{
			p[0].vy = 0;
		}
		else if (p[0].y + p[0].vy > 310)
		{
			p[0].vy = 0;
		}
		else
			p[0].y += p[0].vy;

		if (camera_x + p[0].vx < 0)
		{
			camera_x = 0;
		}
		else if (camera_x + p[0].vx + window_width > scene_width)
		{
			camera_x = scene_width - window_width;
		}
		else
		{
			camera_x += p[0].vx;
		}
	}
	else if (p[0].action[0].isPlaying == 1)
	{
		p[0].vx = 0;
		p[0].vy = 0;
	}
	int w, h;

	//先在mdc中贴上背景图
	SelectObject(bufdc, Hb_bgBack);
	BitBlt(mdc, 0, 0, window_width, window_height, bufdc, 0, 0, SRCCOPY);
	SelectObject(bufdc, Hb_bg);
	//BitBlt(mdc, 0, 0, window_width, window_height, bufdc, camera_x, 0, SRCCOPY);
	MyTransparentBlt(mdc, 0, 0, window_width, window_height, bufdc, camera_x, scene_height - window_height, window_width, window_height, RGB(255, 255, 255));

	//按照目前的移动方向取出对应人物的连续走动图，并确定截取人物图的宽度与高度
	w = 128;
	h = 128;
	//按照目前的X，Y的值在mdc上进行透明贴图，然后显示在窗口画面上


	//跳跃动作优先
	if (p[0].action[3].isPlaying == true)
	{
		
		if (p[0].action[3].frameNow < 12) {
			p[0].y -= 10;
		}
		else {
			p[0].y += 10;
		}
		//跳跃时进行攻击
		if (p[0].action[2].isPlaying == true)
		{
			SelectObject(bufdc, p[0].Hb_attack);
			if (p[0].dir_way == -1)
				MyTransparentBlt(mdc, p[0].x - camera_x, p[0].y - camera_y, w, h, bufdc, (p[0].action[2].frameNow / 2 + 1) * w - 1, 0, w * p[0].dir_way, h, RGB(255, 255, 255));
			else
				MyTransparentBlt(mdc, p[0].x - camera_x, p[0].y - camera_y, w, h, bufdc, p[0].action[2].frameNow / 2 * w, 0, w, h, RGB(255, 255, 255));
			p[0].action[2].frameNow++;
		}
		else
		{
			SelectObject(bufdc, p[0].Hb_jump);
			if (p[0].dir_way == -1)
				MyTransparentBlt(mdc, p[0].x - camera_x, p[0].y - camera_y, w, h, bufdc, (p[0].action[3].frameNow / 2 + 1) * w - 1, 0, w * p[0].dir_way, h, RGB(255, 255, 255));
			else
				MyTransparentBlt(mdc, p[0].x - camera_x, p[0].y - camera_y, w, h, bufdc, p[0].action[3].frameNow / 2 * w, 0, w, h, RGB(255, 255, 255));
		}
		p[0].action[3].frameNow++;
	}
	else
	{

		if (p[0].action[2].isPlaying == true)
		{
			SelectObject(bufdc, p[0].Hb_attack);
			if (p[0].dir_way == -1)
				MyTransparentBlt(mdc, p[0].x - camera_x, p[0].y - camera_y, w, h, bufdc, (p[0].action[2].frameNow / 2 + 1) * w - 1, 0, w * p[0].dir_way, h, RGB(255, 255, 255));
			else
				MyTransparentBlt(mdc, p[0].x - camera_x, p[0].y - camera_y, w, h, bufdc, p[0].action[2].frameNow / 2 * w, 0, w, h, RGB(255, 255, 255));
			p[0].action[2].frameNow++;
		}
		else if (p[0].action[1].isPlaying == true)
		{
			SelectObject(bufdc, p[0].Hb_walk);
			if (p[0].dir_way == -1)
				MyTransparentBlt(mdc, p[0].x - camera_x, p[0].y - camera_y, w, h, bufdc, (p[0].action[1].frameNow / 2 + 1) * w - 1, 0, w * p[0].dir_way, h, RGB(255, 255, 255));
			else
				MyTransparentBlt(mdc, p[0].x - camera_x, p[0].y - camera_y, w, h, bufdc, p[0].action[1].frameNow / 2 * w, 0, w, h, RGB(255, 255, 255));
			p[0].action[1].frameNow++;
		}
		else if (p[0].action[0].isPlaying == true)
		{
			SelectObject(bufdc, p[0].Hb_idle);
			if (p[0].dir_way == -1)
				MyTransparentBlt(mdc, p[0].x - camera_x, p[0].y - camera_y, w, h, bufdc, (p[0].action[0].frameNow / 2 + 1) * w - 1, 0, w * p[0].dir_way, h, RGB(255, 255, 255));
			else
				MyTransparentBlt(mdc, p[0].x - camera_x, p[0].y - camera_y, w, h, bufdc, p[0].action[0].frameNow / 2 * w, 0, w, h, RGB(255, 255, 255));
			p[0].action[0].frameNow++;
		}
	}
	

	BitBlt(hdc, 0, 0, window_width, window_height, mdc, 0, 0, SRCCOPY);

	tPre = GetTickCount();         //记录此次绘图时间

	p[0].num++;			//静止也在动
	actionEnd();
}

//****消息处理函数***********************************
// 1.按下【Esc】键结束程序
// 2.按下方向键重设贴图坐标
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{

	case WM_KEYDOWN:	     //按下键盘消息
		//判断按键的虚拟键码
		switch (wParam)
		{
		case VK_ESCAPE:           //按下【Esc】键
			PostQuitMessage(0);  //结束程序
			break;
		
		case 'A':			  //按下【←】键
			p[0].vx = -10;
			p[0].dir = 2;
			p[0].dir_way = -1;
			TextOut(hdc, 0, 0, "A", 1);
			p[0].action[1].isPlaying = true;
			if (p[0].action[1].isPlaying == false)
			{
				p[0].action[1].frameNow = 0;
			}
			break;
		case 'D':			   //按下【→】键
			p[0].vx = 10;
			p[0].dir = 3;
			p[0].dir_way = 1;

			p[0].action[1].isPlaying = true;
			if (p[0].action[1].isPlaying == false)
			{
				p[0].action[1].frameNow = 0;
			}
			break;
		case 'J':					//按下【J】键
			if (p[0].action[2].isPlaying == false)
			{
				p[0].action[2].isPlaying = true;
				p[0].action[2].frameNow = 0;
				if (p[0].dir_way == 1)
				{
					cow_pos[0].x -= 64;
					cow_pos[1].x -= 64;
				}
				else
				{
					cow_pos[0].x += 64;
					cow_pos[1].x += 64;
				}
				if (p[0].x >= cow_pos[0].x && p[0].x <= cow_pos[1].x && p[0].y >= cow_pos[0].y && p[0].y <= cow_pos[1].y)
				{
					Hb_bg = (HBITMAP)LoadImage(NULL, "BackGroundGrassLand.bmp", IMAGE_BITMAP, window_width, window_height, LR_LOADFROMFILE);
				}
			}
			break;
		case 'K':					//按下【k】键
			if (p[0].action[3].isPlaying == false)
			{
				p[0].action[3].isPlaying = true;
				p[0].action[3].frameNow = 0;
			}
			break;	
		}
		break;
	case WM_DESTROY:			    	//窗口结束消息
		int i;

		DeleteDC(mdc);
		DeleteDC(bufdc);
		for (i = 0;i < 4;i++)
			DeleteObject(p[0].Hb_walk);
		DeleteObject(Hb_bg);
		ReleaseDC(hWnd, hdc);

		PostQuitMessage(0);
		break;
	case WM_KEYUP:
		switch (wParam)
		{
		case 'A':			  //按下【←】键
			if (p[0].action[1].isPlaying == true)
			{
				p[0].action[1].isPlaying = false;
				//vx = 0;
			}
			break;
		case 'D':			   //按下【→】键
			if (p[0].action[1].isPlaying == true)
			{
				p[0].action[1].isPlaying = false;
				//vx = 0;
			}
			break;
		}
		break;
	default:							//其他消息
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void MyTransparentBlt(HDC hdcDest,      // 目标DC
	int nXOriginDest,   // 目标X偏移
	int nYOriginDest,   // 目标Y偏移
	int nWidthDest,     // 目标宽度
	int nHeightDest,    // 目标高度
	HDC hdcSrc,         // 源DC
	int nXOriginSrc,    // 源X起点
	int nYOriginSrc,    // 源Y起点
	int nWidthSrc,      // 源宽度
	int nHeightSrc,     // 源高度
	UINT crTransparent  // 透明色,COLORREF类型
)
{
	HBITMAP hOldImageBMP, hImageBMP = CreateCompatibleBitmap(hdcDest, nWidthDest, nHeightDest);	// 创建兼容位图
	HBITMAP hOldMaskBMP, hMaskBMP = CreateBitmap(nWidthDest, nHeightDest, 1, 1, NULL);			// 创建单色掩码位图
	HDC		hImageDC = CreateCompatibleDC(hdcDest);
	HDC		hMaskDC = CreateCompatibleDC(hdcDest);
	hOldImageBMP = (HBITMAP)SelectObject(hImageDC, hImageBMP);
	hOldMaskBMP = (HBITMAP)SelectObject(hMaskDC, hMaskBMP);

	// 将源DC中的位图拷贝到临时DC中
	if (nWidthDest == nWidthSrc && nHeightDest == nHeightSrc)
		BitBlt(hImageDC, 0, 0, nWidthDest, nHeightDest, hdcSrc, nXOriginSrc, nYOriginSrc, SRCCOPY);
	else
		StretchBlt(hImageDC, 0, 0, nWidthDest, nHeightDest,
			hdcSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, SRCCOPY);

	// 设置透明色
	SetBkColor(hImageDC, crTransparent);

	// 生成透明区域为白色，其它区域为黑色的掩码位图
	BitBlt(hMaskDC, 0, 0, nWidthDest, nHeightDest, hImageDC, 0, 0, SRCCOPY);

	// 生成透明区域为黑色，其它区域保持不变的位图
	SetBkColor(hImageDC, RGB(0, 0, 0));
	SetTextColor(hImageDC, RGB(255, 255, 255));
	BitBlt(hImageDC, 0, 0, nWidthDest, nHeightDest, hMaskDC, 0, 0, SRCAND);

	// 透明部分保持屏幕不变，其它部分变成黑色
	SetBkColor(hdcDest, RGB(0xff, 0xff, 0xff));
	SetTextColor(hdcDest, RGB(0, 0, 0));
	BitBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hMaskDC, 0, 0, SRCAND);

	// "或"运算,生成最终效果
	BitBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hImageDC, 0, 0, SRCPAINT);

	SelectObject(hImageDC, hOldImageBMP);
	DeleteDC(hImageDC);
	SelectObject(hMaskDC, hOldMaskBMP);
	DeleteDC(hMaskDC);
	DeleteObject(hImageBMP);
	DeleteObject(hMaskBMP);
}

