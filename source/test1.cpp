// test1.cpp : �������̨Ӧ�ó������ڵ㡣
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
		printf("�Ѵ�");
		printf("%i", g_ShxParser.Count());//�ֿ���������ַ�
		//��ʼ���·�� �� '��' ��
		CString str("һ");
		//��� '��' �ֱ���
		char* kk=str.GetBuffer(0);
		wchar_t buf1[100];
		char key[8];
		MultiByteToWideChar(CP_OEMCP,0,str.GetBuffer(0),6,buf1,100);
		WideCharToMultiByte(932,0,buf1,3,key,6,NULL,NULL);
		unsigned short k1;
		memcpy(&k1,&kk[0],2);
		//���shx����·��
		MyPenCallback cb;
		fontchrlink* link = g_ShxParser.GetFromCode(k1);
		string patch = g_ShxParser.Patch(&cb, link, 20, 20, 1, 1);
		printf("%s",patch.c_str());
	}
	else{
		printf("�ֿ�򿪴���");
	}

	return 0;
}

