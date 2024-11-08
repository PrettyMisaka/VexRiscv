#include "spi.h"
#include "../uart.h"

void spi_clk_div_set(Spi_Reg* reg, uint32_t val)
{
    reg->CLK_DIV = val;
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

uint8_t spi_read(Spi_Reg* reg)
{
    uint32_t ret;
    do{
        ret = reg->DATA;
    }while ((ret & 0x80000000) != 0x80000000);    
    return ret & 0xff ;
}

int spi_read_len(Spi_Reg* reg, uint8_t *byte, uint16_t len)
{
    uint16_t _len;
    for(int i = 0; i < len; i++){
        byte[i] = spi_read(reg);
    }
    return 0;
}