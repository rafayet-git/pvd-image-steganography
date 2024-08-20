# PVD (+ LSB) Image Steganography

An image steganography program that will allow you to encode and decode images with any text!

Written in C++ using the FreeImage library

## How it works

This program uses a Pixel Value Differencing (PVD) method for encoding, where instead of applying data through the RGB values themselves, it takes the differences between two values and encodes data using the Least Significant Bits (LSB) of the differences.

We do this by encoding the difference of red and green, then the difference of green and red. We then get the new RGB values by taking the average of the two green values and adjusting the red and blue values using our new difference values.

Also, depending on how large the difference is, we can embed more bits while maintaining quality.

I used [this research paper](https://doi.org/10.1098/rsos.161066) as my main reference, but with a couple of tweaks.

## Build instructions

Make sure you have Make and FreeImage installed.

Run these commands:

```
git clone https://github.com/UTkbxRME7c9C/lsb-pvd-image-steganography.git
cd lsb-pvd-image-steganography/
make
```

## Usage instructions

Run `./imgstego -h`  or `--help` for usage instructions

```
Usage:
[-i|--image <filename>]:   Image file to process.
[-o|--output <filename>]:  Image location to write to. (optional)
[-e|--encode <text>]:      Set encode mode and text
[-d|--decode]:             Set decode mode
[-h|--help]:               Show program options and help
```

For example, to encode an image with a text, use this command:

```
./imgstego --image filename.png -o encoded.png --encode "Hello there"
```

And to decode:

```
./imgstego --image encoded.png --decode
```

> [!WARNING]
> This program only works with lossless or uncompressed image file types. Do not use lossy file types, such as .jpeg or .gif.
> 
> By default, the program will store the encoded image to the same directory, but appending `_mod` on the image name that is being used. Encoding `images/image.png` will save the file as `images/image_mod.png` unless if a specific output location is provided.

## Future plans

1. Proper pixel overflow handling, as it seems to cause issues when using very long messages, especially with bird.png.

2. Encryption of some kind
