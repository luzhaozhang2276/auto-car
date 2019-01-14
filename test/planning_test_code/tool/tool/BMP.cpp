#include "BMP.h"


BMP::BMP()
{
}


BMP::~BMP()
{
}


bool BMP::readBmp(char *bmpName)
{
	BITMAPFILEHEADER fh;
	BITMAPINFOHEADER ih;
	

	ifstream src(bmpName, ifstream::binary); //2.bmpΪ�Ҷ�ͼ��
	if (!src)
	{
		cerr << "��ʧ��";
		return 0;
	}
	src.read((char *)&fh, sizeof(BITMAPFILEHEADER));
	if (fh.bfType != 'MB')
	{
		cerr << "�ⲻ��BMPͼ��";
		return 0;
	}
	src.read((char *)&ih, sizeof(BITMAPINFOHEADER));
	rows = ih.biHeight;
	cols = ih.biWidth;
	//bmpͼ�����ݴ洢��4�ֽڶ����
	int colsDQ = (cols + 3) / 4 * 4;
	//�ƶ���ͼ�����ݴ�
	src.seekg(sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)* 256, ios::beg);//���������λ�ã����
	//��ȡͼ������
	unsigned char *pData = new unsigned char[rows*colsDQ];
	src.read((char *)pData, rows*colsDQ);

	data.resize(rows);
	for (auto &vec : data)
		vec.resize(cols);

	for (int i = rows - 1; i >= 0; --i) //bmpͼ��洢Ϊ�е���
	for (int j = 0; j < cols; ++j)
	{
		if (*(pData + i*colsDQ + j) < 400)
			data[rows - 1 - i][j] = 0;
		else
			data[rows - 1 - i][j] = 1;

	}
	return 1;//��ȡ�ļ��ɹ�
}


bool BMP::Read24BMP(const char *strFileName)
{
	if (!strFileName)	return 0;

	BITMAPFILEHEADER	FileHeader;
	BITMAPINFOHEADER	InfoHeader;
	FILE*				fp;
	BYTE			    *pCur;
	BYTE				tmp;
	int					ImgSize, Patch, Extend, i;

	if ((fp = fopen(strFileName, "rb")) == NULL) return 0;
	if (fread((void *)&FileHeader, 1, 14, fp) != 14){ fclose(fp); return 0; }
	if (fread((void *)&InfoHeader, 1, 40, fp) != 40){ fclose(fp); return 0; }
	if (FileHeader.bfOffBits < 54){ fclose(fp); return 0; }
	if (InfoHeader.biBitCount != 24){ fclose(fp); return 0; }

	cols = (int)InfoHeader.biWidth;
	rows = (int)InfoHeader.biHeight;
	Patch = 3 * cols;
	ImgSize = Patch*rows;
	fseek(fp, FileHeader.bfOffBits, SEEK_SET);

	if ((pBmpBuf = new BYTE[ImgSize]) == NULL){ fclose(fp); return 0; }

	Extend = (cols * 3 + 3) / 4 * 4 - cols * 3;
	for (pCur = pBmpBuf + ImgSize - Patch;
		pCur >= pBmpBuf;
		pCur -= Patch)
	{
		if (fread((void *)pCur, 1, Patch, fp) != UINT(Patch))
		{
			delete[]pBmpBuf; fclose(fp); return 0;
		}
		for (i = 0; i < Extend; i++)
		if (fread(&tmp, 1, 1, fp) != 1)
		{
			delete[]pBmpBuf; fclose(fp); return 0;
		}
	}
	fclose(fp);
	bmpbuf_to_vec();
	return 1;
}

bool BMP::Write24BMP(const char *strFileName)
{
	vec_to_bmpbuf();
	if (rows*cols == 0) return false;
	BYTE *pImg = pBmpBuf;

	if (!strFileName)	return false;
	if (!pImg)			return false;

	BITMAPFILEHEADER	FileHeader;
	BITMAPINFOHEADER	InfoHeader;
	FILE				*fp;
	BYTE				*pCur;
	int					ImgSize, Patch, Extend, i;

	Patch = cols * 3;
	ImgSize = (cols + 3) / 4 * 4 * rows;

	if ((fp = fopen(strFileName, "wb+")) == NULL) return false;
	FileHeader.bfType = ((WORD)('M' << 8) | 'B');
	FileHeader.bfOffBits = 54; // sizeof(FileHeader)+sizeof(InfoHeader)
	FileHeader.bfSize = FileHeader.bfOffBits + ImgSize;
	FileHeader.bfReserved1 = 0;
	FileHeader.bfReserved2 = 0;

	InfoHeader.biSize = 40;
	InfoHeader.biWidth = cols;
	InfoHeader.biHeight = rows;
	InfoHeader.biPlanes = 1;
	InfoHeader.biBitCount = 24;
	InfoHeader.biCompression = 0;
	InfoHeader.biSizeImage = ImgSize;
	InfoHeader.biXPelsPerMeter = 0;
	InfoHeader.biYPelsPerMeter = 0;
	InfoHeader.biClrUsed = 0;
	InfoHeader.biClrImportant = 0;

	if (fwrite((void *)&FileHeader, 1, 14, fp) != 14)
	{
		fclose(fp); remove(strFileName); return false;
	}
	if (fwrite((void *)&InfoHeader, 1, 40, fp) != 40)
	{
		fclose(fp); remove(strFileName); return false;
	}

	Extend = (cols * 3 + 3) / 4 * 4 - cols * 3;
	for (pCur = pImg + (rows - 1)*Patch;
		pCur >= pImg;
		pCur -= Patch)
	{
		if (fwrite((void *)pCur, 1, Patch, fp) != UINT(Patch))
		{
			fclose(fp); remove(strFileName); return false;
		}

		for (i = 0; i < Extend; i++)
		if (fwrite((void *)(pCur + Patch - 3), 1, 1, fp) != 1)
		{
			fclose(fp); remove(strFileName); return false;
		}

	}
	fclose(fp);
	return true;
}


void BMP::bmpbuf_to_vec(){
	//resize
	data_R.resize(rows);
	for (auto &vec : data_R)
		vec.resize(cols);
	data_G.resize(rows);
	for (auto &vec : data_G)
		vec.resize(cols);
	data_B.resize(rows);
	for (auto &vec : data_B)
		vec.resize(cols);
	data.resize(rows);
	for (auto &vec : data)
		vec.resize(cols);

	for (int i = 0; i<rows; i++)
	{
		for (int j = 0; j<cols; j++)
		{
			data_R[rows - 1 - i][j] = pBmpBuf[j * 3 + 2 + cols*i * 3];
			data_G[rows - 1 - i][j] = pBmpBuf[j * 3 + 1 + cols *i * 3];
			data_B[rows - 1 - i][j] = pBmpBuf[j * 3 + cols *i * 3];
			if (data_R[rows - 1 - i][j] + data_G[rows - 1 - i][j] + data_B[rows - 1 - i][j]< 400)
				data[rows - 1 - i][j] = 0;
			else
				data[rows - 1 - i][j] = 1;
		}
	}
}


void BMP::vec_to_bmpbuf(){
	for (int i = 0; i<rows; i++)
	{
		for (int j = 0; j<cols; j++)
		{
			pBmpBuf[j * 3 + 2 + cols*i * 3] = data_R[rows - 1 - i][j];
			pBmpBuf[j * 3 + 1 + cols *i * 3] = data_G[rows - 1 - i][j];
			pBmpBuf[j * 3 + cols *i * 3] = data_B[rows - 1 - i][j];
		}
	}
}

void BMP::setcolor(int x, int y, char color){
	switch (color)
	{
		//��
	case 'r': data_R[x][y] = 255;
			  data_G[x][y] = 0;
			  data_B[x][y] = 0;
			  break;
	//��
	case 'g': data_R[x][y] = 0;
			  data_G[x][y] = 255;
		      data_B[x][y] = 0;
			  break;
	//��
	case 'b': data_R[x][y] = 0;
		      data_G[x][y] = 0;
		      data_B[x][y] = 255;
			  break;
			  //��
	case 'W': data_R[x][y] = 255;
		      data_G[x][y] = 255;
		      data_B[x][y] = 255;
			  break;
			  //��
	case 'B': data_R[x][y] = 0;
		      data_G[x][y] = 0;
		      data_B[x][y] = 0;
			  break;
			  //��
	case 'G': data_R[x][y] = 100;
			  data_G[x][y] = 100;
		      data_B[x][y] = 100;
		      break;
	default:
		break;
	}

}


void BMP::compressionBMP(BMP& img, double scale){
	//����ͼ��ߴ�
	img.rows = round(scale*rows);
	img.cols = round(scale*cols);
	
	//����pBmpBuf
	int lineByte2 = (img.cols * 3 + 3) / 4 * 4;
	img.pBmpBuf = new unsigned char[lineByte2 * img.rows];///Ϊ���ź�ͼ�����洢�ռ�
	int pre_i, pre_j, after_i, after_j;//����ǰ���Ӧ�����ص�����
	for (int i = 0; i<img.rows; i++)
	{
		for (int j = 0; j<img.cols; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				after_i = i;
				after_j = j;
				pre_i = min(rows, round(after_i / scale));/////ȡ������ֵ����Ϊ�����ڽ���ֵ������ȡ������
				pre_j = min(cols, round(after_j / scale));
				//��ԭͼ��Χ��
				*(img.pBmpBuf + i * lineByte2 + j * 3 + k) = *(pBmpBuf + pre_i * cols * 3 + pre_j * 3 + k);
			}
		}
	}
	//�������ؾ���
	img.bmpbuf_to_vec();
}