#include"Header/Conversion.h"

/*
 * 将RGB数据存储为bmp格式的图片；
 * */
void RGB2BMP(unsigned char *rgb_buffer, int nWidth, int nHeight, int iFrame)
{
	char szFilename[32];
	//定义bmp文件的头
	typedef struct
	{
		long imageSize;
		long blank;
		long startPosition;
	} BmpHead;

	typedef struct
	{
		long Length;
		long width;
		long height;
		unsigned short colorPlane;
		unsigned short bitColor;
		long zipFormat;
		long realSize;
		long xPels;
		long yPels;
		long colorUse;
		long colorImportant;
	} InfoHead;

	typedef struct
	{
		unsigned char rgbBlue;
		unsigned char rgbGreen;
		unsigned char rgbRed;
		unsigned char rgbReserved;
	} RGBMixPlate;

	BmpHead m_BMPHeader;
	InfoHead m_BMPInfoHeader;
	char bfType[2] =
	{ 'B', 'M' };

	sprintf(szFilename, "/sdcard/kuvision/frame%d.bmp", iFrame);
	FILE* fp = fopen(szFilename, "wb");
	if (fp == NULL)
	{
		return;
	}
	m_BMPHeader.imageSize = 3 * nWidth * nHeight + 54;
	m_BMPHeader.blank = 0;
	m_BMPHeader.startPosition = 54;

	fwrite(bfType, 1, sizeof(bfType), fp);
	fwrite(&m_BMPHeader.imageSize, 1, sizeof(m_BMPHeader.imageSize), fp);
	fwrite(&m_BMPHeader.blank, 1, sizeof(m_BMPHeader.blank), fp);
	fwrite(&m_BMPHeader.startPosition, 1, sizeof(m_BMPHeader.startPosition), fp);

	m_BMPInfoHeader.Length = 40;
	m_BMPInfoHeader.width = nWidth;
	m_BMPInfoHeader.height = nHeight;
	m_BMPInfoHeader.colorPlane = 1;
	m_BMPInfoHeader.bitColor = 24;
	m_BMPInfoHeader.zipFormat = 0;
	m_BMPInfoHeader.realSize = 3 * nWidth * nHeight;
	m_BMPInfoHeader.xPels = 0;
	m_BMPInfoHeader.yPels = 0;
	m_BMPInfoHeader.colorUse = 0;
	m_BMPInfoHeader.colorImportant = 0;

	fwrite(&m_BMPInfoHeader.Length, 1, sizeof(m_BMPInfoHeader.Length), fp);
	fwrite(&m_BMPInfoHeader.width, 1, sizeof(m_BMPInfoHeader.width), fp);
	fwrite(&m_BMPInfoHeader.height, 1, sizeof(m_BMPInfoHeader.height), fp);
	fwrite(&m_BMPInfoHeader.colorPlane, 1, sizeof(m_BMPInfoHeader.colorPlane),
			fp);
	fwrite(&m_BMPInfoHeader.bitColor, 1, sizeof(m_BMPInfoHeader.bitColor), fp);
	fwrite(&m_BMPInfoHeader.zipFormat, 1, sizeof(m_BMPInfoHeader.zipFormat), fp);
	fwrite(&m_BMPInfoHeader.realSize, 1, sizeof(m_BMPInfoHeader.realSize), fp);
	fwrite(&m_BMPInfoHeader.xPels, 1, sizeof(m_BMPInfoHeader.xPels), fp);
	fwrite(&m_BMPInfoHeader.yPels, 1, sizeof(m_BMPInfoHeader.yPels), fp);
	fwrite(&m_BMPInfoHeader.colorUse, 1, sizeof(m_BMPInfoHeader.colorUse), fp);
	fwrite(&m_BMPInfoHeader.colorImportant, 1,
			sizeof(m_BMPInfoHeader.colorImportant), fp);
	fwrite(rgb_buffer, 3 * nWidth * nHeight, 1, fp);

	fclose(fp);
}
