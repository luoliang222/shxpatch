#include "stdafx.h"
#include "fontchrlink.h"
#include ".\shapefont.h"
#include"math.h"
#include "Point2_T.h"
#include <iostream>

#define PI 3.1415926535898
#define strnsame(a,b,c) (!strncmp((a),(b),(c)))
#define strnisame(a,b,c) (!strnicmp((a),(b),(c)))
short getfontval(char **lpp) ;
#define FONTHEADER 40

short ic_bulge2arc(const Point2F& p0, const Point2F& p1, double bulge,
	Point2F& cc, double *rr, double *sa, double *ea);

int strnicmp(const char *s1, const char *s2, int len)  
{  
	unsigned char c1, c2;  
	if(!len)  
		return 0;  
	do{  
		c1 = *s1++;  
		c2 = *s2++;  
		if (!c1 || !c2)  
			break;  
		if (c1 == c2)  
			continue;  
		c1 = tolower(c1);  
		c2 = tolower(c2);  
		if (c1 != c2)  
			break;  
	}while(--len);  
	return (int)c1 - (int)c2;  
}  

ShapeFont::ShapeFont(void)
{
	m_height=1024;
	m_desc=NULL;
	esc=NULL;
}

ShapeFont::~ShapeFont(void)
{	
	for(int i=0;i< m_data.Count();i++)
	{
		fontchrlink* link=m_data[i];
		if(link!=NULL)delete link;
	}
	m_data.Clear();
	if(m_desc!=NULL)
		delete[] m_desc;
	if(esc!=NULL)delete[] esc;
	m_desc=NULL;
	esc=NULL;
}

bool ShapeFont::Load(const char* fileName)
{			
	CFile pFile;
	if(!pFile.Open(fileName,CFile::modeRead))
	{
		//pFile.Close();
		return false;
	}

	if(pFile.GetLength()<FONTHEADER)
	{
		pFile.Close();
		return false;
	}

	int fileType=0;
	char head[FONTHEADER];	
	pFile.Read(head,FONTHEADER);
	if (strnsame(head,"AutoCAD-86 shapes 1.",20))
	{//SHAPE 1.0/1.1
		fileType=2;
		ReadShapes(pFile,head);

		m_type =head[20]=='1'? SHAPE_1_1:SHAPE_1_0;
	}
	else if (strnsame(head,"AutoCAD-86 unifont 1.0\r\n\x1A",25))
	{//unifont
		fileType=3;		

		ReadUnifont(pFile,head);//���Ѿ�������ͷ��Ϣ����,�����ظ���ȡ
		m_type =UNIFONT;

	}
	else if (strnsame(head,"AutoCAD-86 bigfont 1.0\r\n\x1A"/*DNT*/,25))
	{//bigfont
		fileType=4;
		m_type =BIGFONT;
		ReadBigfont(pFile,head);
	}
	else 
	{//SHP file
		ReadSHP(pFile,head,fileType);


	}

	pFile.Close();
	fontchrlink* link= GetFromCode('A');
	if(link==NULL)return true;
	CharData* pData= new CharData();
	link->ShapeCreateVec(pData);
	m_height= pData->m_charInfo.gmCellIncY;
	link->m_dataPtr=pData;
	return true;
}

fontchrlink* ShapeFont::GetFromName(const char* symbolName)
{	
	CString key(symbolName);
	for(int i=0;i<m_data.Count();i++)
	{
		fontchrlink* link= m_data[i];
		if(link==NULL|| link->symbolName==NULL)continue;
		if(key.CompareNoCase(link->symbolName)==0)
			return link;
	}

	return NULL;
}

fontchrlink* ShapeFont::GetFromCode(unsigned short code)
{
	if (m_data.size() == 0)
		return NULL;

	//if(code=='?')code=0xB9F0;
	//if(code>=m_data.Count())
	//	return NULL;

	if(code>0&&code<256)
		return m_data[code];
	else
	{
		for(int i=256;i<m_data.Count();i++)
		{				
			if((m_data[i]->code)==(code))
				return m_data[i];
		}
	}

	return NULL;
}

int ShapeFont::FindCodeByName(const char* name)
{
	for(int i = 0; i <m_data.Count(); ++i)
	{
		fontchrlink* data=m_data[i];
		if (data)
		{
			if(data->symbolName && strnicmp(data->symbolName, name, 2049) == 0)
				return data->code;
		}

	}
	return 0;
}

bool ShapeFont::ReadUnifont(CFile& pFile,char* head)
{

	//1.�ļ�ͷ����
	long nchrsinfile=0;//���������
	short fi1;//����ƫ����
	memcpy(&nchrsinfile,&head[25],4);
	memcpy(&fi1,&head[25 + 4],sizeof(fi1));
	int def1seek=25+ 6 + fi1;//���ݿ�ʼλ�� 

	fi1-=6;//������Ϣ�ĳ���
	char* desc= new char[fi1];
	pFile.Seek(25+6,SEEK_SET);
	pFile.Read(desc,fi1);
	int fi2=0;while (fi2<fi1&&desc[fi2]) fi2++;
	m_desc =new char[fi2+1];
	memcpy(m_desc,desc,fi2);
	m_desc[fi2]='\0';//�����ļ�������Ϣ
	delete[] desc;

	/* ��ȡ�ļ�ͷ���� Get the header bytes: */
	char ln[6];
	pFile.Read(ln,6);	
	above   =ln[0];
	below   =ln[1];
	modes   =ln[2];
	encoding=ln[3];
	m_type    =ln[4];

	//////////////////////////////////////////////////////////////////////////
	//2.��ȡ������Ϣ����

	m_data.resize(256);
	for(int i=0;i<m_data.Count();i++)m_data[i]=NULL;	
	pFile.Seek(def1seek,SEEK_SET);
	long fileLenght= pFile.GetLength();
	char temp[2048];
	while (pFile.GetPosition()< (fileLenght-4))
	{
		int g=pFile.GetPosition();
		char ln[4];
		pFile.Read(ln,4);
		short code,descLen;//�����ţ�������Ϣ����
		memcpy(&code,ln,2);
		memcpy(&descLen,&ln[2],2);

		if(pFile.GetPosition()+ descLen>fileLenght) break;
		fontchrlink* link= new fontchrlink();
		link->code=code;		
		//char* temp= new char[descLen];
		pFile.Read(temp,descLen);

		//��ȡsymbolName��������ʽ�õ�
		int k=0; while(k<descLen&&temp[k]) k++;
		k++;
		if(k<descLen)
		{//
			if(k>1)
			{
				link->symbolName= new char[k];
				memcpy(link->symbolName,temp,k);	
			}
		}
		else
			k=0;

		//��ȡ����ʸ������
		link->defsz=descLen-k;
		link->def=new char[descLen-k];
		memcpy(link->def,&temp[k],descLen-k);	
		if(code>0&&code<256) 
		{
			if(m_data[code]!=NULL)delete m_data[code];
			m_data[code]=link;
		}
		else
			m_data.Add(link);
	}

	return true;
}

bool ShapeFont::ReadShapes(CFile& pFile,char* head)
{
	//1.�ļ�ͷ����
	long nchrsinfile=0;//���������
	short fi1;//����ƫ����
	memcpy(&fi1,&head[28],sizeof(fi1));
	nchrsinfile=fi1;
	int def1seek=30L+nchrsinfile*4;

	struct lendatalink { short code,nbytes; } *lendata;
	lendata= new struct lendatalink [nchrsinfile];
	memset(lendata,0,sizeof(struct lendatalink)*nchrsinfile);
	short shapedef=1;
	char ln[512];
	pFile.Seek(30,SEEK_SET);
	for (fi1=0; fi1<nchrsinfile; fi1++)
	{
		pFile.Read(ln,4);

		memcpy(&lendata[fi1].code  ,ln  ,sizeof(short));
		memcpy(&lendata[fi1].nbytes,ln+2,sizeof(short));
		if (!fi1 && !lendata[fi1].code) shapedef=0;  /* Font */

	}

	if (shapedef) {
		m_desc=new char[17];
		strcpy_s(m_desc,17,"Shape Definition");
		above=1;  
	}
	else 
	{
		pFile.Seek(def1seek,SEEK_SET);
		fi1=lendata[0].nbytes-4;  /* Len of desc (counting the 0). */
		pFile.Read(ln,fi1);

		fi1--;  /* Expected strlen of desc. */
		short fi2=0; while (fi2<fi1 && ln[fi2]) fi2++;
		/* fi2 is actual strlen of desc. */
		ln[fi2]=0;
		m_desc=new char[fi2+1];
		strcpy_s(m_desc,fi2+1,ln);

		/* Get the header bytes: */
		pFile.Read(ln,4);

		above=ln[0];
		below=ln[1];
		modes=ln[2];
	}

	//////////////////////////////////////////////////////////////////////////
	//2.��ȡ������Ϣ����

	pFile.Seek(def1seek,SEEK_SET);
	m_data.resize(256);
	for(int i=0;i <m_data.Count();i++)
		m_data[i]=NULL;
	for (fi1=0; fi1<nchrsinfile; fi1++) {
		/* ����ͷ���� */
		if (!lendata[fi1].code) 
		{
			int	newPosition = pFile.GetPosition() +lendata[fi1].nbytes;
			pFile.Seek(newPosition,SEEK_SET);
			continue;
		}

		char symbolName[255];//Ϊ�ϳ�������ʽ	
		int symbolNameLen = 0;

		short togo=lendata[fi1].nbytes;
		char fint1;
		while (togo>0) {
			pFile.Read(&fint1,sizeof(fint1));
			symbolName[symbolNameLen] = (char)fint1;		
			++symbolNameLen;
			togo--;
			if (fint1==0 || fint1==EOF) break;
		}

		symbolName[symbolNameLen] = 0;
		++symbolNameLen;

		if (togo>0) {

			fontchrlink *flink=new fontchrlink();

			flink->code=lendata[fi1].code;
			flink->defsz=togo;
			flink->symbolName = new char[symbolNameLen];
			strcpy_s(flink->symbolName,symbolNameLen, symbolName);

			flink->def = new char[flink->defsz];
			for (short didx=0; didx<flink->defsz &&
				pFile.Read(&fint1,sizeof(fint1))&&fint1!=EOF; didx++)
				flink->def[didx]=(char)fint1;

			if(flink->code>0&&flink->code<256) m_data[flink->code]=flink;
			else
				m_data.Add(flink);

		}
	}

	if (lendata!=NULL)
		delete []lendata;
	return true;
}

bool ShapeFont::ReadBigfont(CFile& pFile,char* head)
{	
	//1.�ļ�ͷ����
	long nchrsinfile=0;//���������
	short fi1;//����ƫ����
	memcpy(&fi1,&head[27],sizeof(fi1));
	nchrsinfile=fi1;
	memcpy(&fi1,&head[27+sizeof(short)],sizeof(fi1));
	nesc = (char)fi1;

	fi1=2*nesc;
	esc = new char[fi1];
	fi1*=2;
	int def1seek = 27+sizeof(short)+sizeof(short);
	char ln[512];
	pFile.Seek(def1seek,SEEK_SET);
	pFile.Read(ln,fi1);

	def1seek = def1seek+fi1;//���ݿ�ʼλ��

	short fi2,fi3,fi4;
	for (fi1=fi2=fi3=0; fi1<nesc; fi1++,fi2+=2,fi3+=4) {
		memcpy(&fi4,&ln[fi3],sizeof(fi4));
		esc[fi2]=(char)fi4;
		memcpy(&fi4,&ln[fi3+2],sizeof(fi4));
		esc[fi2+1]=(char)fi4;
	}

	fi1=8;
	pFile.Read(ln,fi1);
	memcpy(&fi1,&ln[sizeof(short)],sizeof(fi1));
	long thisseek;
	memcpy(&thisseek,&ln[4],sizeof(thisseek));
	pFile.Seek(thisseek,SEEK_SET);
	pFile.Read(ln,fi1);

	fi2=0; while (fi2<fi1 && ln[fi2]) fi2++;
	fi2++; 
	fi3=fi1-fi2;  
	m_desc=new char[fi2];
	strcpy_s(m_desc,fi2, ln);

	/* ��ȡͷ�ļ����� Get the header bytes: */
	above   =ln[fi2];
	below   =ln[fi2+1];
	modes   =ln[fi2+2];
	if (fi3>4) width=ln[fi2+3];

	//////////////////////////////////////////////////////////////////////////
	//2.��ȡ������Ϣ����

	m_data.resize(256);
	for(int i=0;i<m_data.Count();i++)m_data[i]=NULL;
	pFile.Seek(def1seek,SEEK_SET);
	long fileLenght= pFile.GetLength();
	char temp[1024];
	for (long locidx=0; locidx<nchrsinfile; locidx++)
	{
		unsigned char ch[8];
		pFile.Read(ch,8);
		unsigned short code,descLen;//�����ţ�������Ϣ����
		code= ((unsigned short)ch[0]<<8)|ch[1];
		//memcpy(&code,ch,2);
		/*unsigned short s=0;
		memcpy(&s,ch,sizeof(short));*/

		memcpy(&descLen,&ch[2],2);
		memcpy(&thisseek,&ch[4],sizeof(thisseek));

		if (code ==0)
		{				

			continue;
		}

		if(pFile.GetPosition()+ descLen>fileLenght) break;

		int g = pFile.GetPosition();
		pFile.Seek(thisseek,SEEK_SET);
		fontchrlink* link= new fontchrlink();
		link->code=code;		
		char* temp= new char[descLen];
		pFile.Read(temp,descLen);
		int k=0; while(k<descLen&&!temp[k]) k++;
		link->defsz=descLen-k;
		link->def=new char[descLen-k];
		memcpy(link->def,&temp[k],descLen-k);		
		if(code>0&&code<256) m_data[code]=link;
		else
		{			
			CString mx;mx.Format("%X",code);
			//AfxMessageBox(mx);
			m_data.Add(link);
		}
		pFile.Seek(g,SEEK_SET);
		delete []temp;
	}



	return true;
}

bool ShapeFont::ReadSHP(CFile& pFile,char* head,int &fileType)
{
	//1.�ļ�ͷ����
	pFile.Seek(0,SEEK_SET);
	long def1seek;
	while(!fileType)
	{
		/* ��ȡ��һ�� */
		short lidx=0;
		char fint1;
		char ln[512];
		while (lidx<511 && pFile.Read(&fint1,sizeof(fint1)) &&
			fint1 && fint1!='\n')
			if (fint1!='\r') ln[lidx++]=(char)fint1;
		ln[lidx]=0;

		/*ȥ��˵�� */
		char* fcp1=NULL;
		fcp1=ln; while (*fcp1 && *fcp1!=';') fcp1++;
		*fcp1=0;

		//��ȡ����������
		short fi1,fi2;
		for (fi1=fi2=0; ln[fi2]; fi2++) {
			if (ln[fi2]=='(' || ln[fi2]==')') continue;
			if (fi1!=fi2) ln[fi1]=ln[fi2];
			fi1++;
		}
		ln[fi1]=0;

		/* Start lp at 1st non-space char: */
		char* lp=NULL;			;
		lp=ln; while (*lp && isspace((unsigned char) *lp)) lp++;
		if (!*lp) continue;  

		if (m_desc==NULL) {  /* ��ȡ���� */
			while (*lp && *lp!=',') lp++;
			if (*lp) {
				lp++;
				while (*lp && *lp!=',') lp++;
				if (*lp) {
					lp++;
					m_desc=new char[strlen(lp)+1];
					strcpy_s(m_desc,strlen(lp)+1, lp);
				}
			}
		} else {  /* ��ȡ�ļ�ͷ����. */

			for (fcp1=lp,fi1=0; *fcp1 && fi1<2; fcp1++)
				if (*fcp1==','/*DNT*/) fi1++;

			for (fi1=0; *lp && fi1<5; fi1++) {
				char fc1=(char)((unsigned char)getfontval(&lp));
				switch (fi1) {
				case 0: above   =fc1; break;
				case 1: below   =fc1; break;
				case 2: modes   =fc1; break;
				case 3: encoding=fc1; break;
				case 4: m_type    =fc1; break;
				}
			}

			/* Seek pos is ok for reading defs: */
			def1seek=pFile.GetPosition();

			fileType=1;
		}
	}
	//////////////////////////////////////////////////////////////////////////
	//2.��ȡ������Ϣ����
	pFile.Seek(def1seek,SEEK_SET);

	short thiscode=0;  /* Current chr code being processed. */
	short nditems=0;   /* # chr of definition items. */
	short cidx=0;      /* flink->chr[] idx. */
	short didx=0;      /* flink->chr[cidx].def[] idx */
	for (;;) {  /* Read lines. */


		/* Read 1 text line: */
		short lidx=0;
		char fint1;
		char ln[512];
		while (lidx<511 && pFile.Read(&fint1,sizeof(fint1)) &&
			fint1 && fint1!='\n'/*DNT*/)
			if (fint1!='\r'/*DNT*/) ln[lidx++]=(char)fint1;
		ln[lidx]=0;
		if (fint1==EOF) break;

		/* Chop comment: */
		char* fcp1;
		fcp1=ln; while (*fcp1 && *fcp1!=';'/*DNT*/) fcp1++;
		*fcp1=0;

		/* Compress out all parentheses: */
		short fi1,fi2;
		for (fi1=fi2=0; ln[fi2]; fi2++) {
			if (ln[fi2]=='('/*DNT*/ || ln[fi2]==')'/*DNT*/) continue;
			if (fi1!=fi2) ln[fi1]=ln[fi2];
			fi1++;
		}
		ln[fi1]=0;

		/* Start lp at 1st non-space char: */
		char *lp;
		lp=ln; while (*lp && isspace((unsigned char) *lp)) lp++;
		if (!*lp) continue;  /* Blank line. */

		if (*lp=='*'/*DNT*/) {  /* Start a new chr. */

			lp++; while (*lp && isspace((unsigned char) *lp)) lp++;
			if (*lp>='0'/*DNT*/ && *lp<='9'/*DNT*/) {  /* Numeric */
				thiscode=getfontval(&lp);
			} else {  /* Non-numeric */
				/* Try for one of the non-UNICODE */
				/* symbolic identifiers: */
				thiscode=0;  /* Ignore this chr if not found. */
				if      (strnisame(lp,"DEGREE_SIGN"/*DNT*/,11))
					thiscode=0x00B0;
				else if (strnisame(lp,"PLUS_OR_MINUS_SIGN"/*DNT*/,18))
					thiscode=0x00B1;
				else if (strnisame(lp,"DIAMETER_SYMBOL"/*DNT*/,15))
					thiscode=0x2205;
				/* Step past next comma: */
				while (*lp && *lp!=',') lp++;
				if (*lp) lp++;
			}
			if (!thiscode) continue;  /* 0 reserved for header line */
			if (!*lp) { fileType=0; return false; }
			nditems=getfontval(&lp);  /* # of data items. */
			if (nditems<1) { fileType=0; return false; }
			didx=0;  /* Init data item idx */

			/* Set the code and alloc the def array: */
			fontchrlink *flink=new fontchrlink();
			flink->code=thiscode;
			flink->defsz=nditems;
			flink->def=
				new char[flink->defsz];
			memset(flink->def,0,
				flink->defsz);
			if(flink->code>0&&flink->code<256)
			{
				m_data[flink->code]=flink;
				cidx = flink->code;
			}
			else
			{
				m_data.Add(flink);
				cidx=m_data.Count()-1;
			}

		} else {  /* Gather data for thiscode. */

			if (!thiscode || cidx<1 )
			{ thiscode=0; continue; }
			while (*lp && didx<m_data[cidx]->defsz)
				m_data[cidx]->def[didx++]=
				(char)((unsigned char)getfontval(&lp));
			if (didx>=m_data[cidx]->defsz) thiscode=0;

		}

	}
	return true;

}

string ShapeFont::Patch(PenCallback* cb, fontchrlink* link, float orgX,float orgY,float scX,float scY)
{
	string res = "";
	string cmd;
	if(link==NULL)
		return "";//�Ҳ�������
	bool penDown=false;
	bool vertonly=false;//��ֱ����
	CPoint curPt(0,0);//��ǰ�����
	CPoint stack[102];//��ջ
	int sIndex=-1;	
	float dx=0;
	float dy=0;
	int i;
	for ( i=0; i<link->defsz;i++)
		if(link->def[i]==1||link->def[i]==2)
			break;

	for (; i<link->defsz;i++)
	{
		unsigned char opt= (unsigned char)link->def[i];
		dx=0;dy=0;
		if(opt==0)
		{//��������			
			continue;
		}
		switch(opt)
		{
		case 1:
		{
			float _x = orgX + curPt.x*scX;
			float _y = orgY + curPt.y*scY;
			if (cb)
				cb->setCurrent(_x, _y);

			penDown = true;
			dx = 0;dy = 0;
			res.append("pendown|");
		}
			break;
		case 2:
		{
			float _x = orgX + curPt.x*scX;
			float _y = orgY + curPt.y*scY;
			if (cb)
				cb->setCurrent(_x, _y);

			penDown = false;
			dx = 0;dy = 0;
			res.append("penup|");
		}
			break;
		case 3:
			{
				unsigned char sc= (unsigned char)link->def[++i];
				scX/=sc;scY/=sc;				
			}
			break;
		case 4:
			{
				unsigned char sc= (unsigned char)link->def[++i];
				scX*=sc;scY*=sc;				
			}
			break;
		case 5:
			stack[++sIndex]= curPt;
			break;
		case 6:
			curPt=stack[sIndex--];
			break;
		case 7:
			{
				char basex=0;
				char basey=0;
				char wd=0;
				char ht=0;
				unsigned short subCode=0;
				if(this->m_type==UNIFONT)
				{
					i+=2;
					subCode=(unsigned char)link->def[i-1]<<8
						|(unsigned char)link->def[i];
				}
				else
				{// SHAPE or BIGFONT 
					subCode=(unsigned char)link->def[++i];                                  
					if (!subCode) {
						i+=6;
						subCode=(unsigned char)link->def[i-5]<<8
							|(unsigned char)link->def[i-4];	
						basex= link->def[i-3];	
						basey= link->def[i-2];	
						wd= link->def[i-1];	
						ht= link->def[i];	
					}						
				}

				fontchrlink* sublink = GetFromCode(subCode);
				string _res = Patch(cb, sublink, orgX+ basex,orgY+basey,scX,scY);
				res.append(_res);
			}
			break;
		case 8:
			{
				dx= link->def[++i]; 
				dy= link->def[++i]; 
			}
			break;
		case 9:
			{
				dx= link->def[++i]; 
				dy= link->def[++i]; 
				while(link->def[i-1]||link->def[i])
				{
					curPt.x+=dx;curPt.y-=dy;
					if(penDown){
						float _x = orgX+curPt.x*scX;
						float _y = orgY+curPt.y*scY;
						if (cb)
							cb->LineTo(_x, _y);

						CString sx;
						sx.Format("%f",_x);
						CString sy;
						sy.Format("%f",_y);

						res.append("lineto:");
						res.append(sx + ",");
						res.append(sy + "|");
						////pDC->LineTo(orgX+curPt.x*scX,orgY+curPt.y*scY);
					}
					dx= link->def[++i]; 
					dy= link->def[++i]; 
				}
			}
			break;
		case 10:
			{//�˷�Բ��
				i+=2;
				bool cw= (link->def[i]&0x80)!=0;//true-��ʱ�룬false-˳ʱ��
				unsigned char r= link->def[i-1];
				if(r==0)r=1;
				//��ʼ��λ��(�˷�λ��)
				unsigned char sa=(((unsigned char)(link->def[i]&'\x70'))>>4);

				//�����ĽǶ�(8:Բ)
				unsigned char deta=(unsigned char)(link->def[i]&'\x07');
				if(cw)
				{
					sa-=deta;
					if(sa<0)sa+=8;
				}
				if(!deta)deta=8;

				unsigned char ea= sa+deta;
				/* dx and dy to endpt: */
				dx=r*(cos(ea*PI/4)-cos(sa*PI/4));
				dy=r*(sin(ea*PI/4)-sin(sa*PI/4));
				if(cw)
				{
					dx=-dx;dy=-dy;
				}
				//����Բ(��)
				////pDC->AngleArc(orgX+curPt.x - r * cos(sa*PI/4)*scX,orgY+ curPt.y+ r * sin(sa*PI/4)*scX,r*scX,sa*45,deta*45);
				res.append("anglearc|");
				curPt.x+=dx;curPt.y-=dy;
				dx=0;dy=0;
			}
			break;
		case 11:
			{
				i+=5;
				bool cw= (link->def[i]&0x80)!=0;//true-��ʱ�룬false-˳ʱ��
				float r=256.0*((float)((unsigned char)link->def[i-2]))+
					(float)((unsigned char)link->def[i-1]);
				if (r<1.0) r=1.0;
				//��ʼ��λ��(�˷�λ��)
				float sa=(((unsigned char)(link->def[i]&'\x70'))>>4);

				//�����ĽǶ�(8:Բ)
				float deta=(unsigned char)(link->def[i]&'\x07');

				//��ʼƫ�ƣ���45��ǵȷ�256��
				sa+=(unsigned char)link->def[i-4]/256.0f;
				deta += (unsigned char)link->def[i-3]/256.0f;//��ֹ��ƫ��
				if(cw)
				{
					sa-=deta;
					if(sa<0)sa+=8;
				}
				if(!deta)deta=8;

				unsigned char ea= sa+deta;
				/* dx and dy to endpt: */
				dx=r*(cos(ea*PI/4)-cos(sa*PI/4));
				dy=r*(sin(ea*PI/4)-sin(sa*PI/4));
				if(cw)
				{
					dx=-dx;dy=-dy;
				}
				//����Բ(��)
				////pDC->AngleArc(orgX+curPt.x*scX - r * cos(sa*PI/4),orgY+ curPt.y*scY+ r * sin(sa*PI/4),r*scX,sa*45,deta*45);
				res.append("anglearc|");
				curPt.x+=dx;curPt.y-=dy;
				dx=0;dy=0;
				//����Բ��
			}
			break;
		case 12:
			{//��
				dx= link->def[++i]; 
				dy= link->def[++i]; 
				float bug= link->def[++i]; //͹��
				if(bug>127)bug=127;if(bug<-127)bug=-127;
				bug/=127.0f;

				CString stm;
				stm.Format("Բ��: dx=%f, dy=%f, bug=%f\n", dx, dy, bug);
				cmd += stm;

				// i+=100;
				// pDC->LineTo(orgX+curPt.x*scX-100,orgY+curPt.y*scY);
				// pDC->LineTo(orgX+curPt.x*scX,orgY+curPt.y*scY);
				//����Բ��
				if(penDown)
				{
					// double p1[2],p2[2],cc[2];
					Point2F p1, p2, cc;
					double r,sa,ea;
					p1[0]=curPt.x;
					p1[1]=curPt.y;
					p2[0]=p1[0]+dx;
					p2[1]=p1[1]+dy;

					// short ic_bulge2arc(const Point2F& p0, const Point2F& p1, double bulge,
					//		Point2F& cc, double *rr, double *sa, double *ea)
					ic_bulge2arc(p1,p2, bug, cc, &r, &sa, &ea);
					float step=(ea-sa);						
					if(bug<0)
					{
						step=-step;
						sa=ea;
					}

					if (step < 0)
						step = IC_TWOPI + step;

/*					if(step>3.1415) 										
						step= IC_TWOPI-step;

					if(step<-3.1415)					
						step=-IC_TWOPI-step;
*/
					step/=8;	
					for(int i=1;i<8;i++)
					{
						double x= cc[0] + r* cos(sa+step*i);						
						double y= cc[1] + r* sin(sa+step*i);
						x = orgX + x*scX;
						y = orgY + y*scY;
						if (cb)
							cb->LineTo(x, y);

						CString sx;
						sx.Format("%f",x);
						CString sy;
						sy.Format("%f",y);

						res.append("lineto:");
						res.append(sx + ",");
						res.append(sy + "|");

						////pDC->LineTo(x,y);
					}
				}

			}break;
		case 13:
			{//���Բ��
				unsigned char code1= dx= (unsigned char)link->def[++i]; 
				unsigned char code2= (unsigned char)link->def[++i]; 
				while(code1| code2)
				{//TODO:�˷�Բ�� case 10
					code1= dx= (unsigned char)link->def[++i]; 
					code2= (unsigned char)link->def[++i]; 
				}
			}
			break;
		case 14: {
			int next = link->def[i + 1];
			vertonly=true;
			// ����˫�����壬14����������ڴ�ֱ���������
			// �˴����Դ�ֱ���ֵ�����
			if (next == 8) {	// ��λ��ƫ�ƴ��룬����
				i++;
				i++;
				i++;
			}
			else if(next > 14){
				i++;	// ��ʸ������ͳ��ȴ��룬����
			}
		}
				 break;
		default:
			{// ʸ������ͳ���
				unsigned char vlen=(unsigned char)link->def[i]; ;
				unsigned char vdir=vlen&'\x0F';
				if (!(vlen>>=4)) break;
				switch (vdir) {
				case '\x00': dx= 1.0; dy= 0.0; break;
				case '\x01': dx= 1.0; dy= 0.5; break;
				case '\x02': dx= 1.0; dy= 1.0; break;
				case '\x03': dx= 0.5; dy= 1.0; break;
				case '\x04': dx= 0.0; dy= 1.0; break;
				case '\x05': dx=-0.5; dy= 1.0; break;
				case '\x06': dx=-1.0; dy= 1.0; break;
				case '\x07': dx=-1.0; dy= 0.5; break;
				case '\x08': dx=-1.0; dy= 0.0; break;
				case '\x09': dx=-1.0; dy=-0.5; break;
				case '\x0A': dx=-1.0; dy=-1.0; break;
				case '\x0B': dx=-0.5; dy=-1.0; break;
				case '\x0C': dx= 0.0; dy=-1.0; break;
				case '\x0D': dx= 0.5; dy=-1.0; break;
				case '\x0E': dx= 1.0; dy=-1.0; break;
				case '\x0F': dx= 1.0; dy=-0.5; break;
				}
				dx*=vlen; dy*=vlen; 
			}
			break;
		}
		if(opt==1){
			float _x = orgX+curPt.x*scX;
			float _y = orgY+curPt.y*scY;
			if (cb)
				cb->MoveTo(_x, _y);

			CString sx;
			sx.Format("%f",_x);
			CString sy;
			sy.Format("%f",_y);

			res.append("moveto:");
			res.append(sx + ",");
			res.append(sy + "|");
		}
		else
		{
			curPt.x+=dx; curPt.y+=dy;
			if(penDown){
				////pDC->LineTo(orgX+curPt.x,orgY+curPt.y);

				float _x = orgX+curPt.x*scX;
				float _y = orgY+curPt.y*scY;
				if (cb)
					cb->LineTo(_x, _y);

				CString sx;
				sx.Format("%f",_x);
				CString sy;
				sy.Format("%f",_y);

				res.append("lineto:");
				res.append(sx + ",");
				res.append(sy + "|");
			}
		}

	}
	return res;
}

short getfontval(char **lpp) {

	char *lp = NULL;
	short rc,neg,fi1,fi2;


	rc=neg=0;


	if (lpp==NULL) goto out;

	lp=*lpp;  /* Convenience */

	while (*lp && isspace((unsigned char) *lp)) lp++;
	if (*lp=='+') lp++; else if (*lp=='-') { neg=1; lp++; }
	if (!*lp) goto out;
	if (*lp=='0'/*DNT*/) {  /* Hex */
		lp++;
		for (fi1=0; fi1<4; fi1++,lp++) {
			*lp=toupper(*lp);
			if (*lp>='A'/*DNT*/ && *lp<='F'/*DNT*/) {
				fi2=*lp-'A'/*DNT*/+10;
			} else if (*lp>='0'/*DNT*/ && *lp<='9'/*DNT*/) {
				fi2=*lp-'0'/*DNT*/;
			} else break;  /* Not a hex digit */
			rc<<=4; rc|=fi2;
		}
		if (neg) rc|=((fi1<3) ? 0x0080 : 0x8000);
	} else {  /* Decimal */
		rc=atoi(lp); if (neg) rc=-rc;
	}

	while (*lp && *lp!=',') lp++;
	if (*lp) { lp++; while (*lp && isspace((unsigned char) *lp)) lp++; }


out:
	if (lpp!=NULL) *lpp=lp;
	return rc;
}
