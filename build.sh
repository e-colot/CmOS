rm -f bootloader.bin

nasm -f bin bootloader.asm -o bootloader.bin

qemu-system-i386 -drive file=bootloader.bin,format=raw