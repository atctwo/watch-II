from PIL import Image
from argparse import ArgumentParser
from math import ceil

parser = ArgumentParser(description = "Convert an image from RGB888 to RGB565, and save it as a C array")
parser.add_argument( "input", metavar = "input", help = "The file to adapt" )
parser.add_argument( "--output", "-o", default = "poppyseed.c", help = "Where to save and what to name the final collage")
parser.add_argument( "--xscale", default = 1, type = float, help = "Scale the width of images.  1 means unscaled, 0.5 means scaled to half, etc")
parser.add_argument( "--yscale", default = 1, type = float, help = "Scale the height of images.  1 means unscaled, 0.5 means scaled to half, etc")

args = parser.parse_args()

im = Image.open(args.input)
out = open(args.output, "w")

image_width  = ceil( im.width  * args.xscale)
image_height = ceil( im.height * args.yscale)

im.resize((image_width, image_height))

out.write("//input file: {}\n".format(args.input));
out.write("static unsigned short {}[]".format(args.output.split(".")[0]))
out.write(" = {\n\t")

output_thing = 0
pixels = im.width * im.height

for y in range(im.height):
    for x in range(im.width):

        r,g,b,a = im.getpixel((x, y))
        if (a == 0): rgb = 0
        else: rgb = ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);

        if (output_thing % 20 == 0): out.write("\n\t");

        out.write("{0:#0{1}x}".format(rgb, 6))
        if (output_thing != pixels-1): out.write(",")

        output_thing += 1

out.write("\n};")
print("{} bytes".format(pixels * 2))
