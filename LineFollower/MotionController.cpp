#include "MotionController.h"

MotionController::MotionController() :
	ImageWidth(640),
	ImageHeight(480),
	MaxSpeed(1024),
	MinSpeed(-1024),
	ReferenceSpeed(1024),
	ShiftX(-320),
	ShiftY(251),
	TrackHalf(375.23)
{
	pRealTrackX = std::make_unique<double[]>(ImageWidth);
	pRealTrackY = std::make_unique<double[]>(ImageHeight);
}

MotionController::~MotionController()
{
}

void MotionController::SetImageSize(int Width, int Height)
{
	ImageWidth = Width;
	ImageHeight = Height;
}

void MotionController::SetSpeedLimit(int Max, int Min)
{
	MaxSpeed = Max;
	MinSpeed = Min;
}

bool MotionController::SetReferenceSpeed(int Speed)
{
	if (MaxSpeed < Speed || MinSpeed > Speed)
	{
		return false;
	}

	ReferenceSpeed = Speed;
	return true;
}

void MotionController::SetPhysicalDimensions(int X, int Y, double Track)
{
	ShiftX = X;
	ShiftY = Y;
	TrackHalf = Track;
}

bool MotionController::ImageTargetCoordinates(int ImageX, int ImageY, int *pLeftSpeed, int *pRightSpeed)
{
	double RealX = 0.0;
	double RealY = 0.0;
	CoordinateTransformation(ImageX, ImageY, &RealX, &RealY);

	if (0.0 == RealX)
	{
		*pLeftSpeed = ReferenceSpeed;
		*pRightSpeed = ReferenceSpeed;
	}
	else
	{
		double CenterX = 0.0;
		FindCenter(RealX, RealY, &CenterX);
		CenterConversionSpeed(CenterX, pLeftSpeed, pRightSpeed);
	}

	return true;
}

void MotionController::CoordinateTransformation(int ImageX, int ImageY, double *pRealX, double *pRealY)
{
	*pRealX = static_cast<double>(ImageX + ShiftX);
	*pRealY = static_cast<double>(ImageY + ShiftY);
}

void MotionController::FindCenter(double RealX, double RealY, double *pCenterX)
{
	*pCenterX = (pow(RealX, 2.0) + pow(RealY, 2.0)) / (2 * RealX);
}

void MotionController::CenterConversionSpeed(double CenterX, int *pLeftSpeed, int *pRightSpeed)
{
	double RatioSub = CenterX - TrackHalf;
	double RatioAdd = CenterX + TrackHalf;

	if (0.0 < CenterX)
	{
		*pLeftSpeed = ReferenceSpeed;
		*pRightSpeed = 0.0 > RatioSub ? 0 : static_cast<int>(ReferenceSpeed * RatioSub / RatioAdd);
	}
	else
	{
		*pRightSpeed = ReferenceSpeed;
		*pLeftSpeed = 0.0 < RatioAdd ? 0 : static_cast<int>(ReferenceSpeed * RatioAdd / RatioSub);
	}
}
