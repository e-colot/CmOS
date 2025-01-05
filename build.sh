clear

#export PATH="$PATH:/usr/bin/watcom/binl"

BIN="bin"

echo -e "\n\e[33;40mCompiling files!\e[0m"
nasm -fbin Bootloader/boot.asm -o $BIN/boot.bin

dd if=/dev/zero of
=CmOS.img bs=512 count=2880
mkfs.fat -F 12 -n "CmOS" CmOS.img
dd if=$BIN/boot.bin of=CmOS.img conv=notrunc

if [ $? -eq 0 ]; then
    echo -e "\n\e[36;40mCompiled successfully!\e[0m\n"
else
    echo -e "\n\e[31;40mError compiling!\e[0m\n"
fi

qemu-system-i386 -m 2048 -smp 4 -drive format=raw,file="CmOS.img"
