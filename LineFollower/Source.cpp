#include "ArduinoDevice.h"
#include "BitmapProcessor.h"
#include "MediaDevice.h"
#include "MotionController.h"

#include <cassert>
#include <iostream>
#include <conio.h>
#include <time.h>

#define BitmapFile "Test.bmp"
#define BitmapFileSave "TestSave.bmp"

using namespace std;

bool BitmapTest(void)
{
	Bitmap SourceBitmap;
	bool bResult = SourceBitmap.Read(BitmapFile);
	if (false == bResult)
	{
		assert(false);
	}

	Bitmap SobelBitmap;
	bResult = SobelBitmap.Read(BitmapFile);
	if (false == bResult)
	{
		assert(false);
	}

	bResult = SourceBitmap.Grayscale();
	if (false == bResult)
	{
		assert(false);
	}

	bResult = SobelBitmap.Sobel(SourceBitmap);
	if (false == bResult)
	{
		assert(false);
	}

	bResult = SobelBitmap.Write(BitmapFileSave);
	if (false == bResult)
	{
		assert(false);
	}

	return true;
}

class LineFollower
{
public:
	LineFollower();
	~LineFollower();

	bool Creat(LPCTSTR lpFileName);
	bool GetImage(bool *pNewImage);
	bool FindImageTargetCoordinate(int *pTargetX, int *pTargetY);
	bool ConversionSpeed(int TargetX, int TargetY, int *pLeftSpeed, int *pRightSpeed);
	bool SaveImage(const char* FileName);
	bool Move(int LeftSpeed, int RightSpeed);
	bool Brake();

private:
	bool Created;

	MediaDeviceSet MDevice;
	MediaReader MReader;
	Bitmap WebcamImage;
	MotionController MController;
	ArduinoDevice Arduino;
};

LineFollower::LineFollower() :
	Created(false)
{
}

LineFollower::~LineFollower()
{
	BOOL BResult = ArduinoClose(Arduino);
	if (FALSE == BResult)
	{
		assert(false);
	}

	bool bResult = MediaUninitialize();
	if (false == bResult)
	{
		assert(false);
	}
}

bool LineFollower::Creat(LPCTSTR lpFileName)
{
	bool bResult = MediaInitialize();
	if (false == bResult)
	{
		assert(false);
	}

	bResult = MDevice.CreatVideoSet();
	if (false == bResult)
	{
		assert(false);
	}

	bResult = MDevice.GetMediaReader(0, MReader);
	if (false == bResult)
	{
		assert(false);
	}

	DWORD StreamIndex = 0U;
	UINT32 Width = 0U;
	UINT32 Height = 0U;
	bResult = MReader.GetCurrentFrameSize(StreamIndex, &Width, &Height);
	if (false == bResult)
	{
		assert(false);
	}

	if (640 != Width)
	{
		assert(false);
	}

	if (480 != Height)
	{
		assert(false);
	}

	bResult = WebcamImage.Creat(Width, Height);
	if (false == bResult)
	{
		assert(false);
	}

	BOOL BResult = ArduinoOpen(&Arduino, lpFileName);
	if (FALSE == BResult)
	{
		assert(false);
	}

	Created = true;
	return true;
}

bool LineFollower::GetImage(bool *pNewImage)
{
	uint8_t *pImage = WebcamImage.Image();
	bool bResult = MReader.GetCurrenBitmapImage(0U, pImage, pNewImage);
	if (false == bResult)
	{
		assert(false);
	}

	return true;
}

bool LineFollower::FindImageTargetCoordinate(int *pTargetX, int *pTargetY)
{
	bool bResult = WebcamImage.Grayscale();
	if (false == bResult)
	{
		assert(false);
	}

	bResult = WebcamImage.FindTargetCoordinate(pTargetX, pTargetY);
	if (false == bResult)
	{
		assert(false);
	}

	return true;
}

bool LineFollower::SaveImage(const char * FileName)
{
	bool bResult = WebcamImage.Write(FileName);
	if (false == bResult)
	{
		assert(false);
	}

	return true;
}

bool LineFollower::ConversionSpeed(int TargetX, int TargetY, int *pLeftSpeed, int *pRightSpeed)
{
	bool bResult = MController.ImageTargetCoordinates(TargetX, TargetY, pLeftSpeed, pRightSpeed);
	if (false == bResult)
	{
		assert(false);
	}

	return true;
}

bool LineFollower::Move(int LeftSpeed, int RightSpeed)
{
	BOOL BResult = ArduinoMR2x30aSpeed(Arduino, LeftSpeed, RightSpeed);
	if (FALSE == BResult)
	{
		assert(false);
	}

	return true;
}

bool LineFollower::Brake()
{
	BOOL BResult = ArduinoMR2x30aBrake(Arduino);
	if (FALSE == BResult)
	{
		assert(false);
	}

	return true;
}

int main(void)
{
	LineFollower Follower;
	bool bResult = Follower.Creat(CommPortName(3));
	if (false == bResult)
	{
		assert(false);
	}

	clock_t BeginClock = clock();
	for (size_t ImageIndex = 0;; ImageIndex++)
	{
		bool NewImage = false;
		while (false == NewImage)
		{
			bResult = Follower.GetImage(&NewImage);
			if (false == bResult)
			{
				assert(false);
			}

			if (0 != _kbhit())
			{
				goto Down;
			}
		}

		int FindImageX = 0;
		int FindImageY = 0;
		bResult = Follower.FindImageTargetCoordinate(&FindImageX, &FindImageY);
		if (false == bResult)
		{
			assert(false);
		}

		if (-1 == FindImageX && -1 == FindImageY)
		{
			cout << "Not Find Target Coordinate" << endl;

			char FileName[100];
			sprintf_s<100>(FileName, "LineFollower_%04d.bmp", ImageIndex);

			bResult = Follower.SaveImage(FileName);
			if (false == bResult)
			{
				assert(false);
			}

			continue;
		}

		int LeftSpeed = 0;
		int RightSpeed = 0;
		bResult = Follower.ConversionSpeed(FindImageX, FindImageY, &LeftSpeed, &RightSpeed);
		if (false == bResult)
		{
			assert(false);
		}

		cout << "FindImageX:" << FindImageX << " FindImageY:" << FindImageY << " LeftSpeed:" << LeftSpeed << " RightSpeed:" << RightSpeed << endl;
		bResult = Follower.Move(RightSpeed, LeftSpeed);
		if (false == bResult)
		{
			assert(false);
		}
	}

Down:
	clock_t RunTime = clock() - BeginClock;
	cout << "RunTime : " << static_cast<double>(RunTime) / 1000.0 << "s" << endl;

	bResult = Follower.Move(0, 0);
	if (false == bResult)
	{
		assert(false);
	}

	system("PAUSE");
	return EXIT_SUCCESS;
}
