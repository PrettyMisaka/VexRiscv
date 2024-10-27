# Save all General-Purpose(GP) registers to context.
# struct context *base = &ctx_task;
# base->ra = ra;
# ......
# These GP registers to be saved don't include gp
# and tp, because they are not caller-saved or
# callee-saved. These two registers are often used
# for special purpose. For example, in RVOS, 'tp'
# (aka "thread pointer") is used to store hartid,
# which is a global value and would not be changed
# during context-switch.
.macro reg_save base
        sw ra, 0(\base)
        sw sp, 4(\base)
        sw t0, 16(\base)
        sw t1, 20(\base)
        sw t2, 24(\base)
        sw s0, 28(\base)
        sw s1, 32(\base)
        sw a0, 36(\base)
        sw a1, 40(\base)
        sw a2, 44(\base)
        sw a3, 48(\base)
        sw a4, 52(\base)
        sw a5, 56(\base)
        sw a6, 60(\base)
        sw a7, 64(\base)
        sw s2, 68(\base)
        sw s3, 72(\base)
        sw s4, 76(\base)
        sw s5, 80(\base)
        sw s6, 84(\base)
        sw s7, 88(\base)
        sw s8, 92(\base)
        sw s9, 96(\base)
        sw s10, 100(\base)
        sw s11, 104(\base)
        sw t3, 108(\base)
        sw t4, 112(\base)
        sw t5, 116(\base)
        # we don't save t6 here, due to we have used
        # it as base, we have to save t6 in an extra step
        # outside of reg_save
.endm

# restore all General-Purpose(GP) registers from the context
# except gp & tp.
# struct context *base = &ctx_task;
# ra = base->ra;
# ......
.macro reg_restore base
        lw ra, 0(\base)
        lw sp, 4(\base)
        lw t0, 16(\base)
        lw t1, 20(\base)
        lw t2, 24(\base)
        lw s0, 28(\base)
        lw s1, 32(\base)
        lw a0, 36(\base)
        lw a1, 40(\base)
        lw a2, 44(\base)
        lw a3, 48(\base)
        lw a4, 52(\base)
        lw a5, 56(\base)
        lw a6, 60(\base)
        lw a7, 64(\base)
        lw s2, 68(\base)
        lw s3, 72(\base)
        lw s4, 76(\base)
        lw s5, 80(\base)
        lw s6, 84(\base)
        lw s7, 88(\base)
        lw s8, 92(\base)
        lw s9, 96(\base)
        lw s10, 100(\base)
        lw s11, 104(\base)
        lw t3, 108(\base)
        lw t4, 112(\base)
        lw t5, 116(\base)
        lw t6, 120(\base)
.endm

# Something to note about save/restore:
# - We use mscratch to hold a pointer to context of current task
# - We use t6 as the 'base' for reg_save/reg_restore, because it is the
#   very bottom register (x31) and would not be overwritten during loading.
#   Note: CSRs(mscratch) can not be used as 'base' due to load/restore
#   instruction only accept general purpose registers.

.text

# interrupts and exceptions while in machine mode come here
.global trap_vector
.global kernel_trap_handler
# the trap vector base address must always be aligned on a 4-byte boundary
.align 4
trap_vector:
	# save context(registers).
	csrrw	t6, mscratch, t6	# swap t6 and mscratch
	reg_save t6

	# Save the actual t6 register, which we swapped into
        # mscratch
	mv	t5, t6		# t5 points to the context of current task
	csrr    t6, mscratch	# read t6 back from mscratch
	sw	t6, 120(t5)	# save t6 with t5 as base

	# save mepc to context of current task
	csrr	a0, mepc
	sw	a0, 124(t5)	

	# Restore the context pointer into mscratch
	csrw    mscratch, t5

	# call the C trap handler in trap.c
	csrr	a0, mepc
	call 	kernel_trap_handler

	# trap_handler will return the return address via a0.
	csrw    mepc, a0

	# restore context(registers).
	csrr	t6, mscratch
	reg_restore t6
	
	# return to whatever we were doing before trap.
	mret

# void switch_to(struct context *next);
# a0: pointer to the context of the next task
.global switch_to
.global switch_to_mepc
.global SPIN_UNLOCK
.align 4
switch_to:
    # switch mscratch to point to the context of the next task
        #    csrw    mscratch, a0
	# set mepc to the pc of the next task
	lw	a1, 124(a0)
	csrw 	mepc, a1

        # call spin_unlock

        # Restore all GP registers
        # Use t6 to point to the context of the new task
        mv      t6, a0
        reg_restore t6

        # Do actual context switching.
        # Notice this will enable global interrupt
        mret

.align 4
switch_to_mepc:
	csrw 	mepc, a0
        mret

.align 4
task_save:
	# save context(registers).
        csrrw	t6, mscratch, t6	# swap t6 and mscratch
        reg_save t6

        # Save the actual t6 register, which we swapped into
        # mscratch
        mv	t5, t6		# t5 points to the context of current task
        csrr    t6, mscratch	# read t6 back from mscratch
        sw	t6, 120(t5)	# save t6 with t5 as base

        # save mepc to context of current task
        csrr	a0, mepc
        sw	a0, 124(t5)	

        # Restore the context pointer into mscratch
        csrw    mscratch, t5

        ret
# 定义 MSTATUS_MIE 为 mstatus 寄存器的第 3 位
.equ MSTATUS_MIE, 0x8

# 自旋锁：禁用中断
.global spin_lock
spin_lock:
    # 保存返回地址到栈上
    addi    sp, sp, -4
    sw      ra, 0(sp)
    # 保存 a3 寄存器到栈上
    addi    sp, sp, -4
    sw      a3, 0(sp)
    # 保存 a4 寄存器到栈上
    addi    sp, sp, -4
    sw      a4, 0(sp)
    # 保存 t0 寄存器到栈上
    addi    sp, sp, -4
    sw      t0, 0(sp)
    # 保存 t1 寄存器到栈上
    addi    sp, sp, -4
    sw      t1, 0(sp)

    csrr    a3, mstatus      # 读取 mstatus 寄存器的值到 a0
    andi    a4, a3, ~MSTATUS_MIE # 清除 MIE 位
    csrrw   zero, mstatus, a4 # 将修改后的值写回 mstatus 寄存器
    # clean timerA irq mask
    li t1, 0
    lui         t0,0xf0020
    addi	t0,t0,16 
    sw	t1,4(t0)
    ret

# 自旋解锁：启用中断 清除MEIP
.global spin_unlock 
spin_unlock:
    csrr    a5, mip          # 读取 mip 寄存器的值到 a5
    li      a6, 1 << 11       # 假设 MEIP 是第 11 位，设置位掩码
    csrrc   zero, mip, a6     # 清除 mip 寄存器中的 MEIP 位
    csrr    a3, mstatus      # 读取 mstatus 寄存器的值到 a3
    ori     a3, a3, MSTATUS_MIE # 设置 mstatus 寄存器的 MIE 位
    csrw    mstatus, a3      # 将修改后的值写回 mstatus 寄存器
    li          t1, 1
    lui         t0,0xf0020
    addi	t0,t0,16 
    sw	        t1,4(t0)

    # 恢复 t1 寄存器的值
    lw      t1, 0(sp)
    addi    sp, sp, 4
    # 恢复 t0 寄存器的值
    lw      t0, 0(sp)
    addi    sp, sp, 4
    # 恢复 a4 寄存器的值
    lw      a4, 0(sp)
    addi    sp, sp, 4
    # 恢复 a3 寄存器的值
    lw      a3, 0(sp)
    addi    sp, sp, 4
    # 恢复返回地址
    lw      ra, 0(sp)
    addi    sp, sp, 4

    ret

.end
