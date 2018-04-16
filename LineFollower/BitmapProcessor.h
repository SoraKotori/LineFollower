#pragma once
#include <fstream>
#include <memory>

class Bitmap
{
public:
	Bitmap();
	~Bitmap();
	explicit operator bool() const;

	int32_t Width() const;
	int32_t Height() const;
	uint32_t BitCount() const;
	uint32_t SizeImage() const;
	uint8_t* Image() const;

	bool Creat(const int32_t Width, const int32_t Height, uint16_t BitPerPixel = 24);
	bool Read(const char* FileName);
	bool Write(const char* FileName);
	bool Copy(Bitmap &rBitmap);

	bool Grayscale();
	bool FindTargetCoordinate(int *pTargetX, int *pTargetY);

	bool Sobel(Bitmap &rBitmap);
	bool Canny(Bitmap & rBitmap, char upper_threshold, char low_threshold);
	bool Laplacian(Bitmap &rBitmap);

private:
	bool Opened;
	std::streamsize FileSize;
	std::unique_ptr<char[]> FileData;
};
