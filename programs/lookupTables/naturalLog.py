import math

pattern = bytearray(256)
for i in range(256):
    pattern[i] = int(math.log(i) if i > 0 else 0)

with open("out.bin", "wb") as f:
    f.write(pattern)