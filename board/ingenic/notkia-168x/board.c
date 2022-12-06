/*
 * Notkia 168x board setup code
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 *
 * Copyright (c) 2022 SudoMaker
 * Author: Reimu NotMoe <reimu@sudomaker.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/arch/cpm.h>
#include <asm/arch/clk.h>
#include <asm/arch/mmc.h>

#define BIT(x)          (1 << (x))

#define BIT_GET(x, n)       (((x) >> (n)) & 1)
#define BIT_SET(x, n)       ((x) |= BIT(n))
#define BIT_CLR(x, n)       ((x) &= ~BIT(n))

#define XHAL_WAIT_OK        0
#define XHAL_WAIT_ERROR     -1

typedef int XHAL_Wait_Callback(void *hal_handle);

typedef struct {
	volatile const uint32_t INL;
	volatile const uint32_t _rsvd0[3];
	volatile uint32_t INT;
	volatile uint32_t INTS;
	volatile uint32_t INTC;
	volatile const uint32_t _rsvd1[1];
	volatile uint32_t MSK;
	volatile uint32_t MSKS;
	volatile uint32_t MSKC;
	volatile const uint32_t _rsvd2[1];
	volatile uint32_t PAT1;
	volatile uint32_t PAT1S;
	volatile uint32_t PAT1C;
	volatile const uint32_t _rsvd3[1];
	volatile uint32_t PAT0;
	volatile uint32_t PAT0S;
	volatile uint32_t PAT0C;
	volatile const uint32_t _rsvd4[1];
	volatile const uint32_t FLG;
	volatile const uint32_t _rsvd5[1];
	volatile const uint32_t FLGC;
	volatile const uint32_t _rsvd6[5];
	volatile uint32_t PEN;
	volatile uint32_t PENS;
	volatile uint32_t PENC;
	volatile const uint32_t _rsvd7[29];
	volatile uint32_t GID2LD;
} XHAL_GPIO_TypeDef;

#define XHAL_PHYSADDR_GPIO          0x10010000
#define XHAL_GPIO_PORT_WIDTH            0x100

#define XHAL_GPIO_GetINL(port_addr, pin_num)        BIT_GET(((volatile XHAL_GPIO_TypeDef *)(port_addr))->INL, pin_num)

#define XHAL_GPIO_GetINT(port_addr, pin_num)        BIT_GET(((volatile XHAL_GPIO_TypeDef *)(port_addr))->INT, pin_num)
#define XHAL_GPIO_SetINT(port_addr, pin_num, value) if (value) ((volatile XHAL_GPIO_TypeDef *)(port_addr))->INTS = BIT(pin_num); else \
							((volatile XHAL_GPIO_TypeDef *)(port_addr))->INTC = BIT(pin_num)

#define XHAL_GPIO_GetMSK(port_addr, pin_num)        BIT_GET(((volatile XHAL_GPIO_TypeDef *)(port_addr))->MSK, pin_num)
#define XHAL_GPIO_SetMSK(port_addr, pin_num, value) if (value) ((volatile XHAL_GPIO_TypeDef *)(port_addr))->MSKS = BIT(pin_num); else \
							((volatile XHAL_GPIO_TypeDef *)(port_addr))->MSKC = BIT(pin_num)

#define XHAL_GPIO_GetPAT0(port_addr, pin_num)       BIT_GET(((volatile XHAL_GPIO_TypeDef *)(port_addr))->PAT0, pin_num)
#define XHAL_GPIO_SetPAT0(port_addr, pin_num, value)    if (value) ((volatile XHAL_GPIO_TypeDef *)(port_addr))->PAT0S = BIT(pin_num); else \
							((volatile XHAL_GPIO_TypeDef *)(port_addr))->PAT0C = BIT(pin_num)

#define XHAL_GPIO_GetPAT1(port_addr, pin_num)       BIT_GET(((volatile XHAL_GPIO_TypeDef *)(port_addr))->PAT1, pin_num)
#define XHAL_GPIO_SetPAT1(port_addr, pin_num, value)    if (value) ((volatile XHAL_GPIO_TypeDef *)(port_addr))->PAT1S = BIT(pin_num); else \
							((volatile XHAL_GPIO_TypeDef *)(port_addr))->PAT1C = BIT(pin_num)

#define XHAL_GPIO_GetFLG(port_addr, pin_num)        BIT_GET(((volatile XHAL_GPIO_TypeDef *)(port_addr))->FLG, pin_num)
#define XHAL_GPIO_ClearFLG(port_addr, pin_num)      ((volatile XHAL_GPIO_TypeDef *)(port_addr))->FLGC = BIT(pin_num)

#define XHAL_GPIO_GetPEN(port_addr, pin_num)        BIT_GET(((volatile XHAL_GPIO_TypeDef *)(port_addr))->PEN, pin_num)
#define XHAL_GPIO_SetPEN(port_addr, pin_num)        if (value) ((volatile XHAL_GPIO_TypeDef *)(port_addr))->PENS = BIT(pin_num); else \
							((volatile XHAL_GPIO_TypeDef *)(port_addr))->PENC = BIT(pin_num)

#define XHAL_GPIO_TogglePin(port_addr, pin_num)     if (XHAL_GPIO_GetPAT0(port_addr, pin_num)) \
							((volatile XHAL_GPIO_TypeDef *)(port_addr))->PAT0C = BIT(pin_num); else                                         \
							((volatile XHAL_GPIO_TypeDef *)(port_addr))->PAT0S = BIT(pin_num)

#define XHAL_GPIO_WritePin              XHAL_GPIO_SetPAT0
#define XHAL_GPIO_ReadPin               XHAL_GPIO_GetINL

void XHAL_GPIO_SetAsFunction(volatile XHAL_GPIO_TypeDef *xgpio, uint8_t pin_number, uint8_t function) {
	XHAL_GPIO_SetINT(xgpio, pin_number, 0);
	XHAL_GPIO_SetMSK(xgpio, pin_number, 0);
	XHAL_GPIO_SetPAT0(xgpio, pin_number, BIT_GET(function, 0));
	XHAL_GPIO_SetPAT1(xgpio, pin_number, BIT_GET(function, 1));
}

void XHAL_GPIO_SetAsGPIO(volatile XHAL_GPIO_TypeDef *xgpio, uint8_t pin_number, uint8_t is_input) {
	XHAL_GPIO_SetINT(xgpio, pin_number, 0);
	XHAL_GPIO_SetMSK(xgpio, pin_number, 1);
	XHAL_GPIO_SetPAT1(xgpio, pin_number, is_input);
}

#define XHAL_REG_SPI_CR0_TENDIAN_LSB          BIT(19)
#define XHAL_REG_SPI_CR0_RENDIAN_LSB          BIT(17)
#define XHAL_REG_SPI_CR0_SSIE                 BIT(15)
#define XHAL_REG_SPI_CR0_LOOP                 BIT(10)
#define XHAL_REG_SPI_CR0_EACLRUN              BIT(7)
#define XHAL_REG_SPI_CR0_FSEL                 BIT(6)
#define XHAL_REG_SPI_CR0_TFLUSH               BIT(2)
#define XHAL_REG_SPI_CR0_RFLUSH               BIT(1)
#define XHAL_REG_SPI_CR0_DISREV               BIT(0)

#define XHAL_REG_SPI_CR1_FRMHL_MASK           (BIT(31) | BIT(30))
#define XHAL_REG_SPI_CR1_FRMHL                BIT(30)
#define XHAL_REG_SPI_CR1_LFST                 BIT(25)
#define XHAL_REG_SPI_CR1_UNFIN                BIT(23)
#define XHAL_REG_SPI_CR1_PHA                  BIT(1)
#define XHAL_REG_SPI_CR1_POL                  BIT(0)

#define XHAL_REG_SPI_SR_END                   BIT(7)
#define XHAL_REG_SPI_SR_BUSY                  BIT(6)
#define XHAL_REG_SPI_SR_TFF                   BIT(5)
#define XHAL_REG_SPI_SR_RFE                   BIT(4)
#define XHAL_REG_SPI_SR_TFHE                  BIT(3)
#define XHAL_REG_SPI_SR_RFHF                  BIT(2)
#define XHAL_REG_SPI_SR_UNDR                  BIT(1)
#define XHAL_REG_SPI_SR_OVER                  BIT(0)

typedef struct {
	volatile uint32_t DR;               /*!< SSI Data Register */
	volatile union {
		volatile uint32_t CR0;          /*!< SSI Control Register 0 */
		volatile struct {
			uint32_t DISREV : 1;        /*!< Receive disable */
			uint32_t RFLUSH : 1;        /*!< Flush RX FIFO */
			uint32_t TFLUSH : 1;        /*!< Flush TX FIFO */
			uint32_t TFMODE : 1;        /*!< FIFO drain mode */
			uint32_t VRCNT : 1;     /*!< Enable RCNT */
			uint32_t : 1;           /*!< Reserved */
			uint32_t FSEL : 1;      /*!< CE selection */
			uint32_t EACLRUN : 1;       /*!< Automatically clear TX FIFO underrun flag */
			uint32_t RFINC : 1;     /*!<  */
			uint32_t RFINE : 1;     /*!<  */
			uint32_t LOOP : 1;      /*!< Enable loopback mode */
			uint32_t REIE : 1;      /*!<  */
			uint32_t TEIE : 1;      /*!<  */
			uint32_t RIE : 1;       /*!<  */
			uint32_t TIE : 1;       /*!<  */
			uint32_t SSIE : 1;      /*!< SSI module enable */
			uint32_t RENDIAN : 2;       /*!< RX endian */
			uint32_t TENDIAN : 2;       /*!< TX endian */
			uint32_t : 12;          /*!< Reserved */
		} CR0bits;
	};
	volatile union {
		volatile uint32_t CR1;          /*!< SSI Control Register1 */
		volatile struct {
			uint32_t POL : 1;       /*!< SPI CPOL value */
			uint32_t PHA : 1;       /*!< SPI CPHA value */
			uint32_t : 1;           /*!< Reserved */
			uint32_t FLEN : 5;      /*!< Frame length & FIFO entry size (2~33) */
			uint32_t RTRG : 4;      /*!< RX FIFO half-full threshold (multiply with 8) */
			uint32_t MCOM : 4;      /*!< Length of command in Microwire format */
			uint32_t TTRG : 4;      /*!< TX FIFO half-full threshold (multiply with 8) */
			uint32_t FMAT : 2;      /*!< Transfer format */
			uint32_t : 1;           /*!< Reserved */
			uint32_t UNFIN : 1;     /*!< FIFO xrun behavior */
			uint32_t ITFRM : 1;     /*!< Assert CEs during interval time */
			uint32_t : 1;           /*!< Reserved */
			uint32_t TCKFI : 2;     /*!< Time from clock stop to frame invalid */
			uint32_t TFVCK : 2;     /*!< Time from frame valid to clock start */
			uint32_t FRMHL : 2;     /*!< Frame valid level select */
		} CR1bits;
	};
	volatile union {
		volatile uint32_t SR;           /*!< SSI Status Register */
		volatile struct {
			uint32_t OVER : 1;      /*!< RX FIFO overrun */
			uint32_t UNDR : 1;      /*!< TX FIFO underrun */
			uint32_t RFHF : 1;      /*!< RX FIFO half full */
			uint32_t TFHE : 1;      /*!< TX FIFO half empty */
			uint32_t RFE : 1;       /*!< RX FIFO empty */
			uint32_t TFF : 1;       /*!< TX FIFO full */
			uint32_t BUSY : 1;      /*!< TX/RX in process */
			uint32_t END : 1;       /*!< (Interval) Transfer finished */
			uint32_t RFIFO_NUM : 8;     /*!< RX FIFO usage */
			uint32_t TFIFO_NUM : 8;     /*!< TX FIFO usage */
			uint32_t : 8;           /*!< Reserved */
		} SRbits;
	};
	volatile uint32_t ITR;      /*!< SSI Interval Time Control Register */
	volatile uint32_t ICR;      /*!< SSI Interval Character-per-frame Control Register */
	volatile uint32_t GR;       /*!< SSI Clock Generator Register */
	volatile uint32_t RCNT;     /*!< SSI Receive Counter Register */
} XHAL_SPI_TypeDef;

typedef struct {
	uint8_t spi_mode;
	uint8_t brg;
} XHAL_SPI_InitTypeDef;

typedef struct {
	volatile XHAL_SPI_TypeDef *periph;
	XHAL_SPI_InitTypeDef init;
	XHAL_Wait_Callback *wait_callback;
	void *user_data;
} XHAL_SPI_HandleTypeDef;

#define XHAL_PHYSADDR_SPI       0x10043000
#define XHAL_SPI_FIFO_SIZE      128

int XHAL_SPI_Init(XHAL_SPI_HandleTypeDef *xhspi) {
	volatile XHAL_SPI_InitTypeDef *init = &xhspi->init;
	volatile XHAL_SPI_TypeDef *periph = xhspi->periph;

	periph->GR = init->brg;
	periph->CR0 = XHAL_REG_SPI_CR0_EACLRUN;
	periph->CR1 = 0;
	periph->SR = 0;
	periph->CR0 |= XHAL_REG_SPI_CR0_SSIE;
	periph->CR1 = 0x87830;
	periph->CR0 |= XHAL_REG_SPI_CR0_RFLUSH | XHAL_REG_SPI_CR0_TFLUSH;

	printf("SSICR0: %08lx\n", periph->CR0);

	return 0;
}

int XHAL_SPI_CalculateBRG(uint32_t parent_clk, uint32_t desired_sclk, uint32_t *real_sclk) {
	// TODO

	return 0;
}

int XHAL_SPI_Transmit(XHAL_SPI_HandleTypeDef *xhspi, const uint8_t *buf, unsigned int len) {
	volatile XHAL_SPI_TypeDef *periph = xhspi->periph;

	const unsigned int fifo_half_len = XHAL_SPI_FIFO_SIZE / 2;
	unsigned int fifo_available = XHAL_SPI_FIFO_SIZE;

	periph->CR1bits.TTRG = fifo_half_len / 8;
	periph->CR0bits.DISREV = 1;

	uint8_t transfer_mode = 0;

	// Try to speed up when there are large amount of data
	// if (len >= fifo_half_len) {
	// 	if ((((uintptr_t) buf & 0b11) == 0) && (len % 4 == 0)) { // 32bit aligned
	// 		transfer_mode = 2;
	// 	} else if ((((uintptr_t) buf & 0b1) == 0) && (len % 2 == 0)) { // 16bit aligned
	// 		transfer_mode = 1;
	// 	}
	// }

	if (transfer_mode == 2) { // 32bit accelerated mode
		periph->CR1bits.FLEN = 32 - 2;
		periph->CR0bits.TENDIAN = 3;

		volatile uint32_t *dr32 = (volatile uint32_t *) &periph->DR;
		uint32_t *buf32 = (uint32_t *) buf;

		unsigned int i;

		for (i = 0; i < len / 4; i++) {
			while (!fifo_available) {
				if (periph->SR & XHAL_REG_SPI_SR_TFHE) {
					fifo_available = fifo_half_len;
				} else {
					if (xhspi->wait_callback) {
						int wrc = xhspi->wait_callback(xhspi);
						if (wrc != XHAL_WAIT_OK) {
							return wrc;
						}
					}

				}
			}

			*dr32 = buf32[i];
			fifo_available--;
		}
	} else if (transfer_mode == 1) { // 16bit accelerated mode
		periph->CR1bits.FLEN = 16 - 2;
		periph->CR0bits.TENDIAN = 3;

		volatile uint16_t *dr16 = (volatile uint16_t *) &periph->DR;
		uint16_t *buf16 = (uint16_t *) buf;

		unsigned int i;

		for (i = 0; i < len / 2; i++) {
			while (!fifo_available) {
				if (periph->SR & XHAL_REG_SPI_SR_TFHE) {
					fifo_available = fifo_half_len;
				} else {
					if (xhspi->wait_callback) {
						int wrc = xhspi->wait_callback(xhspi);
						if (wrc != XHAL_WAIT_OK) {
							return wrc;
						}
					}

				}
			}

			*dr16 = buf16[i];
			fifo_available--;
		}

	} else { // 8bit mode
		periph->CR1bits.FLEN = 8 - 2;
		periph->CR0bits.TENDIAN = 0;

		volatile uint8_t *dr = (volatile uint8_t *) &periph->DR;
		unsigned int i;
		for (i = 0; i < len; i++) {
			while (!fifo_available) {
				if (periph->SR & XHAL_REG_SPI_SR_TFHE) {
					fifo_available = fifo_half_len;
				} else {
					if (xhspi->wait_callback) {
						int wrc = xhspi->wait_callback(xhspi);
						if (wrc != XHAL_WAIT_OK) {
							return wrc;
						}
					}

				}
			}

			*dr = buf[i];
			fifo_available--;
		}
	}

	while (periph->SR & XHAL_REG_SPI_SR_BUSY) {
		if (xhspi->wait_callback) {
			int wrc = xhspi->wait_callback(xhspi);
			if (wrc != XHAL_WAIT_OK) {
				return wrc;
			}
		}
	}

	return 0;
}

#define USING_240X320

#define ST7789_ROTATION 0


#define ST7789_WIDTH 240
#define ST7789_HEIGHT 320

#if ST7789_ROTATION == 0
	#define X_SHIFT 0
	#define Y_SHIFT 0
#elif ST7789_ROTATION == 1
	#define X_SHIFT 80
	#define Y_SHIFT 0
#elif ST7789_ROTATION == 2
	#define X_SHIFT 0
	#define Y_SHIFT 0
#elif ST7789_ROTATION == 3
	#define X_SHIFT 0
	#define Y_SHIFT 0
#endif

#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x7FFF
#define YELLOW 0xFFE0
#define GRAY 0X8430
#define BRED 0XF81F
#define GRED 0XFFE0
#define GBLUE 0X07FF
#define BROWN 0XBC40
#define BRRED 0XFC07
#define DARKBLUE 0X01CF
#define LIGHTBLUE 0X7D7C
#define GRAYBLUE 0X5458

#define LIGHTGREEN 0X841F
#define LGRAY 0XC618
#define LGRAYBLUE 0XA651
#define LBBLUE 0X2B12

/* Control Registers and constant codes */
#define ST7789_NOP     0x00
#define ST7789_SWRESET 0x01
#define ST7789_RDDID   0x04
#define ST7789_RDDST   0x09

#define ST7789_SLPIN   0x10
#define ST7789_SLPOUT  0x11
#define ST7789_PTLON   0x12
#define ST7789_NORON   0x13

#define ST7789_INVOFF  0x20
#define ST7789_INVON   0x21
#define ST7789_DISPOFF 0x28
#define ST7789_DISPON  0x29
#define ST7789_CASET   0x2A
#define ST7789_RASET   0x2B
#define ST7789_RAMWR   0x2C
#define ST7789_RAMRD   0x2E

#define ST7789_PTLAR   0x30
#define ST7789_COLMOD  0x3A
#define ST7789_MADCTL  0x36

/* Page Address Order ('0': Top to Bottom, '1': the opposite) */
#define ST7789_MADCTL_MY  0x80  
/* Column Address Order ('0': Left to Right, '1': the opposite) */
#define ST7789_MADCTL_MX  0x40  
/* Page/Column Order ('0' = Normal Mode, '1' = Reverse Mode) */
#define ST7789_MADCTL_MV  0x20  
/* Line Address Order ('0' = LCD Refresh Top to Bottom, '1' = the opposite) */
#define ST7789_MADCTL_ML  0x10
/* RGB/BGR Order ('0' = RGB, '1' = BGR) */
#define ST7789_MADCTL_RGB 0x00

#define ST7789_RDID1   0xDA
#define ST7789_RDID2   0xDB
#define ST7789_RDID3   0xDC
#define ST7789_RDID4   0xDD

#define ST7789_COLOR_MODE_16bit 0x55    //  RGB565 (16bit)
#define ST7789_COLOR_MODE_18bit 0x66    //  RGB666 (18bit)

/* Basic operations */
#define ST7789_RST_Clr() {XHAL_GPIO_WritePin(xhgpiob, 10, 0);}
#define ST7789_RST_Set() {XHAL_GPIO_WritePin(xhgpiob, 10, 1);}

#define ST7789_DC_Clr() {XHAL_GPIO_WritePin(xhgpiob, 7, 0);}
#define ST7789_DC_Set() {XHAL_GPIO_WritePin(xhgpiob, 7, 1);}

#define ST7789_Select() {XHAL_GPIO_WritePin(xhgpiod, 1, 0);}
#define ST7789_UnSelect() {XHAL_GPIO_WritePin(xhgpiod, 1, 1);}

volatile XHAL_GPIO_TypeDef *xhgpioa = NULL;
volatile XHAL_GPIO_TypeDef *xhgpiob = NULL;
volatile XHAL_GPIO_TypeDef *xhgpioc = NULL;
volatile XHAL_GPIO_TypeDef *xhgpiod = NULL;

XHAL_SPI_HandleTypeDef xhspi = {0};

#define HAL_Delay(x) udelay((x) * 1000)

static void ST7789_WriteCommand(uint8_t cmd) {
	ST7789_Select();
	ST7789_DC_Clr();

	XHAL_SPI_Transmit(&xhspi, &cmd, sizeof(cmd));

	ST7789_UnSelect();
}

/**
 * @brief Write data to ST7789 controller
 * @param buff -> pointer of data buffer
 * @param buff_size -> size of the data buffer
 * @return none
 */
static void ST7789_WriteData(uint8_t *buff, size_t buff_size) {
	ST7789_Select();
	ST7789_DC_Set();

	XHAL_SPI_Transmit(&xhspi, buff, buff_size);

	ST7789_UnSelect();
}


/**
 * @brief Write data to ST7789 controller, simplify for 8bit data.
 * data -> data to write
 * @return none
 */
static void ST7789_WriteSmallData(uint8_t data) {
	ST7789_Select();
	ST7789_DC_Set();
	XHAL_SPI_Transmit(&xhspi, &data, sizeof(data));
	ST7789_UnSelect();
}

void ST7789_SetRotation(uint8_t m) {
	ST7789_WriteCommand(ST7789_MADCTL); // MADCTL
	switch (m) {
	case 0:
		ST7789_WriteSmallData(ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_RGB);
		break;
	case 1:
		ST7789_WriteSmallData(ST7789_MADCTL_MY | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);
		break;
	case 2:
		ST7789_WriteSmallData(ST7789_MADCTL_RGB);
		break;
	case 3:
		ST7789_WriteSmallData(ST7789_MADCTL_MX | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);
		break;
	default:
		break;
	}
}

/**
 * @brief Set address of DisplayWindow
 * @param xi&yi -> coordinates of window
 * @return none
 */
static void ST7789_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
	uint16_t x_start = x0 + X_SHIFT, x_end = x1 + X_SHIFT;
	uint16_t y_start = y0 + Y_SHIFT, y_end = y1 + Y_SHIFT;

	/* Column Address set */
	ST7789_WriteCommand(ST7789_CASET);
	{
		uint8_t data[] = {x_start >> 8, x_start & 0xFF, x_end >> 8, x_end & 0xFF};
		ST7789_WriteData(data, sizeof(data));
	}

	/* Row Address set */
	ST7789_WriteCommand(ST7789_RASET);
	{
		uint8_t data[] = {y_start >> 8, y_start & 0xFF, y_end >> 8, y_end & 0xFF};
		ST7789_WriteData(data, sizeof(data));
	}
	/* Write to RAM */
	ST7789_WriteCommand(ST7789_RAMWR);
}

void ST7789_Fill_Color(uint16_t color) {
	uint16_t i, j;
	ST7789_SetAddressWindow(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);
	ST7789_Select();
	for (i = 0; i < ST7789_WIDTH; i++)
		for (j = 0; j < ST7789_HEIGHT; j++) {
			uint8_t data[] = {color >> 8, color & 0xFF};
			ST7789_WriteData(data, sizeof(data));
		}
	ST7789_UnSelect();
}

void ST7789_Init(void) {

	ST7789_RST_Clr();
	HAL_Delay(100);
	ST7789_RST_Set();
	HAL_Delay(150);

	ST7789_WriteCommand(ST7789_COLMOD); //  Set color mode
	ST7789_WriteSmallData(ST7789_COLOR_MODE_16bit);
	ST7789_WriteCommand(0xB2); //   Porch control
	{
		uint8_t data[] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
		ST7789_WriteData(data, sizeof(data));
	}
	ST7789_SetRotation(ST7789_ROTATION); // MADCTL (Display Rotation)

	/* Internal LCD Voltage generator settings */
	ST7789_WriteCommand(0XB7); //   Gate Control
	ST7789_WriteSmallData(0x35); // Default value
	ST7789_WriteCommand(0xBB); //   VCOM setting
	ST7789_WriteSmallData(0x19); // 0.725v (default 0.75v for 0x20)
	ST7789_WriteCommand(0xC0); //   LCMCTRL 
	ST7789_WriteSmallData(0x2C); // Default value
	ST7789_WriteCommand(0xC2); //   VDV and VRH command Enable
	ST7789_WriteSmallData(0x01); // Default value
	ST7789_WriteCommand(0xC3); //   VRH set
	ST7789_WriteSmallData(0x12); // +-4.45v (defalut +-4.1v for 0x0B)
	ST7789_WriteCommand(0xC4); //   VDV set
	ST7789_WriteSmallData(0x20); // Default value
	ST7789_WriteCommand(0xC6); //   Frame rate control in normal mode
	ST7789_WriteSmallData(0x0F); // Default value (60HZ)
	ST7789_WriteCommand(0xD0); //   Power control
	ST7789_WriteSmallData(0xA4); // Default value
	ST7789_WriteSmallData(0xA1); // Default value
	/**************** Division line ****************/

	ST7789_WriteCommand(0xE0);
	{
		uint8_t data[] = {0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23};
		ST7789_WriteData(data, sizeof(data));
	}

	ST7789_WriteCommand(0xE1);
	{
		uint8_t data[] = {0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23};
		ST7789_WriteData(data, sizeof(data));
	}
	ST7789_WriteCommand(ST7789_INVON); //   Inversion ON
	ST7789_WriteCommand(ST7789_SLPOUT); //  Out of sleep mode
	ST7789_WriteCommand(ST7789_NORON); //   Normal Display on
	ST7789_WriteCommand(ST7789_DISPON); //  Main screen turned on

	HAL_Delay(50);
	ST7789_Fill_Color(WHITE); //    Fill with Black.
}

void lcd_init(void) {
	volatile uint32_t *reg_cpm_clkgr = (volatile uint32_t *)(0xb0000000 + 0x20);

	*reg_cpm_clkgr &= ~(1 << 19);

	volatile uint32_t *reg_ssicdr = (volatile uint32_t *)(0xb0000000 + 0x74);

	printf("SSICDR = %08lx\n", *reg_ssicdr);

	xhgpioa = (volatile XHAL_GPIO_TypeDef *)0xb0010000;
	xhgpiob = (volatile XHAL_GPIO_TypeDef *)0xb0010100;
	xhgpioc = (volatile XHAL_GPIO_TypeDef *)0xb0010200;
	xhgpiod = (volatile XHAL_GPIO_TypeDef *)0xb0010300;

	XHAL_GPIO_SetAsFunction(xhgpiod, 0, 0);

	// XHAL_GPIO_SetAsFunction(xhgpiod, 1, 0);
	XHAL_GPIO_SetAsGPIO(xhgpiod, 1, 0);
	XHAL_GPIO_WritePin(xhgpiod, 1, 1);

	XHAL_GPIO_SetAsFunction(xhgpiod, 2, 0);
	XHAL_GPIO_SetAsFunction(xhgpiod, 3, 0);

	printf("GPIOA at %08lx\n", xhgpioa);
	printf("GPIOB at %08lx\n", xhgpiob);
	printf("GPIOC at %08lx\n", xhgpioc);
	printf("GPIOD at %08lx\n", xhgpiod);

	// D_C
	XHAL_GPIO_SetAsGPIO(xhgpiob, 7, 0);
	puts("pb07 as output\n");

	// RST
	XHAL_GPIO_SetAsGPIO(xhgpiob, 10, 0);
	puts("pb10 as output\n");

	memset(&xhspi, 0, sizeof(XHAL_SPI_HandleTypeDef));
	xhspi.periph = (volatile XHAL_SPI_TypeDef *)(XHAL_PHYSADDR_SPI | 0xa0000000);

	printf("SPI0 at %08lx\n", xhspi.periph);

	XHAL_SPI_Init(&xhspi);
	puts("XHAL_SPI_Init, wait\n");

	ST7789_Init();
	puts("ST7789_Init\n");

	// BL
	XHAL_GPIO_SetAsGPIO(xhgpioc, 24, 0);
	XHAL_GPIO_WritePin(xhgpioc, 24, 0);
	puts("backlight\n");

	ST7789_Fill_Color(RED); //    Fill with Black.
}

int board_early_init_f(void) {
	return 0;
}

int board_early_init_r(void) {
	return 0;
}

int misc_init_r(void) {
	return 0;
}


int spl_start_uboot(void) {
	return 1;
}

int checkboard(void) {
	puts("Board: SudoMaker Notkia 168x (Ingenic XBurst X1000E SoC)\n");
	lcd_init();
	return 0;
}

#ifdef CONFIG_SPL_BUILD
void spl_board_init(void) {
}
#endif /* CONFIG_SPL_BUILD */
