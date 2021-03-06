
// MFCApplication1Dlg.cpp: 实现文件
//

#include "stdafx.h"
#include <fstream>
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include "afxdialogex.h"
#include "../../source/ShapeFont.h"
#include "../../source/fontchrlink.h"
#include "../../lib_json/json/json.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


ShapeFont g_ShxParser;

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


// CMFCApplication1Dlg 对话框



CMFCApplication1Dlg::CMFCApplication1Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCAPPLICATION1_DIALOG, pParent)
	, m_strFileName(_T(""))
	, m_bMergeMode(FALSE)
	, m_strDesc(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//	DDX_Control(pDX, IDC_CUSTOM1, m_Canvas);
	DDX_Text(pDX, IDC_EDIT1, m_strFileName);
	DDX_Text(pDX, IDC_EDIT2, m_strText);
	DDX_Control(pDX, IDC_CHECK1, merge);
	DDX_Check(pDX, IDC_CHECK1, m_bMergeMode);
	DDX_Text(pDX, IDC_DESC, m_strDesc);
}

BEGIN_MESSAGE_MAP(CMFCApplication1Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_FILE_OPEN, &CMFCApplication1Dlg::OnBnClickedFileOpen)
	ON_EN_CHANGE(IDC_EDIT2, &CMFCApplication1Dlg::OnEnChangeEdit2)
	ON_EN_CHANGE(IDC_EDIT1, &CMFCApplication1Dlg::OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCApplication1Dlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_CHECK1, &CMFCApplication1Dlg::OnBnClickedCheck1)
END_MESSAGE_MAP()


// CMFCApplication1Dlg 消息处理程序

BOOL CMFCApplication1Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	ShowWindow(SW_MAXIMIZE);

	ShowWindow(SW_MINIMIZE);

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFCApplication1Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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


class MyPenCallback : public PenCallback {
public:
	CDC * pdc;
	CPen pen;
	bool isDrawing;
	double scale;
	int ox, oy;
	MyPenCallback(CDC *dc) :pdc(dc) {
		pdc->SetViewportOrg(300, 800);
		pdc->SetMapMode(MM_LOMETRIC);
		isDrawing = false;
		if (dc) {
			pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
		}
		scale = 5;
		ox = 0;
		oy = 0;
	}

	void paint(CString str) {
		// 横线 0，长度 1000
		pdc->MoveTo(0, oy);
		pdc->LineTo(1000, oy);

		// 横线 above，长度 200
		int y = g_ShxParser.GetAbove()*scale;
		pdc->MoveTo(ox - 100, oy + y);
		pdc->LineTo(ox + 100, oy + y);

		// 横线 below，长度 300
		y = -g_ShxParser.GetBelow()*scale;
		pdc->MoveTo(ox - 150, oy + y);
		pdc->LineTo(ox + 150, oy + y);

		// 竖线 字宽，长度 200
		int w = g_ShxParser.Width()*scale;
		pdc->MoveTo(ox + w, oy - 100);
		pdc->LineTo(ox + w, oy + 100);

		// 竖线 0，长度200
		pdc->MoveTo(ox, oy - 100);
		pdc->LineTo(ox, oy + 100);

		//获得字编码
		if (str.GetLength() == 0)
			return;
		char* kk = str.GetBuffer(0);
		short k1;
		if (kk[0] >= 0)
			k1 = kk[0];
		else
			k1 = *((short *)kk);

		//获得shx字体路径
		fontchrlink* link = g_ShxParser.GetFromCode(k1);
		if (!link)
			return;

		double ss = 1;
		std::string patch = g_ShxParser.Patch(this, link, 0, 0, ss, ss);
		CharData* pData = new CharData();
		link->ShapeCreateVec(pData);

		std::ofstream log("shx.log", ios::app);
		log << str << "  =====>" << std::endl << patch.c_str() << std::endl;
	}

	virtual void MoveTo(float x, float y)
	{
		setCurrent(x, y);
		isDrawing = false;
	}

	virtual void LineTo(float x, float y) {
		if (!isDrawing) {
			// 如果未进入画线状态，则加入起点坐标
			if (pdc)
			{
				pdc->SelectObject(&pen);
				pdc->MoveTo(ox + curX * scale, oy + curY * scale);
			}

			isDrawing = true;
		}

		// 设置当前坐标
		setCurrent(x, y);

		// 画线
		if (pdc)
		{
			pdc->SelectObject(&pen);
			pdc->LineTo(ox + curX * scale, oy + curY * scale);
		}
	}

};


// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCApplication1Dlg::OnPaint()
{
	CPaintDC dc(this); // 用于绘制的设备上下文
	if (IsIconic())
	{
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

	MyPenCallback cb(&dc);
	cb.paint(m_strText);
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCApplication1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMFCApplication1Dlg::OnBnClickedFileOpen()
{
	UpdateData();

	// TODO: 在此添加控件通知处理程序代码
	CFileDialog findFileDlg(TRUE,  // TRUE是创建打开文件对话框，FALSE则创建的是保存文件对话框
					".shx",  // 默认的打开文件的类型
					(LPCTSTR)m_strFileName,  // 默认打开的文件名
					OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,				// 打开只读文件
					"CAD字体文件(*.shx)|*.shx|所有文件 (*.*)|*.*||"		// 所有可以打开的文件类型
	);

	if (IDOK == findFileDlg.DoModal())
	{
		CString m_FilePath = findFileDlg.GetPathName();  // 取出文件路径
		m_strFileName = m_FilePath;
		UpdateData(false);

		// 打开字体文件
		if (!g_ShxParser.Load((LPCTSTR)m_FilePath))
		{
			m_strDesc = ("文件打开错误");
			UpdateData(false);
			return;
		}

		CString msg;
		msg.Format("打开字库文件成功，字库中包含文字 %i 个", g_ShxParser.Count());
		m_strDesc = msg;
		UpdateData(false);

		PenCallback cb;
		// 统计字体的轮廓范围
		for (int i = 0; i < g_ShxParser.Count(); i++)
		{
			fontchrlink *pItem = g_ShxParser.Get(i);
			if (NULL == pItem)
				continue;

			char* ascChar = (char *)&pItem->code;
			char ascBuf[3];
			ascBuf[0] = ascChar[0];
			ascBuf[1] = ascChar[1];
			ascBuf[2] = 0;
			wchar_t wbuf[2];
			MultiByteToWideChar(CP_OEMCP, 0, ascBuf, 3, wbuf, 2);
			g_ShxParser.Patch(&cb, pItem, 0, 0, 1, 1);
		}

		m_strDesc.Format("%s, [%lf, %lf, %lf, %lf]", m_strDesc, cb.minX, cb.minY, cb.maxX, cb.maxY);
		UpdateData(false);
	}
}

// 修改了预览文字
void CMFCApplication1Dlg::OnEnChangeEdit2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	UpdateData();
	Invalidate(TRUE);
	return;

	if (m_strText.GetLength() == 0)
		return;

	// 预览 m_strText
	//开始获得路径 例 '好' 字
	CString str = m_strText;
	//获得基于ASIC的编码
	char* ascChar = str.GetBuffer(0);
	short ascCode;
	if (ascChar[0] >= 0)
		ascCode = ascChar[0];	// 低位是ANSC码，仅需一个字节
	else
	{// 模拟计算 unicode 代码
		ascCode = *((short *)ascChar);
	}

	// 转换为unicode
	wchar_t wbuf[2];
	MultiByteToWideChar(CP_OEMCP, 0, ascChar, 3, wbuf, 2);
	CString key;
	key.Format("h_%d", wbuf[0]);

	//获得shx字体路径
	MyPenCallback cb(NULL);
	fontchrlink* link = g_ShxParser.GetFromCode(ascCode);
	std::string patch = g_ShxParser.Patch(&cb, link, 1000, 1000, 10, 10);
	// printf("%s", patch.c_str());

	std::ofstream log("shx.log", ios::app);
	log << str  << ":" << (LPCTSTR)key << " =====>" << std::endl << patch.c_str() << std::endl;

	Invalidate(TRUE);
}

void CMFCApplication1Dlg::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}

#define AdvanceX "AdvanceX"
#define AdvanceY "AdvanceY"
#define bb "bb"
#define Segment "Segments"
#define Items "Items"
#define FontType "FontType"
#define Ascent "Ascent"
#define Descent "Descent"
#define Shapes "Shapes"
#define JSON_POINTS "Points"

class JsonDrawer : public PenCallback {
public:
	Json::Value jsonSegments;
	bool isDrawing;
	JsonDrawer(){
		Json::Value items;
		jsonSegments[Items] = items;
		isDrawing = false;
	}

	virtual void MoveTo(float x, float y)
	{
		setCurrent(x, y);
		isDrawing = false;
	}

	virtual void LineTo(float x, float y) {
		if (!isDrawing) {
			// 如果未进入画线状态，则加入起点坐标
			Json::Value point;
			point.append((int)curX);
			point.append((int)curY);
			Json::Value Points;
			Points[JSON_POINTS][Items].append(point);
			jsonSegments[Items].append(Points);
			isDrawing = true;
		}

		// 设置当前坐标
		setCurrent(x, y);

		// 画线
		Json::Value point;
		point.append((int)curX);
		point.append((int)curY);
		Json::Value& items = jsonSegments[Items];
		items[items.size() - 1][JSON_POINTS][Items].append(point);
	}
};

void getBox(PenCallback& cb) {
	// 统计字体的轮廓范围
	for (int i = 0; i < g_ShxParser.Count(); i++)
	{
		fontchrlink *pItem = g_ShxParser.Get(i);
		if (NULL == pItem)
			continue;

		char* ascChar = (char *)&pItem->code;
		char ascBuf[3];
		ascBuf[0] = ascChar[0];
		ascBuf[1] = ascChar[1];
		ascBuf[2] = 0;
		wchar_t wbuf[2];
		MultiByteToWideChar(CP_OEMCP, 0, ascBuf, 3, wbuf, 2);
		g_ShxParser.Patch(&cb, pItem, 0, 0, 1, 1);
	}
}

void CMFCApplication1Dlg::OnBnClickedButton1()
{
	UpdateData();

	// 获取字体最大外框
	PenCallback BoxCB;
	getBox(BoxCB);
	double widthWord = BoxCB.maxX - BoxCB.minX;

	// 将范围限定到 Y[0 ~ 1023]，居中
	double a = 1024 / (BoxCB.maxY - BoxCB.minY);
	double bY = 0;	// a * BoxCB.minY;
	double bX = 0;	// -a * BoxCB.minX;

	Json::Value defaultItem;
	Json::Value &ShapesRoot = defaultItem;
	if (m_bMergeMode) {
		Json::Reader reader;
		std::ifstream in("font.json");
		Json::Value root;
		if (!reader.parse(in, root, false))
		{
			MessageBox("打开font.json失败");
		}
		ShapesRoot = root[Shapes];
	}

	/*
	"root": {
	    "FontType": 0,
		"Ascent": 1492,
		"Descent": 434,
		"Shapes": {
		  "Items": [{
			"AdvanceX": 0,
			"AdvanceY": 0,
			"bb": [0, -434, 0, 1492],
			"Segments": {
			  "Items": [{
				"Points": {
				  "Items": [[0, 1492], [0, 1492], [0, -434], [0, -434], [0, 1492]]
				}
			  }]
			}
		  },
		....
	}
	*/

	double minY = INT_MAX;
	double maxY = -INT_MAX;
	int index = ShapesRoot[Items].size();
	for (int i = 0; i < g_ShxParser.Count(); i++)
	{
		fontchrlink *pItem = g_ShxParser.Get(i);
		if (NULL == pItem)
			continue;

		char* ascChar = (char *)&pItem->code;
		char ascBuf[3];
		ascBuf[0] = ascChar[0];
		ascBuf[1] = ascChar[1];
		ascBuf[2] = 0;
		wchar_t wbuf[2];
		MultiByteToWideChar(CP_OEMCP, 0, ascBuf, 3, wbuf, 2);

		CString key;
		key.Format("h_%d", wbuf[0]);
		Json::Value old = ShapesRoot[(LPCTSTR)key];
		if (old == Json::Value::null)
			ShapesRoot[(LPCTSTR)key] = index ++;
		else {// 目标已存在，则跳过
			continue;
		}

		JsonDrawer cb;
		std::string patch = g_ShxParser.Patch(&cb, pItem, bX, bY, a, a);
		std::ofstream log("shx.log", ios::app);
		log << ascBuf << ":" << (LPCTSTR)key << "  =====>" << std::endl << patch.c_str() << std::endl;

		Json::Value jsonItem;
		CharData* pData = new CharData();
		pItem->ShapeCreateVec(pData);
		if (cb.maxX < cb.minX)
			cb.maxX = cb.minX = 0;
		if (cb.maxY < cb.minY)
			cb.maxY = cb.minY = 0;

		double advanceX = cb.maxX - cb.minX;	//pData->m_charInfo.gmCellIncX - pData->m_charInfo.gmptGlyphOrigin.x;
		double advanceY = cb.maxY - cb.minY;	//pData->m_charInfo.gmCellIncY - pData->m_charInfo.gmptGlyphOrigin.y;
		minY = min(minY, advanceY);
		maxY = max(maxY, advanceY);
		Json::Value jsonBB;
		jsonBB.append((int)cb.minX);	// pData->m_charInfo.gmptGlyphOrigin.x);
		jsonBB.append((int)cb.minY);	// pData->m_charInfo.gmptGlyphOrigin.y);
		jsonBB.append((int)cb.maxX);	// pData->m_charInfo.gmBlackBoxX);
		jsonBB.append((int)cb.maxY);	// pData->m_charInfo.gmBlackBoxY);

		jsonItem[AdvanceX] = (int)(advanceX + widthWord*a*0.2);
		jsonItem[AdvanceY] = 0;
		jsonItem[bb] = jsonBB;
		jsonItem[Segment] = cb.jsonSegments;

		ShapesRoot[Items].append(jsonItem);
	}

	Json::Value root;
	root[FontType] = Json::Value(0);
	root[Ascent] = Json::Value((int)maxY);
	root[Descent] = Json::Value((int)minY);
	root[Shapes] = ShapesRoot;

	ofstream jsonOut("font.json");
	Json::StreamWriterBuilder builder;
	builder["indentation"] = "";	// 无缩进
	std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
	writer->write(root, &jsonOut);
	jsonOut << std::endl;
}


void CMFCApplication1Dlg::OnBnClickedCheck1()
{
	// TODO: 在此添加控件通知处理程序代码
}
