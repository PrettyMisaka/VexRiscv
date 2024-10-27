#include "kernel.h"

#include <stdlib.h>
#include <string.h>

#include "../interrupt/interrupt.h"
#include "../prescaler.h"
#include "../uart.h"
#include "time.h"

#define KERNEL_INFO_LOG 0

static uint8_t __attribute__((aligned(16))) task_stack[KERNEL_MAX_TASK_COUNT][KERNEL_TASK_STACK_MAX_SIZE];

static volatile int task_cnt = 0;

static task_t *task_when_none_task_ready = NULL;
static task_t task_running;
static task_t task_queue_ready;
static task_t task_queue_delay;
const static task_t *ptask_running = &task_running;
const static task_t *ptask_queue_delay = &task_queue_delay;
const static task_t *ptask_queue_ready = &task_queue_ready;

static task_t *task_pointer_list[KERNEL_MAX_TASK_COUNT] = {0};
static task_t task_list[KERNEL_MAX_TASK_COUNT] = {0};
static task_status_t *task_status_pointer_list[KERNEL_MAX_TASK_COUNT] = {0};
static task_status_t task_status_list[KERNEL_MAX_TASK_COUNT] = {0};
static context_t task_ctx_list[KERNEL_MAX_TASK_COUNT] = {0};

static context_t temp_ctx, *ptemp_ctx = &temp_ctx;
static uint32_t task_empty_mepc;

static volatile uint32_t systick_cnt = 0;

extern void trap_vector(void);

void kernel_task_info_log(task_t *p)
{
    print("name:");
    print(p->status->name);
    print(",id:");
    printhex(p->status->id);
    print(",addr:0x");
    printhex((uint32_t)p);
    print(",ctx addr:0x");
    printhex((uint32_t)p->ctx);
    print(",status addr:0x");
    printhex((uint32_t)p->status);
    print(",epc:0x");
    printhex(p->ctx->epc);
    print(",delay_cnt:");
    printhex(p->status->delay_cnt);
    println("");
}

void kernel_task_status_log()
{
#if KERNEL_INFO_LOG
    // return;

    println("<<< task status log >>>");
    task_t *p = &task_running;
    println("run task:");
    if(p->next)
        kernel_task_info_log(p->next);
    else
        println("NULL");

    p = &task_queue_ready;
    println("ready task:");
    if(p->next){
        do{
            kernel_task_info_log(p->next);
            p = p->next;
        }while(p->next);
    }
    else
        println("NULL");

    p = &task_queue_delay;
    print("delay task:");
    if(p->next){
        do{
            kernel_task_info_log(p->next);
            p = p->next;
        }while(p->next);
    }
    else
        println("NULL");

#endif
}

void kernel_init()
{
    for(int i =0; i < KERNEL_MAX_TASK_COUNT; i++){
        task_pointer_list[i] = &task_list[i];
        task_status_pointer_list[i] = &task_status_list[i];
        printhex((uint32_t)task_pointer_list[i]);
        print("|");
        printhex((uint32_t)task_status_pointer_list[i]);
        println("");
    };

        printhex((uint32_t)&task_running);
        print("|");
        printhex((uint32_t)&task_queue_ready);
        print("|");
        printhex((uint32_t)&task_queue_delay);
        print("|");

    //systick init
	interruptCtrl_mask_set(TIMER_INTERRUPT,0x1);
	prescaler_set(TIMER_PRESCALER,9 - 1);
	timer_limit_set(TIMER_A,9000 - 1);
	timer_autoreload_en(TIMER_A);

	asm volatile("csrw mtvec, %0" : : "r" ((uint32_t)trap_vector));
    asm volatile ("csrw mscratch, %0" : : "r" (&temp_ctx));

    task_cnt = 0;
    systick_cnt = 0;
    task_running.next = NULL;
    task_queue_ready.next = NULL;
    task_queue_delay.next = NULL;

    println("kernel init success!");
}

extern void switch_to(context_t *next);
extern void switch_to_mepc(uint32_t mepc);

static int kernel_task_queue_add(task_t *head, task_t *p);
static int kernel_task_queue_mv( task_t *dst, task_t *src, task_t *p);

void kernel_start()
{
// spin_lock();
    timer_clear(TIMER_A);
	timer_ticks_en(TIMER_A);
}

void kernel_deinit()
{
	interruptCtrl_mask_set(TIMER_INTERRUPT,0x0);
    task_cnt = 0;

    println("kernel deinit success!");
}

extern void task_save(void);

static inline uint32_t systick_cnt_get()
{
    return systick_cnt;
}

void task_delay(int ms)
{
// spin_lock();
    ptask_running->next->status->delay_cnt = ms;
// spin_unlock();
    while(ptask_running->next->status->delay_cnt);
}

uint32_t kernel_task_init(const char *name, void(*fun)(void))
{
    systick_cnt++;

    print("task_cnt:");
    printhex(task_cnt);
    println("");

    if(task_cnt == KERNEL_MAX_TASK_COUNT)
        return -1;

    task_t *ptask           = task_pointer_list[task_cnt];
    context_t *ctx          = &task_ctx_list[task_cnt];
    task_status_t *status   = task_status_pointer_list[task_cnt];

    ptask->next = NULL;
    ptask->ctx = ctx;
    ptask->status = status;

    status->id           = task_cnt  ;
    status->delay_cnt    = 0         ;
    status->status       = TASK_STATUS_WAIT  ;

    memcpy(&status->name,name,10);

    ctx->sp  = (uint32_t)&task_stack[task_cnt][KERNEL_TASK_STACK_MAX_SIZE];
    ctx->epc = (uint32_t)fun;

    // print("addr:");
    // printhex((uint32_t)ptask);
    // print("|");
    // printhex((uint32_t)ctx);
    // print("|");
    // printhex((uint32_t)status);
    // print("fun:");
    // printhex((uint32_t)fun);
    // print("|");
    // printhex((uint32_t)ctx->epc);
    // print("\n");

    kernel_task_queue_add(&task_queue_ready,ptask);

    kernel_task_status_log();

    task_cnt++;
    return 0;
}

static int kernel_task_queue_add(task_t *head, task_t *p)
{
    while(head->next != NULL)
        head = head->next;
    head->next = p;
    p->next = NULL;
    return 0;
}

static int kernel_task_queue_mv( task_t *dst, task_t *src, task_t *p)
{
    while(src->next != p)
    {
        if(!src->next) return 1;
        src = src->next;
    }
    src->next = p->next;
    kernel_task_queue_add(dst, p);
    return 0;
}

static void kernel_systick_trap_handler()
{
    if(task_cnt == 0)
        return ;

// spin_lock();

    if(task_running.next == NULL){
        task_empty_mepc = ptemp_ctx->epc;
    }else{
        memcpy(task_running.next->ctx,&temp_ctx,sizeof(context_t));
        // task_status_t *pstatus = task_running.next->status;
        // if(pstatus->delay_cnt > 0){
        //     pstatus->status = TASK_STATUS_WAIT;
        //     kernel_task_queue_mv(
        //         &task_queue_delay,
        //         &task_running,
        //         task_running.next
        //     );
        // }
    }

    for(int i = 0; i < KERNEL_MAX_TASK_COUNT; i++)
    {
        task_status_t *p = task_list[i].status;
        if(p->delay_cnt > 0)
            p->delay_cnt--;
    }

    // task_t *_task_queue_delay = &task_queue_delay;
    // _task_queue_delay = _task_queue_delay->next;
    // while(_task_queue_delay != NULL){
    //     task_t *_current_task = _task_queue_delay;
    //     task_status_t *status = _current_task->status;
    //     status->delay_cnt--;
    //     _task_queue_delay = _task_queue_delay->next;
    //     if(status->delay_cnt == 0)
    //         kernel_task_queue_mv(
    //             &task_queue_ready,
    //             &task_queue_delay,
    //             _current_task
    //         );
    // }

    if (ptask_running->next == NULL){
        if(task_queue_ready.next){
            kernel_task_queue_mv(
                &task_running,
                &task_queue_ready,
                task_queue_ready.next
            );
        }else
            task_running.next = NULL;
    } else{
        if(task_queue_ready.next){
            kernel_task_queue_mv(
                &task_queue_ready,
                &task_running,
                task_running.next
            );
            kernel_task_queue_mv(
                &task_running,
                &task_queue_ready,
                task_queue_ready.next
            );
        }
    }

    kernel_task_status_log();

    asm volatile ("csrw mscratch, %0" : : "r" (&temp_ctx));
    if(task_running.next)
        switch_to(task_running.next->ctx);
    else
        switch_to_mepc(task_empty_mepc);

}


uint32_t kernel_trap_handler(uint32_t epc)
{
    uint32_t ret = epc;
    // printhex(epc);
	// uint32_t mcause;
    // asm volatile ("csrr %0, mcause" : "=r" (mcause));

	if(interruptCtrl_pendins_get(TIMER_INTERRUPT)){
		interruptCtrl_clear(TIMER_INTERRUPT);

        kernel_systick_trap_handler();
	}

    return ret;
}
