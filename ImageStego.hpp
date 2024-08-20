#pragma once

#include <FreeImage.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <queue>
#include <bitset>
#include <cmath>

class ImageStego{
public:
  /**
  * @brief Constructor for the ImageStego class.
  *
  * @param imageName Location of the image to be loaded
  * @post Create an object ImageStego containing the bitmap of the image.
  */
  ImageStego(const std::string &imageName);
  
  /**
  * @brief Deconstructor for the ImageStego class.
  *
  * @post Deinitalize FreeImage and the image bitmap.
  */
  ~ImageStego();
  
  /** 
  * @brief Function to encode image.
  *
  * @param textEncode The text to be encoded onto the image
  * @param outputName The output location of the resulting image
  * @post Saves the image with encoded text to outputName
  */
  void encode(const std::string &textEncode, std::filesystem::path &outputName);
  
  /**
  * @brief Function to decode image.
  *
  * @post Prints the text encoded into the image.
  */
  void decode();
private:
  // FreeImage bitmap of the image
  FIBITMAP *image;
  // Filetype of the image, needed for saving
  FREE_IMAGE_FORMAT filetype;
  // Ranges of the pixel differences: 0, 8, 16, 32, 64, 128, 256
  const static int diffRange[];
  // Size of bits that can be encoded: 3, 3, 4, 5, 6, 7
  const static int bitSize[];
  // Bit queue of encoded or decoded text
  std::queue<bool> bits;
  /**
  * @brief Refill bit queue with bits of a character, used for encoding.
  *
  * @param text The text to be encoded onto the image
  * @param index The index of the next character of text
  * @post The bit queue contains 8 more bits
  */
  void refillBits(const std::string &text, int &index);

  /**
  * @brief Encoder helper function
  *
  * @param color1 The color value to be encoded
  * @param color2 The color value to be encoded
  * @post color1 and color2's values are changed to encode text
  */
  void encodeColors(int &color1, int &color2);
  
  /**
  * @brief Extract a character from the bit queue, used for decoding
  *
  * @param text The text to append the character to
  * @return true if the character has been decoded and appended successfully
  * @return false if the character is a null character, indicating end of string.
  */
  bool extractChar(std::string &text);

  /**
  * @brief Decoder helper function
  *
  * @param color1 The color value to be decoded
  * @param color2 The color value to be decoded
  * @post The two color values are decoded and appended to the bit queue.
  */
  void decodeColors(const int &color1, const int &color2);
};
