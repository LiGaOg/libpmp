
# Inspect PMP registers in gdb

### Boot debian on qemu
![](https://cdn.nlark.com/yuque/0/2022/png/26841242/1665519464160-0090c227-230a-436d-b126-713981431aec.png)

### Ctrl a + c to open qemu monitor, and create a remote server for gdb
![](https://cdn.nlark.com/yuque/0/2022/png/26841242/1665519498265-0ff2929d-9fee-4f46-9262-00078b7f834e.png)

### Open gdb for riscv64 (My gdb is compiled locally long time ago)
### Type `target remote:1234` to connect with qemu
![](https://cdn.nlark.com/yuque/0/2022/png/26841242/1665519556514-60a20fca-0984-4da0-891a-cd242eebd3d0.png)

### `info all-r` to see the pmp registers.
![](https://cdn.nlark.com/yuque/0/2022/png/26841242/1665519593279-8d5b3c88-f959-47dd-863a-0de00ebc36c2.png)