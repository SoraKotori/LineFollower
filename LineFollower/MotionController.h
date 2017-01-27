#pragma once
#include <memory>

class MotionController
{
public:
	MotionController();
	~MotionController();

	void SetImageSize(int Width, int Height);
	void SetSpeedLimit(int Max, int Min);
	bool SetReferenceSpeed(int Speed);
	void SetPhysicalDimensions(int X, int Y, double Track);
	bool ImageTargetCoordinates(int ImageX, int ImageY, int *pLeftSpeed, int *pRightSpeed);

private:
	int ImageWidth;
	int ImageHeight;

	int MaxSpeed;
	int MinSpeed;
	int ReferenceSpeed;

	int ShiftX;
	int ShiftY;
	double TrackHalf;

	std::unique_ptr<double[]> pRealTrackX;
	std::unique_ptr<double[]> pRealTrackY;

	void CoordinateTransformation(int ImageX, int ImageY, double *pRealX, double *pRealY);
	void FindCenter(double RealX, double RealY, double *pCenterX);
	void CenterConversionSpeed(double CenterX, int *pLeftSpeed, int *pRightSpeed);
}; 
