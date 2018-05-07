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
	return true;
}

int Bmpmaker::distancetransform(char *SrcBmpName, CDistanceTemplet *pTemplet, const char *OutLocname, const char *OutDisname) {
	BITMAPFILEHEADER srcBmpHead;
	BITMAPINFOHEADER srcBmpInfo;
	unsigned char ColorTab[1024];

	FILE *srcBmp = fopen(SrcBmpName, "rb");
	if (srcBmp == NULL) return false;

	fread(&srcBmpHead, sizeof(BITMAPFILEHEADER), 1, srcBmp);
	fread(&srcBmpInfo, sizeof(BITMAPINFOHEADER), 1, srcBmp);
	fread(ColorTab, 1024, 1, srcBmp);
	if (srcBmpInfo.biBitCount != 8)//256色位图处理
	{
		fclose(srcBmp);
		return false;
	}

	unsigned MtxHeight = srcBmpInfo.biHeight;
	unsigned MtxWidth = srcBmpInfo.biWidth;
	unsigned BufWidth = (MtxWidth + 3) / 4 * 4;
	//建立初始距离场、分配场
	unsigned char **LocMtx = new unsigned char*[MtxHeight];
	float **DisMtx = new float*[MtxHeight];
	//初始化二维数组
	for (int i = 0; i < MtxHeight; i++)
	{
		LocMtx[i] = new unsigned char[BufWidth];
		DisMtx[i] = new float[MtxWidth];
		//读取像素矩阵
		fread(LocMtx[i], BufWidth, 1, srcBmp);
	}
	//遍历像素矩阵，若值为255，则对应距离矩阵值为无穷大，否则为0
	for(int i=0;i<MtxHeight;i++)
		for (int j = 0; j < MtxWidth; j++)
		{
			if (LocMtx[i][j] == 255)
				DisMtx[i][j] = FLT_MAX;
			else
				DisMtx[i][j] = 0.0;
		}
	//从左下到右上开始，八边形模板溜一遍
	for(int i=0;i<MtxHeight;i++)
		for (int j = 0; j < MtxWidth; j++)
		{
			if (fabs(DisMtx[i][j] - 0) < FLT_EPSILON) continue;
			float MinDis = DisMtx[i][j];
			for (int k = 0; k<pTemplet->TmpSize() / 2 + 1; k++)
			{
				int offX = pTemplet->GetOffx(k);
				int offY = pTemplet->GetOffy(k);
				float tempDis = pTemplet->GetTmpDis(k);

				int x = j + offX;
				int y = i + offY;

				if (x<0 || y<0 || x>MtxWidth - 1 || y>MtxHeight - 1) continue;
				if (MinDis > DisMtx[y][x] + tempDis)
				{
					MinDis = DisMtx[y][x] + tempDis;
					DisMtx[i][j] = MinDis;
					LocMtx[i][j] = LocMtx[y][x];
				}
			}
		}
	//从右上到左下，八边形模板溜一遍
	for(int i=MtxHeight-1;i>=0;i--)
		for (int j = MtxWidth; j >= 0; j--)
		{
			if (fabs(DisMtx[i][j] - 0) < FLT_EPSILON) continue;
			float MinDis = DisMtx[i][j];
			for (int k = pTemplet->TmpSize() / 2; k < pTemplet->TmpSize(); k++)
			{
				int offX = pTemplet->GetOffx(k);
				int offY = pTemplet->GetOffy(k);
				float tempDis = pTemplet->GetTmpDis(k);

				int x = j + offX;
				int y = i + offY;

				if (x<0 || y<0 || x>MtxWidth - 1 || y>MtxHeight - 1) continue;
				if (MinDis > DisMtx[y][x] + tempDis)
				{
					MinDis = DisMtx[y][x] + tempDis;
					DisMtx[i][j] = MinDis;
					LocMtx[i][j] = LocMtx[y][x];
				}
			}
		}
	//将距离矩阵写入32位位图
	BITMAPFILEHEADER IdxFileHead; // 32位位图头结构
	BITMAPINFOHEADER IdxFileInfo;

	IdxFileHead.bfType = srcBmpHead.bfType;
	IdxFileHead.bfSize = 54 + srcBmpInfo.biWidth * 4 * srcBmpInfo.biHeight;//
	IdxFileHead.bfReserved1 = 0;
	IdxFileHead.bfReserved2 = 0;
	IdxFileHead.bfOffBits = 54;//

	IdxFileInfo.biSize = 40;
	IdxFileInfo.biWidth = srcBmpInfo.biWidth;//
	IdxFileInfo.biHeight = srcBmpInfo.biHeight;//
	IdxFileInfo.biPlanes = 1;
	IdxFileInfo.biBitCount = 32;//
	IdxFileInfo.biCompression = 0;
	IdxFileInfo.biSizeImage = 0;
	IdxFileInfo.biXPelsPerMeter = 0;
	IdxFileInfo.biYPelsPerMeter = 0;
	IdxFileInfo.biClrUsed = 0;
	IdxFileInfo.biClrImportant = 0;

	//写入距离场
	FILE * bmpWrite = fopen(OutDisname, "wb");
	fwrite(&IdxFileHead, sizeof(BITMAPFILEHEADER), 1, bmpWrite);
	fwrite(&IdxFileInfo, sizeof(BITMAPINFOHEADER), 1, bmpWrite);
	for (int k = 0; k<MtxHeight; k++)
	{
		fwrite(DisMtx[k], sizeof(float), MtxWidth, bmpWrite);
	}
	fclose(bmpWrite);

	//写入分配场矩阵256色位图
	FILE *allocate = fopen(OutLocname, "wb");
	fwrite(&srcBmpHead, sizeof(BITMAPFILEHEADER), 1, allocate);
	fwrite(&srcBmpInfo, sizeof(BITMAPINFOHEADER), 1, allocate);
	fwrite(ColorTab, 1024, 1, allocate);
	for (int i = 0; i<MtxHeight; i++)
		fwrite(LocMtx[i], BufWidth, 1, allocate);
	fclose(allocate);

	//清理，释放内存
	for (int i = 0; i < MtxHeight; i++)
	{
		delete[] LocMtx[i];
		delete[] DisMtx[i];
	}
	delete[] LocMtx;
	delete[] DisMtx;
	fclose(srcBmp);
	return 1;
}	

bool Bmpmaker::BmpOverlap(const char *InputBmpFileName1, const char *InputBmpFileName2, const char *OutputBmpFilename) {
	BITMAPFILEHEADER Bmp1Head, Bmp2Head;
	BITMAPINFOHEADER Bmp1Info, Bmp2Info;
	unsigned char ColorTab1[1024], ColorTab2[1024];

	FILE* file1 = fopen(InputBmpFileName1, "rb");
	FILE* file2 = fopen(InputBmpFileName2, "rb");
	if (file1 == NULL || file2 == NULL)
		return false;

	fread(&Bmp1Head, sizeof(BITMAPFILEHEADER), 1, file1);
	fread(&Bmp2Head, sizeof(BITMAPFILEHEADER), 1, file2);
	fread(&Bmp1Info, sizeof(BITMAPINFOHEADER), 1, file1);
	fread(&Bmp2Info, sizeof(BITMAPINFOHEADER), 1, file2);
	fread(ColorTab1, 1024, 1, file1);
	fread(ColorTab2, 1024, 1, file2);

	unsigned Mtx1_Height = Bmp1Info.biHeight;
	unsigned Mtx2_Height = Bmp2Info.biHeight;
	unsigned Mtx1_Width = Bmp1Info.biWidth;
	unsigned Mtx2_Width = Bmp2Info.biWidth;
	unsigned Buf_Width = (Mtx1_Width + 3) / 4 * 4;

	unsigned char** Bmp1_Buf = new unsigned char*[Mtx1_Height];
	unsigned char** Bmp2_Buf = new unsigned char*[Mtx2_Height];
	unsigned char** dest_Buf = new unsigned char*[Mtx1_Height];

	unsigned i, j;
	for (i = 0; i < Mtx1_Height; i++)
	{
		Bmp1_Buf[i] = new unsigned char[Buf_Width];
		fread(Bmp1_Buf[i], Buf_Width, 1, file1);
	}
	for (i = 0; i < Mtx2_Height; i++)
	{
		Bmp2_Buf[i] = new unsigned char[Buf_Width];
		fread(Bmp2_Buf[i], Buf_Width, 1, file2);
	}
	for (i = 0; i < Mtx1_Height; i++)
	{
		dest_Buf[i] = new unsigned char[Buf_Width];
	}

	for(i=0;i<Mtx1_Height;i++)
		for (j = 0; j < Buf_Width; j++)
		{
		    dest_Buf[i][j] = Bmp1_Buf[i][j] + Bmp2_Buf[i][j];
		}

	FILE *destBMP = fopen(OutputBmpFilename, "wb");
	fwrite(&Bmp1Head, sizeof(BITMAPFILEHEADER), 1, destBMP);
	fwrite(&Bmp1Info, sizeof(BITMAPINFOHEADER), 1, destBMP);
	fwrite(ColorTab1, 1024, 1, destBMP);

	for (i = 0; i < Mtx1_Height; i++)
	{
		fwrite(dest_Buf[i], Buf_Width, 1, destBMP);
		delete[] dest_Buf[i];
		delete[] Bmp1_Buf[i];
		delete[] Bmp2_Buf[i];
		Bmp1_Buf[i] = NULL;
		Bmp2_Buf[i] = NULL;
		dest_Buf[i] = NULL;
	}

	delete[] Bmp1_Buf;
	delete[] Bmp2_Buf;
	delete[] dest_Buf;
	fclose(file1);
	fclose(file2);
	fclose(destBMP);

	return true;
}

bool Bmpmaker::BmpSmooth(const char *InputBmpFilename, const char *OutputBmpFileName, int ModelWidth) {
	//1.read bmp and save to a mat
	//2.data processing
	//3.save result
	BITMAPFILEHEADER srcBmpHead;
	BITMAPINFOHEADER srcBmpInfo;
	unsigned char ColorTab[1024];

	FILE *srcBmp = fopen(InputBmpFilename, "rb");
	if (srcBmp == NULL) return false;

	fread(&srcBmpHead, sizeof(BITMAPFILEHEADER), 1, srcBmp);
	fread(&srcBmpInfo, sizeof(BITMAPINFOHEADER), 1, srcBmp);
	fread(ColorTab, 1024, 1, srcBmp);

	unsigned MtxHeight = srcBmpInfo.biHeight;
	unsigned MtxWidth = srcBmpInfo.biWidth;
	unsigned BufWidth = (MtxWidth + 3) / 4 * 4;

	//保存读取的图片矩阵
	unsigned char **srcBmpBuf = new unsigned char*[MtxHeight];
	//保存变换结果矩阵
	unsigned char **destBmpMtx = new unsigned char *[MtxHeight];
	//初始化二维数组
	for (int i = 0; i < MtxHeight; i++)
	{
		srcBmpBuf[i] = new unsigned char[BufWidth];
		destBmpMtx[i] = new unsigned char[BufWidth];
	}

	unsigned i, j;
	for (i = 0; i < MtxHeight; i++)
		fread(srcBmpBuf[i], BufWidth, 1, srcBmp);

	for (i = ModelWidth/2; i < MtxHeight-ModelWidth/2; i++)
	{
		for (j = ModelWidth/2; j < BufWidth - ModelWidth/2; j++)
		{
			int temp = 0;
			for (int x = -ModelWidth / 2; x < ModelWidth / 2+1; x++)
				for (int y = -ModelWidth / 2; y < ModelWidth / 2+1; y++)
					temp += srcBmpBuf[i+x][j+y];
			destBmpMtx[i][j] = temp / (ModelWidth*ModelWidth);
		}
	}


	FILE *destBmp = fopen(OutputBmpFileName, "wb");

	fwrite(&srcBmpHead, sizeof(BITMAPFILEHEADER), 1, destBmp);
	fwrite(&srcBmpInfo, sizeof(BITMAPINFOHEADER), 1, destBmp);
	fwrite(ColorTab, 1024, 1, destBmp);

	for (i = 0; i<MtxHeight; i++)
	{
		fwrite(destBmpMtx[i], BufWidth, 1, destBmp);
		delete[] destBmpMtx[i];
		destBmpMtx[i] = NULL; 
		delete[] srcBmpBuf[i];
		srcBmpBuf[i] = NULL;
	}

	delete[] srcBmpBuf;
	delete[] destBmpMtx;
	fclose(srcBmp);
	fclose(destBmp);
	return true;
}

bool Bmpmaker::BMP_Voronoi(const char* InputBMPFilePath, const char* OutFileName) {
		BITMAPFILEHEADER srcBmpHead;
	BITMAPINFOHEADER srcBmpInfo;
	unsigned char ColorTab[1024];

	FILE *srcBmp = fopen(InputBMPFilePath, "rb");
	if (srcBmp == NULL) return false;

	fread(&srcBmpHead, sizeof(BITMAPFILEHEADER), 1, srcBmp);
	fread(&srcBmpInfo, sizeof(BITMAPINFOHEADER), 1, srcBmp);
	fread(ColorTab, 1024, 1, srcBmp);

	unsigned MtxHeight = srcBmpInfo.biHeight;
	unsigned MtxWidth = srcBmpInfo.biWidth;
	unsigned BufWidth = (MtxWidth + 3) / 4 * 4;

	//保存读取的图片矩阵
	unsigned char **srcBmpBuf = new unsigned char*[MtxHeight];
	//保存变换结果矩阵
	unsigned char **destBmpMtx = new unsigned char *[MtxHeight];
	//初始化二维数组
	for (int i = 0; i < MtxHeight; i++)
	{
		srcBmpBuf[i] = new unsigned char[BufWidth];
		destBmpMtx[i] = new unsigned char[BufWidth];
	}

	unsigned i, j;
	//循环读取像素矩阵
	for (i = 0; i < MtxHeight; i++)
		fread(srcBmpBuf[i], BufWidth, 1, srcBmp);
	for(i=1;i<MtxHeight-1;i++)
		for (j = 1; j < BufWidth - 1; j++)
		{
			if (srcBmpBuf[i][j] == srcBmpBuf[i - 1][j] && srcBmpBuf[i][j] == srcBmpBuf[i + 1][j] && srcBmpBuf[i][j] == srcBmpBuf[i][j - 1] && srcBmpBuf[i][j] == srcBmpBuf[i][j + 1])
				destBmpMtx[i][j] = 255; //非边界点赋白色
			else
				destBmpMtx[i][j] = 0; //边界点赋黑色
		}
	
	//变换结束，将结果写回
	FILE *destBmp = fopen(OutFileName, "wb");

	fwrite(&srcBmpHead, sizeof(BITMAPFILEHEADER), 1, destBmp);
	fwrite(&srcBmpInfo, sizeof(BITMAPINFOHEADER), 1, destBmp);
	fwrite(ColorTab, 1024, 1, destBmp);

	for (i = 0; i<MtxHeight; i++)
	{
		fwrite(destBmpMtx[i], BufWidth, 1, destBmp);
		delete[] srcBmpBuf[i];
		srcBmpBuf[i] = NULL;
		delete[] destBmpMtx[i];
		destBmpMtx[i] = NULL;
	}

	delete[] srcBmpBuf;
	delete[] destBmpMtx;
	fclose(srcBmp);
	fclose(destBmp);
	return true;
}

bool Bmpmaker::BMP_Buffer(const char* InputBMPFilePath, const char* OutFileName, int distance) {
		//读取32位位图
	BITMAPFILEHEADER srcBmpHead;
	BITMAPINFOHEADER srcBmpInfo;

	FILE *srcBmp = fopen(InputBMPFilePath, "rb");
	if (srcBmp == NULL) return false;

	fread(&srcBmpHead, sizeof(BITMAPFILEHEADER), 1, srcBmp);
	fread(&srcBmpInfo, sizeof(BITMAPINFOHEADER), 1, srcBmp);

	unsigned MtxHeight = srcBmpInfo.biHeight;
	unsigned MtxWidth = srcBmpInfo.biWidth;

	//保存读取的图片矩阵
	float **srcBmpBuf = new float*[MtxHeight];
	//初始化二维数组，读取像素矩阵
	for (int i = 0; i < MtxHeight; i++)
	{
		srcBmpBuf[i] = new float[MtxWidth];
		fread(srcBmpBuf[i], sizeof(float), MtxWidth, srcBmp);
	}

	unsigned i, j;
	for(i=0;i<MtxHeight;i++)
		for (j = 0; j < MtxWidth; j++)
		{
			if (fabs(srcBmpBuf[i][j] - 0) < FLT_EPSILON) continue;
			if (srcBmpBuf[i][j] <= distance) //小于等于距离值，则填充XX颜色
				srcBmpBuf[i][j] = 100;
			else
				srcBmpBuf[i][j] = 0.2;
		}
	//写回去
	FILE * bmpWrite = fopen(OutFileName, "wb");
	fwrite(&srcBmpHead, sizeof(BITMAPFILEHEADER), 1, bmpWrite);
	fwrite(&srcBmpInfo, sizeof(BITMAPINFOHEADER), 1, bmpWrite);
	for (int k = 0; k<MtxHeight; k++)
	{
		fwrite(srcBmpBuf[k], sizeof(float), MtxWidth, bmpWrite);
	}
	fclose(bmpWrite);
}

bool Bmpmaker::BMP_MiddleLine(const char* InputBMPFilePath, const char* OutFileName) {
	BITMAPFILEHEADER srcBmpHead;
	BITMAPINFOHEADER srcBmpInfo;
	unsigned char ColorTab[1024];

	FILE *srcBmp = fopen(InputBMPFilePath, "rb");
	if (srcBmp == NULL)
		return false;

	fread(&srcBmpHead, sizeof(BITMAPFILEHEADER), 1, srcBmp);
	fread(&srcBmpInfo, sizeof(BITMAPINFOHEADER), 1, srcBmp);
	fread(ColorTab, 1024, 1, srcBmp);

	unsigned MtxHeight = srcBmpInfo.biHeight;
	unsigned MtxWidth = srcBmpInfo.biWidth;
	unsigned BufWidth = (MtxWidth + 3) / 4 * 4;

	//保存读取的图片矩阵
	unsigned char **srcBmpBuf = new unsigned char*[MtxHeight];
	//保存变换结果矩阵
	unsigned char **destBmpMtx = new unsigned char *[MtxHeight];
	//初始化二维数组,并读取像素矩阵
	for (int i = 0; i < MtxHeight; i++)
	{
		srcBmpBuf[i] = new unsigned char[BufWidth];
		destBmpMtx[i] = new unsigned char[BufWidth];
		fread(srcBmpBuf[i], BufWidth, 1, srcBmp);
	}

	unsigned i, j;
	for(i=1;i<MtxHeight-1;i++)
		for (j = 1; j < BufWidth - 1; j++)
		{
			destBmpMtx[i][j] = 255;  
			if (srcBmpBuf[i][j] == 0) continue; //只对多边形内部进行操作
			if (srcBmpBuf[i][j] != srcBmpBuf[i - 1][j] || srcBmpBuf[i][j] != srcBmpBuf[i + 1][j] || srcBmpBuf[i][j] != srcBmpBuf[i][j - 1] || srcBmpBuf[i][j] != srcBmpBuf[i][j + 1])
				destBmpMtx[i][j] = 0; //边界点赋黑色
		}
	//变换结束，将结果写回
	FILE *destBmp = fopen(OutFileName, "wb");

	fwrite(&srcBmpHead, sizeof(BITMAPFILEHEADER), 1, destBmp);
	fwrite(&srcBmpInfo, sizeof(BITMAPINFOHEADER), 1, destBmp);
	fwrite(ColorTab, 1024, 1, destBmp);

	for (i = 0; i<MtxHeight; i++)
	{
		fwrite(destBmpMtx[i], BufWidth, 1, destBmp);
		delete[] srcBmpBuf[i];
		srcBmpBuf[i] = NULL;
		delete[] destBmpMtx[i];
		destBmpMtx[i] = NULL;
	}

	delete[] srcBmpBuf;
	delete[] destBmpMtx;
	fclose(srcBmp);
	fclose(destBmp);
	return true;
}

bool Bmpmaker::ScanSrcPtCoors(const char* SourceFileName, const char* CoorsTableFile) {
	BITMAPFILEHEADER srcBmpHead;
	BITMAPINFOHEADER srcBmpInfo;
	unsigned char ColorTab[1024];

	FILE *srcBmp = fopen(SourceFileName, "rb");
	if (srcBmp == NULL)
		return false;

	fread(&srcBmpHead, sizeof(BITMAPFILEHEADER), 1, srcBmp);
	fread(&srcBmpInfo, sizeof(BITMAPINFOHEADER), 1, srcBmp);
	fread(ColorTab, 1024, 1, srcBmp);

	unsigned MtxHeight = srcBmpInfo.biHeight;
	unsigned MtxWidth = srcBmpInfo.biWidth;
	unsigned BufWidth = (MtxWidth + 3) / 4 * 4;

	unsigned char **srcBmpBuf = new unsigned char*[MtxHeight];

	unsigned i, j;
	for (i = 0; i < MtxHeight; i++)
	{
		srcBmpBuf[i] = new unsigned char[BufWidth];
		fread(srcBmpBuf[i], BufWidth, 1, srcBmp);
	}
	FILE* PtCoords = fopen(CoorsTableFile,"w");
	for(i=0;i<MtxHeight;i++)
		for (j = 0; j < BufWidth; j++)
		{
			if (srcBmpBuf[i][j] == 255||srcBmpBuf[i][j]==0) continue;
			else
				fprintf(PtCoords, "%d %d %d\n", srcBmpBuf[i][j], i, j); //依次写入颜色值、行号、列号，注意下标从0开始
		}
	for (i = 0; i < MtxHeight; i++)
		delete[] srcBmpBuf[i];
	delete[] srcBmpBuf;
	srcBmpBuf = NULL;
	fclose(PtCoords);
	fclose(srcBmp);
	return true;
}

bool Bmpmaker::GetTinPtPairs(const char* LocFileName, const char* PtPairsFile) {
	BITMAPFILEHEADER srcBmpHead;
	BITMAPINFOHEADER srcBmpInfo;
	unsigned char ColorTab[1024];

	FILE *srcBmp = fopen(LocFileName, "rb");
	if (srcBmp == NULL) return false;

	fread(&srcBmpHead, sizeof(BITMAPFILEHEADER), 1, srcBmp);
	fread(&srcBmpInfo, sizeof(BITMAPINFOHEADER), 1, srcBmp);
	fread(ColorTab, 1024, 1, srcBmp);

	unsigned MtxHeight = srcBmpInfo.biHeight;
	unsigned MtxWidth = srcBmpInfo.biWidth;
	unsigned BufWidth = (MtxWidth + 3) / 4 * 4;

	//保存读取的图片矩阵
	unsigned char **srcBmpBuf = new unsigned char*[MtxHeight];
	unsigned int* PtPairs = new unsigned int[1000]; //声明一个数组用于保存点对
	int top = 0; //定义一个栈顶指针，指向最后一个数组元素的下标
	//初始化并读取像素矩阵
	for (int i = 0; i < MtxHeight; i++)
	{
		srcBmpBuf[i] = new unsigned char[BufWidth];
		fread(srcBmpBuf[i], BufWidth, 1, srcBmp);
	}

	//遍历像素矩阵，获取点对
	for(int i=1;i<MtxHeight-1;i++)
		for (int j = 1; j < BufWidth-1; j++)
		{
			//若检测到了边界点，则将其加入点对数组
			if (srcBmpBuf[i][j] != srcBmpBuf[i - 1][j] || srcBmpBuf[i][j] != srcBmpBuf[i + 1][j] || srcBmpBuf[i][j] != srcBmpBuf[i][j - 1] || srcBmpBuf[i][j] != srcBmpBuf[i][j + 1])
			{
				unsigned int pair = 0;
				if (srcBmpBuf[i][j] != srcBmpBuf[i - 1][j])
				{
					//老师说较小的乘以1000加上大的，我觉得挺好
					if (srcBmpBuf[i][j] < srcBmpBuf[i - 1][j])
					{
						pair = srcBmpBuf[i][j] * 1000 + srcBmpBuf[i - 1][j];
						SaveToArray(PtPairs, top, pair);
					}
					else
					{
						pair = srcBmpBuf[i-1][j] * 1000 + srcBmpBuf[i][j];
						SaveToArray(PtPairs, top, pair);
					}
				}
				else if (srcBmpBuf[i][j] != srcBmpBuf[i + 1][j])
				{
					if(srcBmpBuf[i][j]<srcBmpBuf[i+1][j])
					{
						pair = srcBmpBuf[i][j] * 1000 + srcBmpBuf[i + 1][j];
						SaveToArray(PtPairs, top, pair);
					}
					else {
						pair = srcBmpBuf[i+1][j] * 1000 + srcBmpBuf[i][j];
						SaveToArray(PtPairs, top, pair);
					}
				}
				else if (srcBmpBuf[i][j] != srcBmpBuf[i][j - 1])
				{
					if (srcBmpBuf[i][j] < srcBmpBuf[i][j - 1])
					{
						pair = srcBmpBuf[i][j] * 1000 + srcBmpBuf[i][j-1];
						SaveToArray(PtPairs, top, pair);
					}
					else
					{
						pair = srcBmpBuf[i][j-1] * 1000 + srcBmpBuf[i][j];
						SaveToArray(PtPairs, top, pair);
					}
				}
				else
				{
					if (srcBmpBuf[i][j] < srcBmpBuf[i][j + 1])
					{
						pair = srcBmpBuf[i][j] * 1000 + srcBmpBuf[i][j+1];
						SaveToArray(PtPairs, top, pair);
					}
					else {
						pair = srcBmpBuf[i][j+1] * 1000 + srcBmpBuf[i][j];
						SaveToArray(PtPairs, top, pair);
					}			
				}
			}
		}
	//将点对写入文件
	FILE* Pts = fopen(PtPairsFile, "w");
	for (int i = 0; i < top; i++)
		fprintf(Pts, "%d\n", PtPairs[i]);
	fclose(Pts);
	fclose(srcBmp);
	return true;
}

bool Bmpmaker::LinkPts(const char* SourceFileName, const char* PtPairsFile, const char* CoorsTableFile) {
	//读取原位图
	BITMAPFILEHEADER srcBmpHead;
	BITMAPINFOHEADER srcBmpInfo;
	unsigned char ColorTab[1024];

	FILE *srcBmp = fopen(SourceFileName, "rb");
	if (srcBmp == NULL)
		return false;

	fread(&srcBmpHead, sizeof(BITMAPFILEHEADER), 1, srcBmp);
	fread(&srcBmpInfo, sizeof(BITMAPINFOHEADER), 1, srcBmp);
	fread(ColorTab, 1024, 1, srcBmp);

	unsigned MtxHeight = srcBmpInfo.biHeight;
	unsigned MtxWidth = srcBmpInfo.biWidth;
	unsigned BufWidth = (MtxWidth + 3) / 4 * 4;

	unsigned char **srcBmpBuf = new unsigned char*[MtxHeight];

	unsigned i, j;
	for (i = 0; i < MtxHeight; i++)
	{
		srcBmpBuf[i] = new unsigned char[BufWidth];
		fread(srcBmpBuf[i], BufWidth, 1, srcBmp);
	}

	//读取颜色坐标对应表
	Pt* Pts = new Pt[100];
	int top1 = 0;
	FILE* fp1 = fopen(CoorsTableFile, "r");
	while (!feof(fp1)) //用这种方法读，最后一行会读两次，很奇怪==，好在我有栈
	{
		Pt point;
		fscanf(fp1, "%d %d %d", &point.color, &point.row, &point.column);
		Pts[top1] = point;
		top1++;
	}
	top1--; //为了避免不必要的麻烦，将栈顶指针下移
	fclose(fp1);

	//读取要连接的点对
	int pairs[1000];
	int top2 = 0;
	FILE* fp2 = fopen(PtPairsFile, "r");
	while (!feof(fp2))
	{
		int pair;
		fscanf(fp2, "%d", &pair);
		pairs[top2] = pair;
		top2++;
	}
	top2--; //为了避免不必要的麻烦，将栈顶指针下移
	fclose(fp2);

	//用中点画线法连接点对
	for (i = 0; i < top2; i++)
	{
		int point1 = pairs[i] / 1000;
		int point2 = pairs[i] % 1000;
		Pt pt1, pt2;
		for (j = 0; j < top1; j++)
		{
			if (Pts[j].color == point1)
				pt1 = Pts[j];
			if (Pts[j].color == point2)
				pt2 = Pts[j];
		}
		if (pt1.column <= pt2.column)
			line(pt1.column, pt1.row, pt2.column, pt2.row, srcBmpBuf);
		else
			line(pt2.column, pt2.row, pt1.column, pt1.row, srcBmpBuf);
	}

	//输出Delauney三角网
	FILE *destBmp = fopen("Delauney_trigger.bmp", "wb");

	fwrite(&srcBmpHead, sizeof(BITMAPFILEHEADER), 1, destBmp);
	fwrite(&srcBmpInfo, sizeof(BITMAPINFOHEADER), 1, destBmp);
	fwrite(ColorTab, 1024, 1, destBmp);

	for (i = 0; i<MtxHeight; i++)
	{
		fwrite(srcBmpBuf[i], BufWidth, 1, destBmp);
		delete[] srcBmpBuf[i];
		srcBmpBuf[i] = NULL;
	}
	fclose(destBmp);
	fclose(srcBmp);
}

void Bmpmaker::line(int x1, int y1, int x2, int y2, unsigned char** srcBmpBuf) {
	double increx, increy, length;
	double x, y;
	int i;
	//选择增值较大的方向
	if (abs(x2 - x1) > abs(y2 - y1))
		length = abs(x2 - x1);
	else
		length = abs(y2 - y1);
	increx = (x2 - x1) / length; //x方向增量
	increy = (y2 - y1) / length; //y方向增量
	x = x1; y = y1;
	for (i = 1; i <= length; i++)
	{
		srcBmpBuf[int(y)][int(x)] = 0;
		x += increx;
		y += increy;
	}
}

