#!/usr/bin/env python3

import cgi
import cgitb
import random
from io import BytesIO
from PIL import Image

print("Content-Type: image/png")
print()  # End of headers

# Enable debugging (optional in production)
cgitb.enable()

# Image size
width = 256
height = 256

# Create a new image with random pixels (RGB)
image = Image.new('RGB', (width, height))
pixels = image.load()

for x in range(width):
    for y in range(height):
        r = random.randint(0, 255)
        g = random.randint(0, 255)
        b = random.randint(0, 255)
        pixels[x, y] = (r, g, b)

# Save image to an in-memory byte buffer
buffer = BytesIO()
image.save(buffer, format='PNG')
image_bytes = buffer.getvalue()

# Output the binary image
import sys
sys.stdout.flush()  # Ensure headers are written
sys.stdout.buffer.write(image_bytes)
