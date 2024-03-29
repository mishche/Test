﻿#include "stm32f4xx.h" 

#include "math.h"

#include <stdlib.h>

#include <time.h>








void SetAltFunc(GPIO_TypeDef* Port, int Channel, int AF)

{

	Port->MODER &= ~(3 << (2 * Channel)); // ????? ??????

	Port->MODER |= 2 << (2 * Channel); // ????????? ????. ??????

	if (Channel < 8) // ????? ???????? ??????? ?? ?????? ????????

	{

		Port->AFR[0] &= ~(15 << 4 * Channel); // ????? ????. ???????

		Port->AFR[0] |= AF << (4 * Channel); // ????????? ????. ???????

	}

	else

	{

		Port->AFR[1] &= ~(15 << 4 * (Channel - 8)); // ????? ????. ???????

		Port->AFR[1] |= AF << (4 * (Channel - 8)); // ????????? ????. ???????

	}

}






class I2C_work
{
	
	
public:
  
void I2C2_Write(int Address, char Reg, char* Data, int Size) // ??????? ? ??????

{

	I2C2->CR1 |= I2C_CR1_START; // ???????? ????? ????? ??? ??????

	while (!(I2C2->SR1 & I2C_SR1_SB)) {} // ???? ??????? ?????

	I2C2->DR = (Address << 1) & ~I2C_OAR1_ADD0; // ???? ????? ??? ???????? ??????

	while (!(I2C2->SR1 & I2C_SR1_ADDR)) {} // ???? ???????? ????? ? ???????????

	I2C2->SR2; // ?????? SR2 ??? ??? ????????

	while (!(I2C2->SR1 & I2C_SR1_TXE)) {} // ???? ?????????? ???????? ????

	I2C2->DR = Reg; // ???????? ???? ????????

	while (Size--) // ???? ???????? ????

	{

		while (!(I2C2->SR1 & I2C_SR1_TXE)) {} // ???? ?????????? ???????? ????

		I2C2->DR = *Data++; // ???????? ???? ??????

	}

	while (!(I2C2->SR1 & I2C_SR1_BTF)) {} // ???? ????????? ???????? ??????

	I2C2->CR1 |= I2C_CR1_STOP; // ??????????? ????? ?????

	while (I2C2->CR1 & I2C_CR1_STOP) {} // ???? ???????????? ?????

}
	
	
	
void I2C3_Read(int Address, char* Data, int Size) // ??????? ??????

{

	if (Size > 1) I2C3->CR1 |= I2C_CR1_ACK; // ???? ?????? 1 ???? ?? ????? ACK

	else I2C3->CR1 &= ~I2C_CR1_ACK; // ???? ?????? 2 ???? ?? ACK ?? ?????

	I2C3->CR1 |= I2C_CR1_START; // ???????? ????? ????? ??? ??????

	while (!(I2C3->SR1 & I2C_SR1_SB)) {} // ???? ??????? ?????

	I2C3->DR = (Address << 1) | I2C_OAR1_ADD0; // ???? ????? ??? ?????? ??????

	while (!(I2C3->SR1 & I2C_SR1_ADDR)) {} // ???? ???????? ????? ? ???????????

	I2C3->SR2; // ?????? SR2 ??? ??? ????????

	while (Size--) // ???? ?????? ????

	{

		while (!(I2C3->SR1 & I2C_SR1_RXNE)) {} // ???? ??????? ????? ??? ??????

		if (Size == 1) I2C3->CR1 &= ~I2C_CR1_ACK; // ???? ??????? 1 ????, ?? ACK ??????

		*Data++ = I2C3->DR; // ?????? ????????? ????

	}

	I2C3->CR1 |= I2C_CR1_STOP; // ??????????? ????? ?????

	while (I2C3->CR1 & I2C_CR1_STOP) {} // ???? ???????????? ?????

}


	
	void I2C3_Write(int Address, char* Data, int Size) // ??????? ????????

{

	I2C3->CR1 |= I2C_CR1_START; // ???????? ????? ????? ??? ??????

	while (!(I2C3->SR1 & I2C_SR1_SB)) {} // ???? ??????? ?????

	I2C3->DR = (Address << 1) & ~I2C_OAR1_ADD0; // ???? ????? ??? ???????? ??????

	while (!(I2C3->SR1 & I2C_SR1_ADDR)) {} // ???? ???????? ????? ? ???????????

	I2C3->SR2; // ?????? SR2 ??? ??? ????????

	while (Size--) // ???? ???????? ????

	{

		while (!(I2C3->SR1 & I2C_SR1_TXE)) {} // ???? ?????????? ???????? ????

		I2C3->DR = *Data++; // ???????? ????

	}

	while (!(I2C3->SR1 & I2C_SR1_BTF)) {} // ???? ????????? ???????? ??????

	I2C3->CR1 |= I2C_CR1_STOP; // ??????????? ????? ?????

	while (I2C3->CR1 & I2C_CR1_STOP) {} // ???? ???????????? ?????

}


void InitI2C3()
{
RCC->APB1ENR |= RCC_APB1ENR_I2C3EN; // I2C 3 ????????????

RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN; // ???? H ????????????

GPIOH->OTYPER |= GPIO_OTYPER_OT_7 | GPIO_OTYPER_OT_8; // ???????? ???? ??? PH7 ? PH8

GPIOH->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR7_0 | GPIO_OSPEEDER_OSPEEDR7_1; // ???. ???????? PH7

GPIOH->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR8_0 | GPIO_OSPEEDER_OSPEEDR8_1; // ???. ???????? PH8

GPIOH->PUPDR |= GPIO_PUPDR_PUPDR7_0 | GPIO_PUPDR_PUPDR8_0; // ???????? 3.3V ??? PH7 ? PH8

SetAltFunc(GPIOH, 7, 4); // ????????? ????. ?????? AF4 ??? SCL(PH7) (??. ???. 2)

SetAltFunc(GPIOH, 8, 4); // ????????? ????. ?????? AF4 ??? SDA(PH8) (??. ???. 2)

I2C3->CR2 = (I2C_CR2_FREQ & 0x2A);

I2C3->CCR = I2C_CCR_FS | (I2C_CCR_CCR & 0x006C);

I2C3->TRISE = (I2C_TRISE_TRISE & 0x14);

I2C3->CR1 = I2C_CR1_PE;

while (I2C3->SR2 & I2C_SR2_BUSY) {}
}


void InitI2C2()

{
	RCC->APB1ENR |= RCC_APB1ENR_I2C2EN; // I2C 2 ????????????

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN; // ???? H ????????????

	GPIOH->OTYPER |= GPIO_OTYPER_OT_4 | GPIO_OTYPER_OT_5; // ???????? ???? ??? PH7 ? PH8

	GPIOH->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR4_0 | GPIO_OSPEEDER_OSPEEDR4_1; // ???. ???????? PH7

	GPIOH->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR5_0 | GPIO_OSPEEDER_OSPEEDR5_1; // ???. ???????? PH8

	GPIOH->PUPDR |= GPIO_PUPDR_PUPDR4_0 | GPIO_PUPDR_PUPDR5_0; // ???????? 3.3V ??? PH7 ? PH8

	SetAltFunc(GPIOH, 4, 4); // ????????? ????. ?????? AF4 ??? SCL(PH7) (??. ???. 2)

	SetAltFunc(GPIOH, 5, 4); // ????????? ????. ?????? AF4 ??? SDA(PH8) (??. ???. 2)



	I2C2->CR2 = (I2C_CR2_FREQ & 0x2A);

	I2C2->CCR = I2C_CCR_FS | (I2C_CCR_CCR & 0x006C);

	I2C2->TRISE = (I2C_TRISE_TRISE & 0x14);



	I2C2->CR1 = I2C_CR1_PE;



	while (I2C2->SR2 & I2C_SR2_BUSY) {}





}

};

	const int Address = 0x68; // ????? ??????????

class OLED
{

  
  I2C_work work;
  
  public:
	const int Addr = 0x3C; // ?????????? ?????????? ? ??????? ??????????

	void Command(char Value) { work.I2C2_Write(Addr, 0x00, &Value, 1); } // ?????? ? 0x00 ???????



	static const int Width = 128; // ?????????? ?????????? ? ??????? ??????

	static const int Height = 64; // ?????????? ?????????? ? ??????? ??????

	char Buffer[Width * Height / 8]; // ????? ?????? ??????











	void Clear() // ??????? ??????? ?????? ??????

	{

		for (int a = 0; a < sizeof(Buffer); a++) Buffer[a] = 0;

	}





	void DrawPixel(int X, int Y) // ??????? ????????? ???????

	{

		if (X >= Width || Y >= Height) return;

		Buffer[X + (Y / 8) * Width] |= 1 << (Y % 8);

	}





	void UpdateScreen() // ??????? ?????????? ?????? ?????? ?? ???????

	{

		for (char a = 0; a < 8; a++)

		{

			Command(0xB0 + a);

			Command(0x00);

			Command(0x10);

			 work.I2C2_Write(Addr, 0x40, &Buffer[Width * a], Width);

		}

	}





	void DrawCirlce(int X0, int Y0, int R)

	{

		int x = 0;

		int y = R;

		int delta = 1 - 2 * R;

		int error = 0;

		while (y >= 0)

		{

			DrawPixel(X0 + x, Y0 + y);

			DrawPixel(X0 + x, Y0 - y);

			DrawPixel(X0 - x, Y0 + y);

			DrawPixel(X0 - x, Y0 - y);

			error = 2 * (delta + y) - 1;


			if (delta < 0 && error <= 0)

			{

				++x;

				delta += 2 * x + 1;

				continue;

			}

			error = 2 * (delta - x) - 1;

			if (delta > 0 && error > 0)

			{

				--y;

				delta += 1 - 2 * y;

				continue;

			}

			++x;

			delta += 2 * (x - y);

			--y;

		}

	}




	void InitOLED()

	{
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN; // ???? E ????????????

		GPIOE->MODER &= ~GPIO_MODER_MODER7; // ????? ?????? ??? PE7

		GPIOE->MODER |= GPIO_MODER_MODER7_0; // ????????? ?????? ?? ????? PE7

		GPIOE->BSRRL = 1 << 7; // ?????????? ???????? HIGH (3.3V) ??? PE7



		for (int a = 0; a < 10000000; a++) asm volatile("nop"); // ????????? ?????????? ??????????

		Command(0xAE); // Display off

		Command(0x20); // Set Memory Addressing Mode

		Command(0x10); // Page Addressing Mode (RESET)

		Command(0xB0); // Set Page Start Address for Page Addressing Mode,0-7

		Command(0xC8); // Set COM Output Scan Direction

		Command(0x00); //---set low column address

		Command(0x10); //---set high column address

		Command(0x40); //--set start line address

		Command(0x81); //--set contrast control register

		Command(0xFF); // Orientation

		Command(0xA1); //--set segment re-map (0 to 127)

		Command(0xA6); //--set normal display

		Command(0xA8); //--set multiplex ratio (1 to 64)

		Command(0x3F); //

		Command(0xA4); // Output follows RAM content

		Command(0xD3); //-set display offset

		Command(0x00); //-not offset

		Command(0xD5); //--set display clock divide ratio/oscillator frequency

		Command(0xF0); //--set divide ratio

		Command(0xD9); //--set pre-charge period

		Command(0x22); //

		Command(0xDA); //--set com pins hardware configuration

		Command(0x12); //

		Command(0xDB); //--set vcomh

		Command(0x20); // 0.77 * Vcc

		Command(0x8D); //--set DC-DC enable

		Command(0x14); //

		Command(0xAF); //--turn on SSD1309 panel

	}





};















void InitMPU()
{


        I2C_work work;
	int timer = 1000000; while (timer--) {} // ???????? ?????????? ?????????? ? ??????

	char reg[2];

	reg[0] = 0x06; reg[1] = 0x80; // ????? ??????????

	 work.I2C3_Write(Address, reg, 2);

	timer = 1000000; while (timer--) {} // ???????? ?????????? ?????????? ? ??????

	reg[0] = 0x06; reg[1] = 0x01; // ???????? ?????????? ? ??????? ????????????

	 work.I2C3_Write(Address, reg, 2);

	timer = 1000000; while (timer--) {} // ???????? ?????????? ?????????? ? ??????

}




float AccX = 0;

float AccY = 0;


short AccXt = 0;

short AccYt = 0;


int CBX = 0;

int CBY = 0;

int CBR = 5;

int CSX = 0;

int CSY = 0;

int CSR = 2;

bool boom = false;





int main()

{


        I2C_work W;
        OLED O;
          
	W.InitI2C3();
	W.InitI2C2();
	O.InitOLED();
	InitMPU();


	char data[6];





	srand(time(NULL));



	CBX = 15 + rand() % (110 - 16);

	CBY = 15 + rand() % (45 - 16);



	int GoX = 15 + rand() % (110 - 16);

	int GoY = 15 + rand() % (45 - 16);

	int i = 0;

	int j = 0;






	while (1)

	{



		data[0] = 0x2D; // ??????? ????????????

		W.I2C3_Write(Address, data, 1);

		W.I2C3_Read(Address, data, 6);
                
                AccXt = (((short)data[0]) << 8) + data[1];
                
                AccX += ((float)AccXt)/5.0;

		if (AccX > 7000)
		{
			AccX = 7000;
		}
		else if (AccX < -7000)
		{
			AccX = -7000;
		}

		// ????????? ????????????? ??? X
                
                AccYt = (((short)data[2]) << 8) + data[3];
                
                AccY += ((float)AccYt)/5.0;
                  
		if (AccY > 3000) // ????????? ????????????? ??? Y
		{
			AccY = 3000;
		}
		else if (AccY < -3000)
		{
			AccY = -3000;
		}



		if (boom)

		{



			CBX = 15 + rand() % (110 - 16);

			CBY = 15 + rand() % (45 - 16);

			GoX = 15 + rand() % (110 - 16);

			GoY = 15 + rand() % (45 - 16);

			boom = false;

			i = 0;

			j = 0;



		}



		if (CBX + i < GoX)

		{
			i++;
		}

		else

		{

			if (CBX + i != GoX)

			{

				i--;

			}

			else

			{

				GoX = 15 + rand() % (110 - 16);




			}



		}



		if (CBY + j < GoY)

		{
			j++;
		}

		else

		{

			if (CBY + j != GoY)

			{

				j--;

			}

			else

			{

				GoY = 15 + rand() % (45 - 16);





			}

		}





		CSX = 64 - (AccX / 127);

		CSY = 32 - (AccY / 127);





		O.Clear(); // ??????? ????? ??????



		O.DrawCirlce(CSX, CSY, CSR); // ??????? ???? ?? ?????? ? ???????? 15 ????????

		if (((sqrt(pow((CBX + i - CSX), 2) + pow((CBY + j - CSY), 2)) > CBR + CSR) && !boom))

		{



			O.DrawCirlce(CBX + i, CBY + j, CBR); // ??????? ???? ?? ?????? ? ???????? 15 ????????

		}

		else

		{



			boom = true;

		}



		O.UpdateScreen(); // ??????? ????? ?????? ?? ???????





	}

















}
