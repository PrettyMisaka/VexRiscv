#include "sd_ctl.h"
#include "../interrupt/interrupt.h"
#include "../uart.h"

static uint8_t sd_card_tx_cmd[6];
static uint8_t sd_card_rx_buf[6];

static int sd_card_get_r1(uint8_t target, uint8_t mask, uint8_t cnt);

static void sd_card_get_arr(uint8_t *byte,int len);

static int sd_card_send_cmd(uint8_t index, uint32_t arg, uint8_t crc7);

void sd_card_init()
{
    int cnt = 0;

    spi_clk_div_set(SD_CARD_REG,36);

	spi_cs_en(SPI, 0);

    sd_card_get_r1(0,0,1);
    sd_card_get_r1(0,0,1);
    sd_card_get_r1(0,0,1);
    sd_card_get_r1(0,0,1);
    sd_card_get_r1(0,0,1);
    sd_card_get_r1(0,0,1);
    sd_card_get_r1(0,0,1);
    sd_card_get_r1(0,0,1);
    sd_card_get_r1(0,0,1);
    sd_card_get_r1(0,0,1);
    sd_card_get_r1(0,0,1);

    //CMD0
    sd_card_send_cmd(0,0,0x95);
    systick_delayms(30);
    //R1
    sd_card_get_r1(0x00,0x80,-1);

    //CMD8
    sd_card_send_cmd(8,0x000001aa,0x87);
    sd_card_get_r1(0x00,0x80,-1);
    sd_card_get_arr(sd_card_rx_buf,4);
    if(sd_card_rx_buf[2] == 0x01 && sd_card_rx_buf[3] == 0xaa)
        println("SD Card CMD8 success");
        
    for (int i = 0; i < 5; i++)
    {
        //APP_CMD
        sd_card_send_cmd(55,0,0x65);
        //R1
        sd_card_get_r1(0x00,0x80,-1);
        //CMD41->ACMD41
        sd_card_send_cmd(41,0x40000000,0xFF);

        if(sd_card_get_r1(0x01,0x81,3) == 0) break;

        systick_delayms(500);
    }

    sd_card_send_cmd(58,0,0x4d);

    sd_card_get_r1(0x00,0x80,-1);

    sd_card_get_arr(sd_card_rx_buf,4);

    if(sd_card_rx_buf[0]&0x40)    //检查CCS标志
        println("SD_Type:SD_TYPE_V2HC");
    else
        println("SD_Type:SD_TYPE_V2");

    // systick_delayms(500);

    // sd_card_send_cmd(17,0x300,0x6f);
    // sd_card_get_r1(0x00,0x80,-1);

    spi_cs_disable(SPI,0);

    sd_card_get_r1(0x00,0x80,1);

}

static void sd_card_get_arr(uint8_t *byte,int len)
{
    uint16_t empty;
    for(int i = 0; i < len; i++)
    {
        spi_send_and_rx(SD_CARD_REG, 0xff);
        spi_read(SD_CARD_REG,&byte[i],&empty);
    }
}

static int sd_card_get_r1(uint8_t target, uint8_t mask, uint8_t cnt)
{
    int ret = 1;
    while (ret && cnt != 0)
    {
        spi_send_and_rx(SD_CARD_REG, 0xff);
        spi_read_len(SD_CARD_REG,sd_card_rx_buf,1);
        ret = (sd_card_rx_buf[0] & mask) == target?0:1;
        if(ret == 0) return 0;
        cnt--;
    }
    return 1;
}

static int sd_card_send_cmd(uint8_t index, uint32_t arg, uint8_t crc7)
{
    if(index >= 0x40) return 1;

    sd_card_tx_cmd[0] = 0x40 | index;
    sd_card_tx_cmd[4] = 0xff & arg;
    sd_card_tx_cmd[3] = 0xff & (arg >> 8);
    sd_card_tx_cmd[2] = 0xff & (arg >> 16);
    sd_card_tx_cmd[1] = 0xff & (arg >> 24);
    // sd_card_tx_cmd[5] = 0x00 | (crc7 << 1);
    sd_card_tx_cmd[5] = 0x00 | crc7;

    spi_send_arr(SD_CARD_REG, sd_card_tx_cmd, 6);
    return 1;
}
