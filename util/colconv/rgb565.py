import sys

# Open the input file
with open(sys.argv[1], 'r') as f:
    lines = f.readlines()

# Initialize counter
counter = 0

# Loop through each line of the input file
for line in lines:
    # Strip whitespace and convert the hex color to an integer
    hex_color = line.strip()
    int_color = int(hex_color, 16)

    # Extract the red, green, and blue components
    red = (int_color >> 16) & 0xff
    green = (int_color >> 8) & 0xff
    blue = int_color & 0xff

    # Convert the components to 5-bit and 6-bit values
    r5 = (red * 31 + 127) // 255
    g6 = (green * 63 + 127) // 255
    b5 = (blue * 31 + 127) // 255

    # Pack the components into a 16-bit value
    rgb565 = (r5 << 11) | (g6 << 5) | b5

    # Print the result in C syntax without a comma at the end of each line
    print("0x{:04x}, ".format(rgb565), end='')

    # Increment counter
    counter += 1

    # Add newline after every 16 values
    if counter % 16 == 0:
        print('')
