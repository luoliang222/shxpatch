#pragma once
#include <string>
#include "Array.h"
using std::string;
class fontchrlink;

class PenCallback {
public:
	double minX, maxX, minY, maxY;
	double curX, curY;
	PenCallback() {
		minX = minY = INT_MAX;
		maxX = maxY = -INT_MAX;
		curX = curY = 0;
	}

	virtual void setCurrent(float x, float y) {
		curX = x;
		curY = y;
		minX = min(minX, x);
		maxX = max(maxX, x);
		minY = min(minY, y);
		maxY = max(maxY, y);
	}

	virtual void MoveTo(float x, float y) { setCurrent(x, y); };
	virtual void LineTo(float x, float y) { setCurrent(x, y); };
};

//CAD字体读写、解析类
class  ShapeFont
{
public:
	ShapeFont(void);
	~ShapeFont(void);
	bool Load(const char* fileName);
	void Display(CDC* pDC,unsigned short charCode,int orgX,int orgY);//测试字体坐标解析
	fontchrlink* GetFromCode(unsigned short code);//查找字体
	char GetFontType(){return m_type;}
	fontchrlink* GetFromName(const char* symbolName);//查找字体
	int FindCodeByName(const char* name);
	int Count(){return m_data.Count();}
	fontchrlink* Get(int i){return m_data[i];}
	int Height(){return m_height;}
	int Width() { return width; }
	int GetAbove() { return above; }
	int GetBelow() { return below; }

	string Patch(PenCallback* cb, fontchrlink* link, float orgX, float orgY, float scX, float scY);

protected:
	bool ReadUnifont(CFile& pFile,char* head);
	bool ReadShapes(CFile& pFile,char* head);
	bool ReadBigfont(CFile& pFile,char* head);
	bool ReadSHP(CFile& pFile,char* head,int &fileType);
protected:
	char* m_desc;

	unsigned char           above,  /* Or height for extended bigfont */
		below;
	char                    modes;
	unsigned char           width;  /* Extended bigfont */
	char                    encoding;
	char m_type;

	char                    nesc;
	char                   *esc; 

	xArray< fontchrlink * > m_data;//数据记录
	int m_height;//字体高度
};
