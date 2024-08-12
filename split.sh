# Extract the first 256 bytes
dd if=a.out of=program.m bs=1 count=256

# Extract the last 4096 bytes
size=$(stat -f%z a.out)
offset=$((size - 4096))
dd if=a.out of=flash.m bs=1 skip=$offset count=4096