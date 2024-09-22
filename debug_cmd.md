# first
```bash
cd /home/lyy/vecRiscv/src/test/cpp/murax && obj_dir/VMurax
```
# second
```bash
cd /home/lyy/Downloads/openocd_riscv && src/openocd -f tcl/interface/jtag_tcp.cfg -c 'set MURAX_CPU0_YAML /home/lyy/vecRiscv/cpu0.yaml' -f tcl/target/murax.cfg
```
# finally
```bash
/opt/rv32im/bin/riscv32-unknown-elf-gdb /home/lyy/vecRiscv/src/main/c/murax/myRtos/build/myRtos.elf
#gdb cmd
target remote localhost:3333
#下面的命令每次重新debug都要输一遍
monitor reset halt
load
continue
```