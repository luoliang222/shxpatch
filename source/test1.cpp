// test1.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ShapeFont.h"
#include "fontchrlink.h"

class MyPenCallback : public PenCallback {
public:
	virtual void MoveTo(float x, float y) {

	}

	virtual void LineTo(float x, float y) {

	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	ShapeFont g_ShxParser;
	if(g_ShxParser.Load("../shxfont/gbcbig.SHX")){
		printf("已打开");
		printf("%i", g_ShxParser.Count());//字库包含多少字符
		//开始获得路径 例 '好' 字
		CString str("一");
		//获得 '好' 字编码
		char* kk=str.GetBuffer(0);
		wchar_t buf1[100];
		char key[8];
		MultiByteToWideChar(CP_OEMCP,0,str.GetBuffer(0),6,buf1,100);
		WideCharToMultiByte(932,0,buf1,3,key,6,NULL,NULL);
		unsigned short k1;
		memcpy(&k1,&kk[0],2);
		//获得shx字体路径
		MyPenCallback cb;
		fontchrlink* link = g_ShxParser.GetFromCode(k1);
		string patch = g_ShxParser.Patch(&cb, link, 20, 20, 1, 1);
		printf("%s",patch.c_str());
	}
	else{
		printf("字库打开错误");
	}

	return 0;
}

