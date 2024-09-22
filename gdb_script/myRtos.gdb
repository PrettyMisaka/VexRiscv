target remote localhost:3333
monitor reset halt
load

break kernel_start
break trap_vector
break kernel_systick_trap_handler
break task_delay
break src/kernel/kernel.c:295
break src/kernel/kernel.c:139
break src/main.c:20
break src/main.c:34

display /x $mepc 
display /x $mscratch
display ptask_running->next->status->name
display /x ptask_running->next->ctx.epc
display ptask_queue_ready->next->status->name
display /x ptask_queue_ready->next->ctx.epc
display ptask_queue_delay->next->status->name

