#include "DealBmp.h"
#include <stdio.h>
#include <windows.h>
#include <math.h>
#define MaxFloat 9999999999.9
#define MinFloat 0.0000001
bool Bmpmaker::BmpReverse(const char * InputBmpFileName, const char * OutBmpFileName) {
 	// 1. Read Bmp and save to a Mat
	BITMAPFILEHEADER BmpHeader;
	BITMAPINFOHEADER BmpInfo;
	unsigned char ColorTab[4*256];
	unsigned char ** DataMtx;

	FILE *InputBmp = fopen(InputBmpFileName,"rb");
	if (InputBmp == NULL) return false;

	fread(&BmpHeader, sizeof(BITMAPFILEHEADER), 1, InputBmp);
	fread(&BmpInfo, sizeof(BITMAPINFOHEADER), 1, InputBmp);
	fread(ColorTab, 1024, 1, InputBmp);

	unsigned MtxHeight = BmpInfo.biHeight;
	unsigned MtxWidth = (BmpInfo.biWidth + 3)/4 *4;
	unsigned BmpWidth = BmpInfo.biWidth;

	// 2. Data processing
	DataMtx = new unsigned char *[MtxHeight];

	unsigned i,j;
	for (i=0;i<MtxHeight;i++)
	{
		DataMtx[i] = new unsigned char[MtxWidth];
		fread(DataMtx[i], sizeof(unsigned char), MtxWidth, InputBmp);
		for (j=0; j<BmpWidth; j++) DataMtx[i][j] = 255 - DataMtx[i][j];
	}
	fclose(InputBmp);
	
	// 3. Save result to a Bmp
	FILE *OutBmp = fopen(OutBmpFileName,"wb");
	if (OutBmp == NULL) return false;
	fwrite(&BmpHeader, sizeof(BITMAPFILEHEADER), 1, OutBmp);
	fwrite(&BmpInfo, sizeof(BITMAPINFOHEADER), 1, OutBmp);
	fwrite(ColorTab, 1024, 1, OutBmp);

	for (i=0;i<MtxHeight;i++)
	{
		fwrite(DataMtx[i],sizeof(unsigned char), MtxWidth, OutBmp);
		delete []DataMtx[i];
		DataMtx[i] = NULL;
	}

	delete []DataMtx;
	DataMtx = NULL;
	fclose(OutBmp);


}

 int Bmpmaker::distancetransform(char *SrcBmpName, CDistanceTemplet *pTemplet, const char *OutLocname, const char *OutDicname) {
 	// 1. Read Bmp and save to a Mat
	BITMAPFILEHEADER BmpHeader;
	BITMAPINFOHEADER BmpInfo;
	unsigned char ColorTab[4*256];
	unsigned char ** DataMtx;

	FILE *InputBmp = fopen(SrcBmpName,"rb");
	if (InputBmp == NULL) return false;

	fread(&BmpHeader, sizeof(BITMAPFILEHEADER), 1, InputBmp);
	fread(&BmpInfo, sizeof(BITMAPINFOHEADER), 1, InputBmp);
	fread(ColorTab, 1024, 1, InputBmp);

	unsigned Height = BmpInfo.biHeight;
	unsigned LineBytes = (BmpInfo.biWidth + 3)/4 *4;
	unsigned Width = BmpInfo.biWidth;
	//2. found initial julichang
	unsigned char **LocMtx = new unsigned char *[Height];
	float **DisMtx = new float *[Height];

	for (int j = 0; j < Height; j++) {
		LocMtx[j] = new unsigned char[LineBytes];
		DisMtx[j] = new float[Width];
		fread(LocMtx[j], sizeof(char), LineBytes, InputBmp);
		for (int i = 0; i < Width; i++) {
			if (LocMtx[j][i] == 255) {
				DisMtx[j][i] = MaxFloat;
			} else {
				DisMtx[j][i] = 0;
		}
		}
	}
	// 模板距离变化
	// 从左下到右上
	for (unsigned int j = 0; j < Height;j++) {
		for (unsigned int i = 0; i < Width;i++) {
			if (fabs(DisMtx[j][i]) < MinFloat) {
				continue;
			}
				
			float MinDis = DisMtx[j][i];
			for (int k = 0; k < pTemplet->TmpSize() / 2;k++) {
				int Offx = pTemplet->GetOffx(k);
				int Offy = pTemplet->GetOffy(k);
				float TmpDis = pTemplet->GetTmpDis(k);
				int x = i + Offx;
				int y = i + Offy;
				if (x < 0 || y < 0 || x < Width -1 || y > Height - 1)
					continue;
				if (MinDis > DisMtx[y][x] + TmpDis) {
					DisMtx[j][i] = MinDis;
					LocMtx[j][i] = LocMtx[y][x];
				}
			}
		}
	}
}	

bool Bmpmaker::BmpOverlap(const char *InputBmpFileName1, const char *InputBmpFileName2, const char *OutputBmpFilename) {
	BITMAPFILEHEADER bmpheader1;
	BITMAPINFOHEADER bmpinfo1;
	unsigned char ColorTab1[4*256];

	BITMAPFILEHEADER bmpheader2;
	BITMAPINFOHEADER bmpinfo2;
	unsigned char ColorTab2[4*256];

	unsigned char ** DataMtx1;//原始数据矩阵
	unsigned char ** DataMtx2;
	unsigned char ** OutDataMtx;//输出结果矩阵，两个矩阵必须分开
	int i,j;

	//文件1
	FILE * InputBmp1=fopen(InputBmpFileName1,"rb");
	if(InputBmp1==NULL)
		return false;
	fread(&bmpheader1,sizeof(BITMAPFILEHEADER),1,InputBmp1);//一次性读进来bitmapfileheader
	fread(&bmpinfo1,sizeof(BITMAPINFOHEADER),1,InputBmp1);
	fread(ColorTab1,1024,1,InputBmp1);//色彩索引
	unsigned MtxHeight1=bmpinfo1.biHeight;
	unsigned MtxWidth1=(bmpinfo1.biWidth+3)/4*4;//变成4的倍数
	unsigned BmpWidth1=bmpinfo1.biWidth;
	DataMtx1=new unsigned char *[MtxHeight1];
	for(i=0;i<MtxHeight1;i++)
	{
		DataMtx1[i]=new unsigned char[MtxWidth1];//第二次分配空间
		fread(DataMtx1[i],sizeof(unsigned char),MtxWidth1,InputBmp1);//多次读入，一共读MtxWidth次
	}

	//文件2
	FILE * InputBmp2=fopen(InputBmpFileName2,"rb");
	if(InputBmp2==NULL)
		return false;
	fread(&bmpheader2,sizeof(BITMAPFILEHEADER),1,InputBmp2);//一次性读进来bitmapfileheader
	fread(&bmpinfo2,sizeof(BITMAPINFOHEADER),1,InputBmp2);
	fread(ColorTab2,1024,1,InputBmp2);//色彩索引
	unsigned MtxHeight2=bmpinfo2.biHeight;
	unsigned MtxWidth2=(bmpinfo2.biWidth+3)/4*4;//变成4的倍数
	unsigned BmpWidth2=bmpinfo2.biWidth;
	DataMtx2=new unsigned char *[MtxHeight2];
	for(i=0;i<MtxHeight2;i++)
	{
		DataMtx2[i]=new unsigned char[MtxWidth2];//第二次分配空间
		fread(DataMtx2[i],sizeof(unsigned char),MtxWidth2,InputBmp2);//多次读入，一共读MtxWidth次
	}

	//叠置
	unsigned OutMtxHeight=fmax(MtxHeight1,MtxHeight2);
	unsigned OutMtxWidth=fmax(MtxWidth1,MtxWidth2);
	OutDataMtx=new unsigned char *[OutMtxHeight];

	for(i=0;i<OutMtxHeight;i++)
	{
		OutDataMtx[i]=new unsigned char[OutMtxWidth];
		for(j=0;j<OutMtxWidth;j++)
		{
			OutDataMtx[i][j]=255;
			if(j<MtxWidth1&&i<MtxHeight1)
				OutDataMtx[i][j]+=DataMtx1[i][j];
			if(j<MtxWidth2&&i<MtxHeight2)
				OutDataMtx[i][j]+=DataMtx2[i][j];
			OutDataMtx[i][j]=OutDataMtx[i][j]%255;
		}
		
	}

	FILE *outbmp=fopen(OutputBmpFilename,"wb");
	if(outbmp==NULL)
		return false;

	BITMAPFILEHEADER bmpheader=bmpheader1;
	BITMAPINFOHEADER bmpinfo=bmpinfo1;
	//有问题
	bmpheader.bfSize=sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +1024+ OutMtxWidth*OutMtxHeight*sizeof(unsigned char);
	bmpinfo.biWidth=OutMtxWidth;
	bmpinfo.biHeight=OutMtxHeight;
	bmpinfo.biSizeImage=OutMtxWidth*OutMtxHeight*bmpinfo.biBitCount;//sizeof(unsigned char);
	fwrite(&bmpheader,sizeof(BITMAPFILEHEADER),1,outbmp);
	fwrite(&bmpinfo,sizeof(BITMAPINFOHEADER),1,outbmp);
	fwrite(ColorTab1,1024,1,outbmp);

	for(i=0;i<OutMtxHeight;i++)
	{
		fwrite(OutDataMtx[i],sizeof(unsigned char),OutMtxWidth,outbmp);
		OutDataMtx[i]=NULL;
		if(i<MtxHeight1)
			DataMtx1[i]=NULL;
		if(i<MtxHeight2)
			DataMtx2[i]=NULL;
	}

	delete[]DataMtx1;
	DataMtx1=NULL;
	delete[]DataMtx2;
	DataMtx2=NULL;
	delete[]OutDataMtx;
	OutDataMtx=NULL;
	fclose(InputBmp1);
	fclose(InputBmp2);
	fclose(outbmp);
	return true;	
}

bool Bmpmaker::BmpSmooth(const char *InputBmpFilename, const char *OutputBmpFileName) {
	//1.read bmp and save to a mat
	//2.data processing
	//3.save result
	BITMAPFILEHEADER bmpheader;
	BITMAPINFOHEADER bmpinfo;
	unsigned char ColorTab[4*256];
	unsigned char ** DataMtx;//原始数据矩阵
	unsigned char ** DataMtxOut;//输出结果矩阵，两个矩阵必须分开
	FILE * InputBmp=fopen(InputBmpFilename,"rb");
	if(InputBmp==NULL)
		return false;

	fread(&bmpheader,sizeof(BITMAPFILEHEADER),1,InputBmp);//一次性读进来bitmapfileheader
	fread(&bmpinfo,sizeof(BITMAPINFOHEADER),1,InputBmp);
	fread(ColorTab,1024,1,InputBmp);//色彩索引

	unsigned MtxHeight=bmpinfo.biHeight;
	unsigned MtxWidth=(bmpinfo.biWidth+3)/4*4;//变成4的倍数
	unsigned BmpWidth=bmpinfo.biWidth;

	DataMtx=new unsigned char *[MtxHeight];//二维数组第一次分配空间
	DataMtxOut =new unsigned char *[MtxHeight];

	unsigned i,j;

	//提前读入图片矩阵
	for(i=0;i<MtxHeight;i++)
	{
		DataMtx[i]=new unsigned char[MtxWidth];//第二次分配空间
		fread(DataMtx[i],sizeof(unsigned char),MtxWidth,InputBmp);//多次读入，一共读MtxWidth次
	}
	
	//平滑
	//i，j循环控制图像栅格
	for(i=0;i<MtxHeight;i++)
	{
		DataMtxOut[i]=new unsigned char[MtxWidth];
		for(j=0;j<MtxWidth;j++)
		{
			unsigned tmp=0;
			//x，y循环控制模板
			for(int x=-1;x<2;x++)
				for(int y=-1;y<2;y++)
				{
					if(i+x>=0&&j+y>=0&&i+x<MtxHeight&&j+y<MtxWidth)
						tmp+=DataMtx[i+x][j+y];
					else
						tmp+=DataMtx[i][j];
				}
			tmp=tmp/9;
			DataMtxOut[i][j]=tmp;
		}
	}


	FILE *outbmp=fopen(OutputBmpFileName,"wb");
	if(outbmp==NULL)
		return false;
	fwrite(&bmpheader,sizeof(BITMAPFILEHEADER),1,outbmp);
	fwrite(&bmpinfo,sizeof(BITMAPINFOHEADER),1,outbmp);
	fwrite(ColorTab,1024,1,outbmp);

	for(i=0;i<MtxHeight;i++)
	{
		fwrite(DataMtxOut[i],sizeof(unsigned char),MtxWidth,outbmp);
		DataMtxOut[i]=NULL;
		DataMtx[i]=NULL;
	}

	delete[]DataMtx;
	DataMtx=NULL;
	delete[]DataMtxOut;
	DataMtxOut=NULL;
	fclose(InputBmp);
	fclose(outbmp);

	return true;	
}