import math

# Define the number of bits in the table
NUM_BITS = 256

# Define the maximum input value (8 bits)
MAX_INPUT = 255

# Define the scaling factor (256/ln(256))
SCALE_FACTOR = 65.861

# Create an empty table
ln_table = bytearray(NUM_BITS)

# Generate the table
for i in range(NUM_BITS):
    # Convert the index to a value between 0 and 1
    x = i / MAX_INPUT
    # Calculate the natural logarithm of x
    ln_x = math.log(x + 1e-12)
    # Scale the result and round to the nearest integer
    value = int(ln_x * SCALE_FACTOR + 0.5)
    # Clamp the value to the range [0, 255]
    value = min(max(value, 0), 255)
    # Store the value in the table
    ln_table[i] = value

# Save the table to a binary file
with open("out.bin", "wb") as f:
    f.write(ln_table)