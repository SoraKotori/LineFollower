#ifndef Arduino_h
#define __AVR_ATmega328P__
#include <avr/interrupt.h>
#include <stdbool.h>

#else
#ifndef __AVR_ATmega328P__
#error "Only works with ATmega328P"
#endif

#endif

void	USART_Initialise(void);
bool	USART_isNewData(void);
uint8_t	USART_Receive_Byte(void);
void	USART_Transmit_Byte(uint8_t Data);
void	USART_Receive(void *Buffer, uint8_t Size);
void	USART_Transmit(void *Buffer, uint8_t Size);

void	DC_Motor_Initialise(void);
void	DC_Motor_Control(uint8_t Direction, uint8_t PWM_A, uint8_t PWM_B);
void	DC_Motor_Brake(void);
void	DC_Motor_Current_Sensing(uint8_t *Current_A, uint8_t *Current_B);

bool	Servo_Motor_Initialise(uint8_t *Pins, uint16_t *Pulses, uint8_t Count);
bool	Servo_Motor_Control(uint8_t Index, uint16_t Pulse, uint16_t Speed);
void	Servo_Motor_Wait(uint8_t Index);

void	TWI_Initialise(void);
void	TWI_Write(void *Buffer, uint8_t Size);

void	MR2x30a_Speed(int16_t LeftSpeed, int16_t RightSpeed);
void	MR2x30a_Brake(void);

enum Decode
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

int main(void)
{
	uint8_t CurrentA, CurrentB;

	uint8_t ServoPin[] = { 2, 3, 4, 5, 6, 7 };
	uint16_t ServoMicroseconds[] = { 1500, 1500, 1500, 1500, 1500, 1500 };

	uint8_t ServoIndex = 0;
	uint16_t SetMicroseconds = 0;
	uint16_t SpeedMicroseconds = 0;

	int16_t MR2x30a_LeftSpeed = 0;
	int16_t MR2x30a_RightSpeed = 0;

	USART_Initialise();
	TWI_Initialise();
	DC_Motor_Initialise();
	Servo_Motor_Initialise(ServoPin, ServoMicroseconds, sizeof(ServoPin) / sizeof(uint8_t));

	while (true)
	{
		switch (USART_Receive_Byte())
		{
		case Enum_DC_Motor_Control_CW_CW:
			DC_Motor_Control(0, USART_Receive_Byte(), USART_Receive_Byte());
			break;

		case Enum_DC_Motor_Control_CW_CCW:
			DC_Motor_Control(_BV(PORTB5), USART_Receive_Byte(), USART_Receive_Byte());
			break;

		case Enum_DC_Motor_Control_CCW_CW:
			DC_Motor_Control(_BV(PORTB4), USART_Receive_Byte(), USART_Receive_Byte());
			break;

		case Enum_DC_Motor_Control_CCW_CCW:
			DC_Motor_Control(_BV(PORTB5) | _BV(PORTB4), USART_Receive_Byte(), USART_Receive_Byte());
			break;

		case Enum_DC_Motor_Brake:
			DC_Motor_Brake();
			break;

		case Enum_DC_Motor_Current_Sensing:
			DC_Motor_Current_Sensing(&CurrentA, &CurrentB);
			break;

		case Enum_ServoMicrosecond:
			USART_Receive(&ServoIndex, sizeof(ServoIndex));
			USART_Receive(&SetMicroseconds, sizeof(SetMicroseconds));
			USART_Receive(&SpeedMicroseconds, sizeof(SpeedMicroseconds));
			Servo_Motor_Control(ServoIndex, SetMicroseconds, SpeedMicroseconds);
			break;

		case Enum_ServoWait:
			USART_Receive(&ServoIndex, sizeof(ServoIndex));
			Servo_Motor_Wait(ServoIndex);
			break;

		case Enum_PinLevel:
			break;

		case Enum_MR2x30a_Speed:
			USART_Receive(&MR2x30a_LeftSpeed, sizeof(MR2x30a_LeftSpeed));
			USART_Receive(&MR2x30a_RightSpeed, sizeof(MR2x30a_RightSpeed));
			MR2x30a_Speed(MR2x30a_LeftSpeed, MR2x30a_RightSpeed);
			break;

		case Enum_MR2x30a_Brake:
			MR2x30a_Brake();
			break;
		}
	}

	return 0;
}

static struct
{
	uint8_t ReceiveBuffer[256];
	volatile uint8_t ReceiveFront;
	uint8_t ReceiveRear;
	uint8_t TransmitBuffer[256];
	uint8_t TransmitFront;
	volatile uint8_t TransmitRear;
}
USART_t;

void USART_Initialise(void)
{
	UBRR0 = 0;
	UCSR0A = _BV(U2X0);
	UCSR0B = _BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0);
	UCSR0C = _BV(UCSZ00) | _BV(UCSZ01); //Asynchronous USART | Disabled | 1-bit | 8-bit
	sei();
}

bool USART_isNewData(void)
{
	return USART_t.ReceiveFront != USART_t.ReceiveRear ? true : false;
}

ISR(USART_RX_vect)
{
	uint8_t Data = UDR0;

	USART_t.ReceiveFront++;
	if (USART_t.ReceiveFront != USART_t.ReceiveRear)
	{
		USART_t.ReceiveBuffer[USART_t.ReceiveFront] = Data;
	}
}

ISR(USART_UDRE_vect)
{
	if (USART_t.TransmitFront != USART_t.TransmitRear)
	{
		USART_t.TransmitRear++;
		UDR0 = USART_t.TransmitBuffer[USART_t.TransmitRear];
	}
	else
	{
		UCSR0B &= ~_BV(UDRIE0);
	}
}

uint8_t USART_Receive_Byte(void)
{
	UCSR0B &= ~_BV(RXCIE0);

	if (USART_t.ReceiveFront == USART_t.ReceiveRear)
	{
		while (!(UCSR0A & _BV(RXC0)));
		uint8_t Data = UDR0;
		UCSR0B |= _BV(RXCIE0);
		return Data;
	}
	else
	{
		UCSR0B |= _BV(RXCIE0);

		USART_t.ReceiveRear++;
		return USART_t.ReceiveBuffer[USART_t.ReceiveRear];
	}
}

void USART_Transmit_Byte(uint8_t Data)
{
	UCSR0B &= ~_BV(UDRIE0);

	if (USART_t.TransmitFront == USART_t.ReceiveRear && UCSR0A & _BV(UDRE0))
	{
		UDR0 = Data;
	}
	else
	{
		USART_t.TransmitFront++;
		while (USART_t.TransmitFront == USART_t.TransmitRear);

		USART_t.TransmitBuffer[USART_t.TransmitFront] = Data;
		UCSR0B |= _BV(UDRIE0);
	}
}

void USART_Transmit(void *Buffer, uint8_t Size)
{
	for (uint8_t Index = 0; Index < Size; Index++)
	{
		USART_Transmit_Byte(((uint8_t*)Buffer)[Index]);
	}
}

void USART_Receive(void *Buffer, uint8_t Size)
{
	for (uint8_t Index = 0; Index < Size; Index++)
	{
		((uint8_t*)Buffer)[Index] = USART_Receive_Byte();
	}
}

void DC_Motor_Initialise(void)
{
	DDRB |= _BV(DDB5) | _BV(DDB4) | _BV(DDB3) | _BV(DDB1) | _BV(DDB0);
	DDRD |= _BV(DDD3);
	TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
	TCCR2B = _BV(CS20);
}

void DC_Motor_Control(uint8_t Direction, uint8_t PWM_A, uint8_t PWM_B)
{
	PORTB = PORTB & ~(_BV(PORTB5) | _BV(PORTB4) | _BV(PORTB1) | _BV(PORTB0)) | Direction;
	OCR2A = PWM_B;
	OCR2B = PWM_A;
}

void DC_Motor_Brake(void)
{
	PORTB |= _BV(PORTB1) | _BV(PORTB0);
}

void DC_Motor_Current_Sensing(uint8_t *Current_A, uint8_t *Current_B)
{

}

#define CPU_HZ 16000000L

#define SERVO_CYCLE_TIME 20000
#define SERVO_UNIT 2500
#define SERVO_MAX_WIDTH 2200
#define SERVO_MIN_WIDTH 800
#define SERVO_COUNT_MAX (SERVO_CYCLE_TIME / SERVO_UNIT)
#define TICKS(Width) (CPU_HZ / 1000000L / 8 * Width)

static struct
{
	volatile uint8_t Level;
	volatile uint8_t Index;
	uint8_t Count;

	struct
	{
		volatile uint8_t *pPORTxn;
		uint8_t Bit;
		volatile uint16_t NowTick;
		uint16_t SetTick;
		uint16_t SpeedTick;
	}Pin[SERVO_COUNT_MAX];
}
Servo_t;

bool Servo_Motor_Initialise(uint8_t *Pins, uint16_t *Pulses, uint8_t Count)
{
	for (uint8_t Index = 0; Index < Count; Index++)
	{
		volatile uint8_t *pDDxn;
		volatile uint8_t *pPORTxn;
		uint8_t Bit;

		uint8_t Temp_Pin = Pins[Index];
		if (8 > Temp_Pin)
		{
			pDDxn = &DDRD;
			pPORTxn = &PORTD;
			Bit = _BV(Temp_Pin);
		}
		else if (14 > Temp_Pin)
		{
			pDDxn = &DDRB;
			pPORTxn = &PORTB;
			Bit = _BV(Temp_Pin - 8);
		}
		else if (20 > Temp_Pin)
		{
			pDDxn = &DDRC;
			pPORTxn = &PORTC;
			Bit = _BV(Temp_Pin - 14);
		}
		else // PinIndex >= 20
		{
			return false;
		}

		uint16_t PulseWidth = Pulses[Index];
		if (PulseWidth < SERVO_MIN_WIDTH || SERVO_MAX_WIDTH < PulseWidth)
		{
			return false;
		}

		*pDDxn |= Bit;
		Servo_t.Pin[Index].pPORTxn = pPORTxn;
		Servo_t.Pin[Index].Bit = Bit;
		Servo_t.Pin[Index].SetTick = TICKS(PulseWidth);
		Servo_t.Pin[Index].NowTick = TICKS(PulseWidth);
		Servo_t.Pin[Index].SpeedTick = 0;
	}

	Servo_t.Level = 0;
	Servo_t.Index = 0;
	Servo_t.Count = Count;

	TCCR1A = 0;
	TCCR1B = _BV(CS11);
	TCNT1 = 0;
	TIFR1 |= _BV(OCF1A);
	TIMSK1 |= _BV(OCIE1A);

	return true;
}

ISR(TIMER1_COMPA_vect)
{
	if (0 == Servo_t.Level) //LOW == Servo_t.Level
	{
		if (0 == Servo_t.Index)
		{
			OCR1A = 0;
			TCNT1 = 0;
		}
		*Servo_t.Pin[Servo_t.Index].pPORTxn |= Servo_t.Pin[Servo_t.Index].Bit;

		if (0 == Servo_t.Pin[Servo_t.Index].SpeedTick)
		{
			OCR1A += Servo_t.Pin[Servo_t.Index].SetTick;
		}
		else
		{
			int16_t DiffTick = Servo_t.Pin[Servo_t.Index].NowTick - Servo_t.Pin[Servo_t.Index].SetTick;
			int16_t SpeedTick = (int16_t)Servo_t.Pin[Servo_t.Index].SpeedTick;
			if (-DiffTick > SpeedTick)
			{
				Servo_t.Pin[Servo_t.Index].NowTick += Servo_t.Pin[Servo_t.Index].SpeedTick;
			}
			else if (DiffTick > SpeedTick)
			{
				Servo_t.Pin[Servo_t.Index].NowTick -= Servo_t.Pin[Servo_t.Index].SpeedTick;
			}
			else //abs(DiffTick) < SpeedTick
			{
				Servo_t.Pin[Servo_t.Index].NowTick = Servo_t.Pin[Servo_t.Index].SetTick;
			}
			OCR1A += Servo_t.Pin[Servo_t.Index].NowTick;
		}
		Servo_t.Level = 1;
	}
	else //HIGH == Servo_t.Level
	{
		*Servo_t.Pin[Servo_t.Index].pPORTxn &= ~Servo_t.Pin[Servo_t.Index].Bit;
		if (Servo_t.Count == Servo_t.Index + 1)
		{
			Servo_t.Index = Servo_t.Index + 1;
			OCR1A += TICKS(SERVO_UNIT) - Servo_t.Pin[Servo_t.Index++].NowTick;
		}
		else
		{
			Servo_t.Index = 0;
			OCR1A = TICKS(SERVO_CYCLE_TIME);
		}
		Servo_t.Level = 0;
	}
}

bool Servo_Motor_Control(uint8_t Index, uint16_t Pulse, uint16_t Speed)
{
	if (Pulse < SERVO_MIN_WIDTH || SERVO_MAX_WIDTH < Pulse)
	{
		return false;
	}

	TIMSK1 &= ~_BV(OCIE1A);
	Servo_t.Pin[Index].SetTick = TICKS(Pulse);
	Servo_t.Pin[Index].SpeedTick = TICKS(Speed);
	TIMSK1 |= _BV(OCIE1A);

	return true;
}

void Servo_Motor_Wait(uint8_t Index)
{
	if (0 == Servo_t.Pin[Index].SpeedTick)
	{
		return;
	}

	while (Servo_t.Pin[Index].SetTick != Servo_t.Pin[Index].NowTick);
}

#define MID_Address (0 << 1)

static struct
{
	uint8_t MID;
	uint8_t CID;
	uint8_t CheckSum1;
	uint8_t DutyCycleA_L;
	uint8_t DutyCycleA_H;
	uint8_t DutyCycleB_L;
	uint8_t DutyCycleB_H;
	uint8_t CheckSum2;
	uint8_t Dummy;
}
SetVelAB = { MID_Address, 118U, 137U };

static struct
{
	uint8_t MID;
	uint8_t CID;
	uint8_t CheckSum1;
	uint8_t Dummy;
}
BrakeDual = { MID_Address, 101U, 154U };

static struct
{
	volatile uint8_t *DataAddress;
	uint8_t *DataAddressEnd;
}
TWI_t;

void TWI_Initialise(void)
{
	PORTC |= _BV(PORTC4) | _BV(PORTC5);
	TWBR = 12; //SCL frequency = CPU Clock frequency / (16 + 2 * TWBR * PrescalerValue)
	TWCR = _BV(TWEN);
}

ISR(TWI_vect)
{
	switch (TWSR) //Prescaler = 0
	{
	case 0x08: //A START condition has been transmitted
	case 0x10: //A repeated START condition has been transmitted
	case 0x18: //SLA+W has been transmitted; ACK has been received
	case 0x28: //Data byte has been transmitted; ACK has been received
		if (TWI_t.DataAddress != TWI_t.DataAddressEnd)
		{
			TWDR = *TWI_t.DataAddress++;
			TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
		}
		else
		{
			TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
		}
		break;

	case 0x38: //Arbitration lost in SLA+W or data bytes
		TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN) | _BV(TWIE);
		break;

	case 0x20: //SLA+W has been transmitted; NOT ACK has been received
	case 0x30: //Data byte has been transmitted; NOT ACK has been received
	default:
		TWCR = _BV(TWEN);
		break;
	}
}

void TWI_Write(void *Buffer, uint8_t Size)
{
	while (TWCR & _BV(TWIE));

	TWI_t.DataAddress = (volatile uint8_t*)Buffer;
	TWI_t.DataAddressEnd = (uint8_t*)Buffer + Size;

	TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN) | _BV(TWIE);
}

void MR2x30a_Speed(int16_t LeftSpeed, int16_t RightSpeed)
{
	*(int16_t*)&SetVelAB.DutyCycleA_L = LeftSpeed;
	*(int16_t*)&SetVelAB.DutyCycleB_L = RightSpeed;
	SetVelAB.CheckSum2 = 255 - (SetVelAB.DutyCycleA_L + SetVelAB.DutyCycleA_H + SetVelAB.DutyCycleB_L + SetVelAB.DutyCycleB_H);

	TWI_Write(&SetVelAB, sizeof(SetVelAB));
}

void MR2x30a_Brake(void)
{
	TWI_Write(&BrakeDual, sizeof(BrakeDual));
}
