#ifndef __KERNEL_H__
#define __KERNEL_H__

#include "stdint.h"
#include "../const.h"

#define _TASK_FUN static void __attribute__((aligned(32)))

typedef enum task_status_e{
    TASK_STATUS_WAIT = 0,
    TASK_STATUS_STOP    ,
    TASK_STATUS_DELAY
} task_status_e_t;

typedef struct task_status {
    task_status_e_t status;
    int delay_cnt;
    uint32_t timer_value_save;
    uint32_t id;
    char *name;
}task_status_t;

/* task management */
typedef struct context {
        /* ignore x0 */
        uint32_t ra;    //0
        uint32_t sp;    //4
        uint32_t gp;    //8
        uint32_t tp;    //c
        uint32_t t0;    //0
        uint32_t t1;    //4
        uint32_t t2;    //8
        uint32_t s0;    //c
        uint32_t s1;    //0
        uint32_t a0;    //4
        uint32_t a1;    //8
        uint32_t a2;    //c
        uint32_t a3;    //0
        uint32_t a4;    //4
        uint32_t a5;    //8
        uint32_t a6;    //c
        uint32_t a7;    //0
        uint32_t s2;    //4
        uint32_t s3;    //8
        uint32_t s4;    //c
        uint32_t s5;    //0
        uint32_t s6;    //4
        uint32_t s7;    //8
        uint32_t s8;    //c
        uint32_t s9;    //0
        uint32_t s10;   //4
        uint32_t s11;   //8
        uint32_t t3;    //c
        uint32_t t4;    //0
        uint32_t t5;    //4
        uint32_t t6;    //8

	// save the mepc to run in next schedule cycle
	uint32_t epc;       // offset: 31 * 4 = 124
} context_t;

typedef struct task{
    context_t *ctx;         //0
    task_status_t *status;  //4
    struct task *next;      //8
}task_t;

void kernel_init(void);
void kernel_start(void);
void kernel_deinit(void);
void task_delay(int ms);
void kernel_task_status_log();
// uint32_t kernel_task_init(task_info_t *info);
uint32_t kernel_task_init( char *name, void(*fun)(void));



#endif