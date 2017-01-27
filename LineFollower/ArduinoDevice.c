#include "ArduinoDevice.h"

#include <stdint.h>

#define fosc 16000000L
#define UBRRn 0

enum Opcode
{
	Nothing,

	Enum_DC_Motor_Initialise,
	Enum_DC_Motor_Control_CW_CW,
	Enum_DC_Motor_Control_CW_CCW,
	Enum_DC_Motor_Control_CCW_CW,
	Enum_DC_Motor_Control_CCW_CCW,
	Enum_DC_Motor_Brake,
	Enum_DC_Motor_Current_Sensing,

	Enum_ServoStart,
	Enum_ServoMicrosecond,
	Enum_ServoWait,

	Enum_PinLevel,

	Enum_MR2x30a_Start,
	Enum_MR2x30a_Speed,
	Enum_MR2x30a_Brake,
};

BOOL ArduinoOpen(ArduinoDevice *arduino, LPCTSTR lpFileName)
{
	HANDLE hFile = CreateFile
	(
		lpFileName,
		GENERIC_READ | GENERIC_WRITE,
		0UL,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL
	);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}

	DCB dcb =
	{
		.DCBlength = sizeof(DCB),
		.BaudRate = fosc / (8 * (UBRRn + 1)),
		.ByteSize = 8,
		.Parity = NOPARITY,
		.StopBits = ONESTOPBIT,
		.fDtrControl = DTR_CONTROL_DISABLE
	};

	if (FALSE == SetCommState(hFile, &dcb))
	{
		return FALSE;
	}

	HANDLE Read = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (NULL == Read)
	{
		return FALSE;
	}

	HANDLE Write = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (NULL == Write)
	{
		return FALSE;
	}

	arduino->hFile = hFile;
	arduino->Read.hEvent = Read;
	arduino->Write.hEvent = Write;
	return TRUE;
}

BOOL ArduinoClose(ArduinoDevice arduino)
{
#ifdef _MSC_VER
	DWORD dResult = WaitForSingleObject(arduino.Read.hEvent, INFINITE);
	if (WAIT_OBJECT_0 != dResult)
	{
		return FALSE;
	}

	dResult = WaitForSingleObject(arduino.Write.hEvent, INFINITE);
	if (WAIT_OBJECT_0 != dResult)
	{
		return FALSE;
	}
#endif // _MSC_VER

	BOOL bResult = CloseHandle(arduino.Read.hEvent);
	if (FALSE == bResult)
	{
		return FALSE;
	}

	bResult = CloseHandle(arduino.Write.hEvent);
	if (FALSE == bResult)
	{
		return FALSE;
	}

	bResult = CloseHandle(arduino.hFile);
	if (FALSE == bResult)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL ArduinoRead(ArduinoDevice arduino, void *pBuffer, int Byte)
{
	DWORD NumberOfBytesRead = 0UL;
	ReadFile(arduino.hFile, pBuffer, Byte, &NumberOfBytesRead, &arduino.Read);

	DWORD Error = GetLastError();
	if (ERROR_IO_PENDING != Error)
	{
		return FALSE;
	}

	DWORD Result = WaitForSingleObject(arduino.Read.hEvent, INFINITE);
	if (WAIT_OBJECT_0 != Result)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL ArduinoWrite(ArduinoDevice arduino, void *pBuffer, int Byte)
{
	DWORD Result = WaitForSingleObject(arduino.Write.hEvent, INFINITE);
	if (WAIT_OBJECT_0 != Result)
	{
		return FALSE;
	}

	DWORD NumberOfBytesWritten = 0UL;
	WriteFile(arduino.hFile, pBuffer, Byte, &NumberOfBytesWritten, &arduino.Write);

	DWORD Error = GetLastError();
	if (ERROR_IO_PENDING != Error)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL ArduinoMotorSpeed(ArduinoDevice arduino, int Left, int Right)
{
	struct
	{
		uint8_t Opcode;
		uint8_t Left;
		uint8_t Right;
	}Data = { 0 };

	if (0 < Left)
	{
		Data.Left = (uint8_t)Left;
		if (0 < Right)
		{
			Data.Right = (uint8_t)Right;
			Data.Opcode = Enum_DC_Motor_Control_CW_CW;
		}
		else // 0 >= Right
		{
			Data.Right = (uint8_t)-Right;
			Data.Opcode = Enum_DC_Motor_Control_CW_CCW;
		}
	}
	else // 0 >= Left
	{
		Data.Left = (uint8_t)-Left;
		if (0 < Right)
		{
			Data.Right = (uint8_t)Right;
			Data.Opcode = Enum_DC_Motor_Control_CCW_CW;
		}
		else // 0 >= Right
		{
			Data.Right = (uint8_t)-Right;
			Data.Opcode = Enum_DC_Motor_Control_CCW_CCW;
		}
	}

	return ArduinoWrite(arduino, &Data, sizeof(Data));
}

BOOL ArduinoMotorBrake(ArduinoDevice arduino)
{
	struct
	{
		uint8_t Opcode;
	}
	Data = { Enum_DC_Motor_Brake };

	return ArduinoWrite(arduino, &Data, sizeof(Data));
}

BOOL ArduinoMR2x30aSpeed(ArduinoDevice arduino, int left, int right)
{
#pragma pack(push)
#pragma pack(1)
	struct
	{
		uint8_t Opcode;
		int16_t Left;
		int16_t Right;
	}
	Data = { Enum_MR2x30a_Speed, (int16_t)left, (int16_t)right };
#pragma pack(pop)
	return ArduinoWrite(arduino, &Data, sizeof(Data));
}

BOOL ArduinoMR2x30aBrake(ArduinoDevice arduino)
{
	struct
	{
		uint8_t Opcode;
	}
	Data = { Enum_MR2x30a_Brake };

	return ArduinoWrite(arduino, &Data, sizeof(Data));
}

BOOL ArduinoServoMicrosecond(ArduinoDevice arduino, int ServoIndex, int SetMicroseconds, int SpeedMicroseconds)
{
#pragma pack(push)
#pragma pack(1)
	struct
	{
		uint8_t Opcode;
		uint8_t ServoIndex;
		uint16_t SetMicroseconds;
		uint16_t SpeedMicroseconds;
	}
	Data = { Enum_ServoMicrosecond, (uint8_t)ServoIndex, (uint16_t)SetMicroseconds, (uint16_t)SpeedMicroseconds };
#pragma pack(pop)
	return ArduinoWrite(arduino, &Data, sizeof(Data));
}

BOOL ArduinoServoWait(ArduinoDevice arduino, int ServoIndex)
{
#pragma pack(push)
#pragma pack(1)
	struct
	{
		uint8_t Opcode;
		uint8_t ServoIndex;
	}
	Data = { Enum_ServoWait, (uint8_t)ServoIndex };
#pragma pack(pop)
	return ArduinoWrite(arduino, &Data, sizeof(Data));
}
