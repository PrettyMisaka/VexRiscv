#include "spi.h"
#include "../uart.h"

inline uint16_t spi_tx_fifo_get(Spi_Reg* reg)
{
    return (reg->STATUS >>16 )& 0xffff;
}

void spi_clk_div_set(Spi_Reg* reg, uint32_t val)
{
    reg->CLK_DIV = val;
}

volatile inline void spi_wait_tx_complete(Spi_Reg* reg, int tx_fifo_max)
{
    while(((reg->STATUS >>16 )& 0xffff) != tx_fifo_max);
}

void spi_send(Spi_Reg* reg, uint8_t byte)
{
    while(!SPI_TX_AVAILABLE);
    reg->DATA = byte;
}

void spi_send_and_rx(Spi_Reg* reg, uint8_t byte)
{
    while(!SPI_TX_AVAILABLE);
    reg->DATA = 0x01000000 | byte;
}

void spi_send_arr(Spi_Reg* reg, uint8_t *byte, int n)
{
    for(int i = 0; i < n; i++)
    {
        spi_send(reg,byte[i]);
    }
}

void spi_send_and_rx_arr(Spi_Reg* reg, uint8_t *byte, int n)
{
    for(int i = 0; i < n; i++)
    {
        spi_send_and_rx(reg,byte[i]);
    }
}

void spi_cs_en(Spi_Reg* reg, uint8_t n)
{
    if(n > 15) return;
    reg->DATA = 0x11000000 | n;
}

void spi_cs_disable(Spi_Reg* reg, uint8_t n)
{
    if(n > 15) return;
    reg->DATA = 0x10000000 | n;
}

void spi_read_irq_en(Spi_Reg* reg)
{
    reg->STATUS = reg->STATUS | 0x2;
}

int spi_read_irq_pending_get(Spi_Reg* reg)
{
    reg->STATUS = reg->STATUS & ~0x00000200;
    return SPI_RX_IRQ_AVAILABLE ? 0:1;
}

int spi_read(Spi_Reg* reg, uint8_t *byte, uint16_t *len)
{
    while(!SPI_RX_AVAILABLE);
    uint32_t ret = reg->DATA;
    *len = (ret & 0x7fff0000) >> 16;
    *byte = (ret & 0x80000000) ? ret & 0xff : 0;
    return ret & 0x80000000 ? 0:1;
}

int spi_read_len(Spi_Reg* reg, uint8_t *byte, uint16_t len)
{
    uint16_t _len;
    for(int i = 0; i < len; i++){
        spi_read(reg,&byte[i],&_len);
    }
    return 0;
}