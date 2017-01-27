#include "BitmapProcessor.h"
#include <cassert>

#pragma pack(push, 1)
typedef struct BitmapStruct
{
	uint16_t bfType;
	uint32_t bfSize;
	uint16_t bfReserved1;
	uint16_t bfReserved2;
	uint32_t bfOffBits;

	uint32_t biSize;
	int32_t  biWidth;
	int32_t  biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	int32_t  biXPelsPerMeter;
	int32_t  biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
}BitmapStruct;
#pragma pack(pop)

Bitmap::Bitmap() :
	Opened(false)
{
}

Bitmap::~Bitmap()
{
}

Bitmap::operator bool() const
{
	return Opened;
}

int32_t Bitmap::Width() const
{
	return reinterpret_cast<BitmapStruct*>(FileData.get())->biWidth;
}

int32_t Bitmap::Height() const
{
	return reinterpret_cast<BitmapStruct*>(FileData.get())->biHeight;
}

uint32_t Bitmap::BitCount() const
{
	return reinterpret_cast<BitmapStruct*>(FileData.get())->biBitCount;
}

uint32_t Bitmap::SizeImage() const
{
	return reinterpret_cast<BitmapStruct*>(FileData.get())->biSizeImage;
}

uint8_t* Bitmap::Image() const
{
	uint32_t bfOffBits = reinterpret_cast<BitmapStruct*>(FileData.get())->bfOffBits;
	return reinterpret_cast<uint8_t*>(FileData.get() + bfOffBits);
}

bool Bitmap::Creat(const int32_t Width, const int32_t Height, uint16_t BitPerPixel)
{
	switch (BitPerPixel)
	{
	case 1:
	case 4:
	case 8:
	case 16:
	case 24:
	case 32:
		break;

	default:
		return false;
	}

	size_t ImageSize_t = (Width * Height) * BitPerPixel / 8;
	size_t FileSize_t = sizeof(BitmapStruct) + ImageSize_t;

	try
	{
		FileSize = static_cast<streamsize>(FileSize_t);
		FileData = make_unique<char[]>(FileSize_t);
	}
	catch (const std::bad_alloc)
	{
		return false;
	}

	BitmapStruct *pBitmap = reinterpret_cast<BitmapStruct*>(FileData.get());
	pBitmap->bfType = 19778;
	pBitmap->bfSize = FileSize_t;
	pBitmap->bfReserved1 = 0;
	pBitmap->bfReserved2 = 0;
	pBitmap->bfOffBits = 54;

	pBitmap->biSize = 40;
	pBitmap->biWidth = Width;
	pBitmap->biHeight = Height;
	pBitmap->biPlanes = 1;
	pBitmap->biBitCount = BitPerPixel;
	pBitmap->biCompression = 0;
	pBitmap->biSizeImage = ImageSize_t;
	pBitmap->biXPelsPerMeter = 0;
	pBitmap->biYPelsPerMeter = 0;
	pBitmap->biClrUsed = 0;
	pBitmap->biClrImportant = 0;

	Opened = true;
	return true;
}

bool Bitmap::Read(const char* FileName)
{
	ifstream File(FileName, ifstream::binary);

	bool bResult = (File.rdstate() & File.failbit) == 0;
	if (false == bResult)
	{
		return false;
	}

	bResult = File.seekg(0, File.end).good();
	if (false == bResult)
	{
		return false;
	}

	FileSize = static_cast<streamsize>(File.tellg());
	if (-1 == FileSize)
	{
		return false;
	}

	bResult = File.seekg(0, File.beg).good();
	if (false == bResult)
	{
		return false;
	}

	if (true == Opened)
	{
		FileData.reset();
	}

	try
	{
		FileData = make_unique<char[]>(static_cast<size_t>(FileSize));
	}
	catch (const std::bad_alloc)
	{
		return false;
	}

	bResult = File.read(FileData.get(), FileSize).good();
	if (false == bResult)
	{
		return false;
	}

	Opened = true;
	return true;
}

bool Bitmap::Write(const char* FileName)
{
	ofstream File(FileName, ofstream::out | ofstream::binary);

	bool bResult = (File.rdstate() & File.failbit) == 0;
	if (false == bResult)
	{
		return false;
	}

	bResult = File.write(FileData.get(), FileSize).good();
	if (false == bResult)
	{
		return false;
	}

	return true;
}

bool Bitmap::Copy(Bitmap &rBitmap)
{
	if (false == rBitmap.Opened)
	{
		return false;
	}

	if (true == Opened)
	{
		FileData.reset();
	}

	FileSize = rBitmap.FileSize;
	size_t _Size = static_cast<size_t>(FileSize);

	try
	{
		FileData = make_unique<char[]>(_Size);
	}
	catch (const std::bad_alloc)
	{
		return false;
	}

	memcpy(FileData.get(), rBitmap.FileData.get(), _Size);

	Opened = true;
	return true;
}

bool Bitmap::Grayscale()
{
	if (false == Opened)
	{
		return false;
	}

	uint8_t *pImage = Image();
	uint32_t Channel = BitCount() / 8;
	uint32_t SizeImage = Bitmap::SizeImage();

	for (uint32_t Index = 0; Index < SizeImage; Index += Channel)
	{
		int Blue = static_cast<int>(pImage[Index]);
		int Green = static_cast<int>(pImage[Index + 1]);
		int Red = static_cast<int>(pImage[Index + 2]);

		int Grey = (Red * 299 + Green * 587 + Blue * 114 + 500) / 1000;
		memset(&pImage[Index], Grey, Channel);
	}

	return true;
}

bool Bitmap::FindTargetCoordinate(int *pTargetX, int *pTargetY)
{
	if (false == Opened)
	{
		return false;
	}

	int32_t Width = Bitmap::Width();
	int32_t Height = Bitmap::Height();
	uint32_t Channel = BitCount() / 8;
	uint8_t *pImage = Image();

	int WhitePointCount = 0;
	int TrackX = 0;
	int TrackY = 0;

	for (int Index = 0; Index < Height; Index++)
	{
		if (pImage[Width * Index * Channel] >= 250)
		{
			WhitePointCount++;
			TrackY = Index;
			TrackX = 0;
		}
		else if (pImage[(Width * Index + (Width - 1)) * Channel] >= 250)
		{
			WhitePointCount++;
			TrackY = Index;
			TrackX = Width - 1;
		}
		else
		{
			if (WhitePointCount < 30)
			{
				WhitePointCount = 0;
			}
			else
			{
				TrackY -= WhitePointCount / 2;
				break;
			}
		}
	}

	if (WhitePointCount == 0)
	{
		for (int Index = 0; Index < Width; Index++)
		{
			if (pImage[(Width * (Height - 1) + Index) * Channel] >= 250)
			{
				WhitePointCount++;
				TrackY = Height - 1;
				TrackX = Index;
			}
			else
			{
				if (WhitePointCount < 30)
				{
					WhitePointCount = 0;
				}
				else
				{
					TrackX -= WhitePointCount / 2;
					break;
				}
			}
		}
	}

	if (WhitePointCount != 0)
	{
		*pTargetX = TrackX;
		*pTargetY = TrackY;
	}
	else
	{
		*pTargetX = -1;
		*pTargetY = -1;
	}
	return true;
}

bool Bitmap::Sobel(Bitmap & rBitmap)
{
	if (false == Opened || false == rBitmap.Opened)
	{
		return false;
	}

	if (FileSize != rBitmap.FileSize)
	{
		return false;
	}

	if (this == &rBitmap)
	{
		return false;
	}

	int32_t Width = Bitmap::Width();
	int32_t Height = Bitmap::Height();
	uint32_t Channel = BitCount() / 8;

	int32_t WidthSub = Width - 1;
	int32_t HeightSub = Height - 1;

	uint8_t *pSourcImage = rBitmap.Image();
	uint8_t *pSobelImage = Image();

	auto SourcPixel = [pSourcImage, Width, HeightSub, Channel](int32_t x, int32_t y)->int
	{
		uint8_t Pixel = pSourcImage[(x + (HeightSub - y) * Width) * Channel];
		return static_cast<int>(Pixel);
	};

	auto SobelPixel = [pSobelImage, Width, HeightSub, Channel](int32_t x, int32_t y)->uint8_t&
	{
		return pSobelImage[(x + (HeightSub - y) * Width) * Channel];
	};

	for (int32_t WidthSubSub = WidthSub - 1, y = 1; y < HeightSub; y++)
	{
		for (int32_t x = 1; x < WidthSub; x++)
		{
			int Gx = 0
				- 1 * SourcPixel(x - 1, y - 1) + 1 * SourcPixel(x + 1, y - 1)
				- 2 * SourcPixel(x - 1, y + 0) + 2 * SourcPixel(x + 1, y + 0)
				- 1 * SourcPixel(x - 1, y + 1) + 1 * SourcPixel(x + 1, y + 1);

			int Gy = 0
				- 1 * SourcPixel(x - 1, y - 1) - 2 * SourcPixel(x + 0, y - 1) - 1 * SourcPixel(x + 1, y - 1)
				+ 1 * SourcPixel(x - 1, y + 1) + 2 * SourcPixel(x + 0, y + 1) + 1 * SourcPixel(x + 1, y + 1);

			double G = sqrt(Gx * Gx + Gy * Gy);
			uint8_t Pixel = 255.0 < G ? 255 : static_cast<uint8_t>(G);

			memset(&SobelPixel(x, y), Pixel, Channel);
		}

		memset(&SobelPixel(0, y), SobelPixel(1, y), Channel);
		memset(&SobelPixel(WidthSub, y), SobelPixel(WidthSubSub, y), Channel);
	}

	for (int32_t HeightSubSub = HeightSub - 1, x = 0; x < Width; x++)
	{
		uint8_t PixelTop = SobelPixel(x, 1);
		memset(&SobelPixel(x, 0), PixelTop, Channel);

		uint8_t PixelBottom = SobelPixel(x, HeightSubSub);
		memset(&SobelPixel(x, HeightSub), PixelBottom, Channel);
	}

	return true;
}

//bool Bitmap::Canny(Bitmap & rBitmap, char upper_threshold, char low_threshold)
//{
//	int data, x, y, row;
//	int Gx, Gy;
//	int Sobel_ans, Canny_ans;
//	double weight, g1, g2, g3, g4;
//	int add01 = 0, add02 = 0, add03 = 0;
//	double Gradient_Direct;
//
//	FILE *csv = fopen("Canny.csv", "wb");
//	FILE *img = fopen("Canny.bmp", "wb");
//
//	int temp;
//	//**********************************************************************Initial two dimension dynamic array
//	unsigned char *TempData = (unsigned char*)malloc(filesize * sizeof(unsigned char));
//	unsigned char **pixel_2 = (unsigned char**)malloc((height + 2) * sizeof(unsigned char*));
//	unsigned char **Gaussian = (unsigned char**)malloc((height + 2) * sizeof(unsigned char*));
//	unsigned char **Sobel = (unsigned char**)malloc((height + 2) * sizeof(unsigned char*));
//	double **Gtan = (double**)malloc((height + 2) * sizeof(double*));
//
//	unsigned char **NMS = (unsigned char**)malloc((height + 2) * sizeof(unsigned char*));
//	for (row = 0; row < height + 2; row++)
//	{
//		pixel_2[row] = (unsigned char*)malloc((width + 2) * sizeof(unsigned char));
//		Gaussian[row] = (unsigned char*)malloc((width + 2) * sizeof(unsigned char));
//		Sobel[row] = (unsigned char*)malloc((width + 2) * sizeof(unsigned char));
//		Gtan[row] = (double*)malloc((width + 2) * sizeof(double));
//		NMS[row] = (unsigned char*)malloc((width + 2) * sizeof(unsigned char));
//	}
//
//	for (data = 0; data < filesize; data++)
//		TempData[data] = pixel_1[data];
//
//	for (x = 0; x < height + 2; x++)
//	{
//		for (y = 0; y < width + 2; y++)
//		{
//			pixel_2[x][y] = 0;
//			Gaussian[x][y] = 0;
//			Sobel[x][y] = 0;
//			Gtan[x][y] = 0;
//			NMS[x][y] = 0;
//		}
//	}
//	//***********************************************************************Load pixel to two dimension array
//	for (x = 1; x < height + 1; x++)
//	{
//		for (y = 1; y < width + 1; y++)
//		{
//			add01 += 1;
//			pixel_2[x][y] = (float)(TempData[54 + (add01 - 1) * 3] +
//				TempData[55 + (add01 - 1) * 3] +
//				TempData[56 + (add01 - 1) * 3]) / 3 + 0.5;
//		}
//	}
//	//***********************************************************************Gaussian Smoothing
//	for (x = 1; x < height + 1; x++)
//	{
//		for (y = 1; y < width + 1; y++)
//		{
//			Gaussian[x][y] = (pixel_2[x - 1][y - 1] + 2 * pixel_2[x - 1][y] + pixel_2[x - 1][y + 1] +
//				2 * pixel_2[x][y - 1] + 4 * pixel_2[x][y] + 2 * pixel_2[x][y + 1] +
//				pixel_2[x + 1][y - 1] + 2 * pixel_2[x + 1][y] + pixel_2[x + 1][y + 1]) / 16;
//		}
//	}
//	//***********************************************************************Calculate Sobel
//	for (x = 1; x < height + 1; x++)
//	{
//		for (y = 1; y < width + 1; y++)
//		{
//			Gx = Gaussian[x - 1][y + 1] + 2 * Gaussian[x][y + 1] + Gaussian[x + 1][y + 1] -
//				(Gaussian[x - 1][y - 1] + 2 * Gaussian[x][y - 1] + Gaussian[x + 1][y - 1]);
//
//			Gy = Gaussian[x + 1][y - 1] + 2 * Gaussian[x + 1][y] + Gaussian[x + 1][y + 1] -
//				(Gaussian[x - 1][y - 1] + 2 * Gaussian[x - 1][y] + Gaussian[x - 1][y + 1]);
//
//			Sobel_ans = abs(Gx) + abs(Gy);
//			Sobel[x][y] = (Sobel_ans > 255) ? 255 : Sobel_ans;
//			NMS[x][y] = Sobel[x][y];
//
//			Gtan[x][y] = (Gx != 0) ? (double)Gy / Gx : 0;
//			/*
//			add02 += 1;
//			pixel_1[54+(add02-1)*3] = Sobel[x-1][y-1];
//			pixel_1[55+(add02-1)*3] = Sobel[x-1][y-1];
//			pixel_1[56+(add02-1)*3] = Sobel[x-1][y-1];
//			*/
//		}
//	}
//	//***********************************************************************Calculate Non-Maximum Suppression(NMS)
//	for (x = 1; x < height + 1; x++)
//	{
//		for (y = 1; y < width + 1; y++)
//		{
//			if (-0.4142 < Gtan[x - 1][y - 1] && Gtan[x - 1][y - 1] <= 0.4142)/*-22.5~22.5*/
//			{
//				if (Sobel[x][y] < Sobel[x][y + 1] || Sobel[x][y] < Sobel[x][y - 1])
//					NMS[x][y] = 0;
//			}
//
//			else if (0.4142 < Gtan[x - 1][y - 1] && Gtan[x - 1][y - 1] <= 2.4142)/*22.5~67.5*/
//			{
//				if (Sobel[x][y] < Sobel[x - 1][y + 1] || Sobel[x][y] < Sobel[x + 1][y - 1])
//					NMS[x][y] = 0;
//			}
//
//			else if (abs(Gtan[x - 1][y - 1]) > 2.4142)/*>67.5*/
//			{
//				if (Sobel[x][y] < Sobel[x - 1][y] || Sobel[x][y] < Sobel[x + 1][y])
//					NMS[x][y] = 0;
//			}
//
//			else if (-2.4142 < Gtan[x - 1][y - 1] && Gtan[x - 1][y - 1] <= -0.4142)/*-67.5~-22.5*/
//			{
//				if (Sobel[x][y] < Sobel[x - 1][y - 1] || Sobel[x][y] < Sobel[x + 1][y + 1])
//					NMS[x][y] = 0;
//			}
//			/*
//			add02 += 1;
//			pixel_1[54+(add02-1)*3] = NMS[x][y];
//			pixel_1[55+(add02-1)*3] = NMS[x][y];
//			pixel_1[56+(add02-1)*3] = NMS[x][y];
//			*/
//		}
//	}
//	//***********************************************************************Calculate Hysteresis thresholding
//	//int upper_threshold = Ostu(pixel_1, width, height, filesize);
//	//int low_threshold = upper_threshold / 2;
//
//	//int upper_threshold = 100;
//	//int low_threshold = 50;
//
//	for (x = 1; x < height + 1; x++)
//	{
//		for (y = 1; y < width + 1; y++)
//		{
//			if (NMS[x][y] >= upper_threshold)
//				NMS[x][y] = 255;
//			else if (NMS[x][y] >= low_threshold && NMS[x][y] < upper_threshold)
//				NMS[x][y] = 128;
//			else if (NMS[x][y] < low_threshold)
//				NMS[x][y] = 0;
//		}
//	}
//
//	temp = 0;
//	while (temp == 0)
//	{
//		temp = 1;
//
//		for (x = 1; x < height + 1; x++)
//		{
//			for (y = 1; y < width + 1; y++)
//			{
//				if (NMS[x][y] == 128)
//				{
//					if (NMS[x - 1][y - 1] == 255 || NMS[x - 1][y] == 255 || NMS[x - 1][y + 1] == 255 || NMS[x][y - 1] == 255 ||
//						NMS[x][y + 1] == 255 || NMS[x + 1][y - 1] == 255 || NMS[x + 1][y] == 255 || NMS[x + 1][y + 1] == 255)
//					{
//						NMS[x][y] = 255;
//						temp = 0;
//					}
//				}
//			}
//		}
//	}
//
//	fprintf(csv, "Canny");
//	for (x = 1; x < height + 1; x++)
//	{
//		for (y = 1; y < width + 1; y++)
//		{
//			NMS[x][y] = (NMS[x][y] == 128) ? 0 : NMS[x][y];
//
//			add02 += 1;
//			TempData[54 + (add02 - 1) * 3] = NMS[x][y];
//			TempData[55 + (add02 - 1) * 3] = NMS[x][y];
//			TempData[56 + (add02 - 1) * 3] = NMS[x][y];
//
//			fprintf(csv, "%d, %lf\n", add02, (float)NMS[x][y]);
//		}
//	}
//	fclose(csv);
//	//***********************************************************************Save calculating result after calculate Sobel(save as image)
//	fwrite(TempData, filesize, 1, img);
//	fclose(img);
//
//	for (row = 0; row < height + 2; row++)
//	{
//		free(pixel_2[row]);
//		free(Gaussian[row]);
//		free(Sobel[row]);
//		free(NMS[row]);
//		free(Gtan[row]);
//	}
//
//	free(TempData);
//	free(pixel_2);
//	free(Gaussian);
//	free(Sobel);
//	free(NMS);
//	free(Gtan);
//}
//
//bool Bitmap::Laplacian(Bitmap & rBitmap)
//{
//	int data, row, col, add = 0;
//
//	unsigned char *TempData = (unsigned char*)malloc(filesize * sizeof(unsigned char));
//	unsigned char **Laplace = (unsigned char**)malloc((height + 2) * sizeof(unsigned char*));
//	for (row = 0; row < height + 2; row++)
//		Laplace[row] = (unsigned char *)malloc((width + 2) * sizeof(unsigned char));
//
//	FILE *csv = fopen("Laplacian.csv", "wb");
//	FILE *img = fopen("Laplacian.bmp", "wb");
//
//	for (data = 0; data < filesize; data++)
//		TempData[data] = pixel_1[data];
//
//	for (row = 0; row < height + 2; row++)
//	{
//		for (col = 0; col < width + 2; col++)
//			Laplace[row][col] = 0;
//	}
//
//	for (row = 1; row < height + 1; row++)
//	{
//		for (col = 1; col < width + 1; col++)
//		{
//			add += 1;
//			Laplace[row][col] = (TempData[56 + (add - 1) * 3] +
//				TempData[55 + (add - 1) * 3] +
//				TempData[54 + (add - 1) * 3]) / 3;
//		}
//	}
//
//	add = 0;
//	fprintf(csv, "Laplacian");
//	for (row = 1; row < height + 1; row++)
//	{
//		for (col = 1; col < width + 1; col++)
//		{
//			float Temp = 1 * Laplace[row - 1][col - 1] + 4 * Laplace[row - 1][col] + 1 * Laplace[row - 1][col + 1] +
//				(4 * Laplace[row][col - 1] - 20 * Laplace[row][col] + 4 * Laplace[row][col + 1]) +
//				(1 * Laplace[row + 1][col - 1] + 4 * Laplace[row + 1][col] + 1 * Laplace[row + 1][col + 1]);
//
//			add += 1;
//			fprintf(csv, "%d, %lf\n", add - 1, Temp);
//			Temp = (abs(Temp) > 255) ? 255 : abs(Temp);
//
//			TempData[56 + (add - 1) * 3] = Temp;
//			TempData[55 + (add - 1) * 3] = Temp;
//			TempData[54 + (add - 1) * 3] = Temp;
//		}
//	}
//	fclose(csv);
//
//	fwrite(TempData, filesize, 1, img);
//	fclose(img);
//
//	for (row = 0; row < height + 2; row++)
//		free(Laplace[row]);
//
//	free(Laplace);
//	free(TempData);
//}
