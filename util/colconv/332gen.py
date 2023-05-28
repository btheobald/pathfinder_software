def rgb332_to_rgb565(rgb332_list):
    rgb565_list = []
    for rgb332 in rgb332_list:
        r = (rgb332 & 0xE0) >> 5
        g = (rgb332 & 0x1C) >> 2
        b = (rgb332 & 0x03) 

        rx = (r << 2) | ((r & 0x06) >> 1)
        gx = (g << 3) |   g
        bx = (b << 3) |  (b << 1) | ((b & 0x2) >> 1)

        rgb565 = rx << 11 | gx << 5 | bx
        rgb565_list.append(rgb565)
    return rgb565_list

rgb565_values = rgb332_to_rgb565(range(256))

counter = 0

for value in rgb565_values:
    print(f"{value:#06x},", end='')

    # Increment counter
    counter += 1

    # Add newline after every 16 values
    if counter % 16 == 0:
        print('')
