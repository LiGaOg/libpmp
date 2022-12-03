export riscv64_toolchains_path=/opt/riscv/bin/
cd lib
make clean
make
cd ../kernel
make clean
make debug
