#include "pff_user.h"
#include "../uart.h"

#define BE32_TO_CPU(arr,i) \
    ((uint32_t)((arr)[i+0] << 24) | \
     (uint32_t)((arr)[i+1] << 16) | \
     (uint32_t)((arr)[i+2] <<  8) | \
     (uint32_t)((arr)[i+3] <<  0))

static uint8_t buf[PFF_USER_READ_MAX_LEN+10];
static uint32_t* target_adder;
static uint32_t target_len;
static uint32_t target_rd_len;
static uint32_t offset;

void pf_loadfile2dram()
{
    FRESULT ret;
    UINT btr, bret;
    int i;

    offset = 0;
lab_loop:
    ret = pf_read(buf,8,&bret);
    if(ret != FR_OK){
        print("pf_read error! code:");
        goto lab_err_ret;
    }
    if(bret == 0){
        println("load file done!");
        goto lab_ret;
    }
    if(bret != 8){
        print("pf_read error! target:8, ret:");
        printhex(bret);
        println("");
        goto lab_ret;
    }

    offset += bret;
    ret = pf_lseek(offset);
    if(ret != FR_OK){
        print("pf_lseek error! code:");
        goto lab_err_ret;
    }

    target_adder = (uint32_t*)BE32_TO_CPU(buf,0);
    target_len = BE32_TO_CPU(buf,4) & 0x0fffffff;

    print(">addr:0x");
    printhex(target_adder);
    print(",len:");
    printhex(target_len);
    println("");

    do{
        target_rd_len = (target_len>PFF_USER_READ_MAX_LEN)?
                    PFF_USER_READ_MAX_LEN : target_len ;
        target_len -= target_rd_len;

        ret = pf_read(buf,target_rd_len,&bret);
        if(ret != FR_OK){
            print("pf_read error! code:");
            goto lab_err_ret;
        }
        if(bret != target_rd_len){
            print("pf_read error! target:");
            printhex(target_rd_len);
            print(", ret:");
            printhex(bret);
            println("");
            goto lab_ret;
        }

        i = 0;
        do{
            *target_adder = BE32_TO_CPU(buf,i);
            i += 4;
            target_adder += 1;
        }while(i != target_rd_len);

        offset += target_rd_len;
        ret = pf_lseek(offset);
        if(ret != FR_OK){
            print("pf_lseek error! code:");
            goto lab_err_ret;
        }

    }while(target_len != 0);

    goto lab_loop;

lab_err_ret:
    printhex(ret);
    println("");

lab_ret:
    return;
}