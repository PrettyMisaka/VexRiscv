#ifndef __SPI_H__
#define __SPI_H__

#include <stdint.h>
#include "../const.h"

#define SD_CARD_REG SPI

typedef struct
{
  volatile uint32_t DATA;       //0
  volatile uint32_t STATUS;     //4
  volatile uint32_t CFG;        //8
  volatile uint32_t CLK_DIV;    //c
  volatile uint32_t SS_SETUP;   //10
  volatile uint32_t SS_HOLD;    //14
  volatile uint32_t SS_DISABLE; //18
} Spi_Reg;

#define SPI_TX_AVAILABLE SPI->STATUS & 0xffff0000
#define SPI_RX_AVAILABLE SPI->STATUS & 0x00000200
#define SPI_RX_IRQ_AVAILABLE SPI->STATUS & 0x00000200

uint16_t spi_tx_fifo_get(Spi_Reg* reg);
void spi_clk_div_set(Spi_Reg* reg, uint32_t val);
void spi_wait_tx_complete(Spi_Reg* reg, int tx_fifo_max);

void spi_send(Spi_Reg* reg, uint8_t byte);
void spi_send_arr(Spi_Reg* reg, uint8_t *byte, int n);
void spi_send_and_rx(Spi_Reg* reg, uint8_t byte);
void spi_send_and_rx_arr(Spi_Reg* reg, uint8_t *byte, int n);

int spi_read(Spi_Reg* reg, uint8_t *byte, uint16_t *len);
int spi_read_len(Spi_Reg* reg, uint8_t *byte, uint16_t len);

void spi_cs_en(Spi_Reg* reg, uint8_t n);
void spi_cs_disable(Spi_Reg* reg, uint8_t n);

void spi_read_irq_en(Spi_Reg* reg);
int spi_read_irq_pending_get(Spi_Reg* reg);

#endif