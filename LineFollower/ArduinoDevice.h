#pragma once
#include <Windows.h>
#include <tchar.h>

#define CommPortName(Port) _T("\\\\.\\COM"#Port)

#ifdef __cplusplus 
extern "C" {
#endif // __cplusplus 

	typedef struct ArduinoDevice
	{
		HANDLE hFile;
		OVERLAPPED Read;
		OVERLAPPED Write;
	}ArduinoDevice;

	BOOL ArduinoOpen(ArduinoDevice *Arduino, LPCTSTR lpFileName);
	BOOL ArduinoClose(ArduinoDevice Arduino);

	BOOL ArduinoRead(ArduinoDevice Arduino, void *pBuffer, int Byte);
	BOOL ArduinoWrite(ArduinoDevice Arduino, void *pBuffer, int Byte);

	BOOL ArduinoMotorSpeed(ArduinoDevice Arduino, int Left, int Right);
	BOOL ArduinoMotorBrake(ArduinoDevice Arduino);

	BOOL ArduinoMR2x30aSpeed(ArduinoDevice Arduino, int left, int right);
	BOOL ArduinoMR2x30aBrake(ArduinoDevice Arduino);

	BOOL ArduinoServoMicrosecond(ArduinoDevice Arduino, int ServoIndex, int SetMicroseconds, int SpeedMicroseconds);
	BOOL ArduinoServoWait(ArduinoDevice Arduino, int ServoIndex);

#ifdef __cplusplus 
}
#endif // __cplusplus 
