#include "image.hpp"
#include <FreeImage.h>

// Initialize const arrays
const int ImageStego::diffRange[] = {0,8,16,32,64,128,256};
const int ImageStego::bitSize[] = {3,3,4,5,6,7};

ImageStego::ImageStego(const std::string &imageName){
  // Initialize library
  FreeImage_Initialise();
  filetype = FreeImage_GetFileType(imageName.c_str());
  // Check if the file exists and can be processed
  if (filetype == FIF_UNKNOWN){
    filetype = FreeImage_GetFIFFromFilename(imageName.c_str());
  }
  // FIF_UNKNOWN implies file access issues
  // Also do not accept readonly filetypes, as we need it to save the image
  if (filetype == FIF_UNKNOWN || !FreeImage_FIFSupportsReading(filetype) || !FreeImage_FIFSupportsWriting(filetype)){
    std::cerr << "Unable to access file, or incompatible filetype provided. Exiting." << std::endl;
    FreeImage_DeInitialise();
    exit(3);
  }
  FIBITMAP *imagedata = FreeImage_Load(filetype, imageName.c_str());
  image = FreeImage_ConvertTo24Bits(imagedata);
  FreeImage_Unload(imagedata);
  // Doing this just in case
  if (!image){
    std::cerr << "Unable to access file. Exiting." << std::endl;
    this->~ImageStego();
    exit(3);
  }
}

ImageStego::~ImageStego(){
  FreeImage_Unload(image);
  FreeImage_DeInitialise();
}

void ImageStego::encode(const std::string &textEncode, const std::string &outputName){
  // Initialize a queue to place bits 
  std::queue<bool> bits;
  int charIndex = 0;
  refillBits(bits, textEncode, charIndex);
  
  // Go through every pixel
  int width = FreeImage_GetWidth(image);
  int height = FreeImage_GetHeight(image);
  for (int i = 0; i < width; i++){
    for (int j = 0; j < height; j++){
      // Initalize color
      RGBQUAD color;
      FreeImage_GetPixelColor(image, i, j, &color);
      int newR = color.rgbRed;
      int newGR = color.rgbGreen;
      int newGB = color.rgbGreen;
      int newB = color.rgbBlue;
      bool oGR = true, oGB = true;
      int diffGR, diffGB;

      // Start encoding red and green
      encodeColors(oGR, diffGR, newR, newGR, bits);
      // Refill bits queue
      if (bits.size() < 7) refillBits(bits, textEncode, charIndex);

      // Start encoding green and blue
      encodeColors(oGB, diffGB, newGB, newB, bits);
      // Refill bits queue
      if (bits.size() < 7) refillBits(bits, textEncode, charIndex);

      // Encode values into the pixel
      if (oGR && oGB){
        color.rgbGreen = ((newGR + newGB)/2);
        color.rgbRed = newR - (newGR - color.rgbGreen);
        color.rgbBlue = newB - (newGB - color.rgbGreen);
      } else if (oGR){
        color.rgbRed = newR;
        color.rgbGreen = newGR;
      } else if (oGB){
        color.rgbBlue = newB;
        color.rgbGreen = newGB;
      }
      FreeImage_SetPixelColor(image, i, j, &color);
    }
  }
  // Save the image
  if (!FreeImage_Save(filetype, image, outputName.c_str())){
    std::cerr << "Failed to save image: " << outputName << std::endl;
    exit(4);
  }
  std::cout << "Success! Saved image with encoded text to " << outputName << std::endl;
}

void ImageStego::decode(){
  // Initialize variables
  std::queue<bool> bits;
  std::string text = "";
  bool finishedDecode = false;

  // Go through every pixel
  int width = FreeImage_GetWidth(image);
  int height = FreeImage_GetHeight(image);
  for (int i = 0; i < width; i++){
    for (int j = 0; j < height; j++){
      // Initalize color
      RGBQUAD color;
      FreeImage_GetPixelColor(image, i, j, &color);
      
      // Start decoding red and green values
      decodeColors(bits, color.rgbRed, color.rgbGreen);

      // Start decoding green and blue values
      decodeColors(bits, color.rgbGreen, color.rgbBlue);

      // Extract characters in the queue
      // Check if null characters are in the string
      if (!extractChar(bits, text)){
        finishedDecode = true;
        break;
      }
    }
    if (finishedDecode) break;
  }
  // Print the text
  if (!finishedDecode){
    std::cerr << "Unable to decode image." << std::endl;
  } else {
    std::cout << "Success! Decoded text: \"" << text << "\"" << std::endl;
  }
}

void ImageStego::refillBits(std::queue<bool> &bits, const std::string &text, int &index){
  // Call this function whenever there is less than 8 bits in the queue
  // For characters: go from bitset 7 to 0
  if (index < text.size()){
    std::bitset<8> bitsChar(text[index]);
    for (int i = 7; i >= 0; i--){
      bits.push(bitsChar[i]);
    }
    index++;
  }else{
    for (int i = 0; i < 8; i++){
      bits.push(0);
    }
  }
}

void ImageStego::encodeColors(bool &encodable, int &diff, int &color1, int &color2, std::queue<bool> &bits){
  // Calculate larger and smaller values of the two colors.
  int large = (color1 >= color2) ? color1 : color2;
  int small = (color1 >= color2) ? color2 : color1;
  // Difference of green and red
  diff = large - small;
  // Get bit sizes of the difference
  int bitRange = 0;
  for (int k = 0; k < 7; k++){
    if (diffRange[k] > diff){
      bitRange = k-1;
      break;
    }
  }
  if ((255 - large >= diffRange[bitRange+1] - diffRange[bitRange]) 
      && (small >= diffRange[bitRange+1] - diffRange[bitRange])){
    // Generate new difference value
    std::bitset<8> newDiff(diffRange[bitRange]);
    for (int k = bitSize[bitRange]-1; k >= 0; k--){
      if (bits.front()) newDiff.set(k);
      bits.pop();
    }
    int newDiff_int = static_cast<int>(newDiff.to_ulong());
    
    // Get the new red and green values
    double m = std::abs(newDiff_int - diff);
    if ((color1 >= color2) && (newDiff_int > diff)){
      color1 += static_cast<int>(std::ceil(m / 2.0));
      color2 -= static_cast<int>(std::floor(m / 2.0));
    } else if ((color1 < color2) && (newDiff_int > diff)){
      color1 -= static_cast<int>(std::floor(m / 2.0));
      color2 += static_cast<int>(std::ceil(m / 2.0));
    } else if ((color1 >= color2) && (newDiff_int <= diff)){
      color1 -= static_cast<int>(std::ceil(m / 2.0));
      color2 += static_cast<int>(std::floor(m / 2.0));
    } else if ((color1 < color2) && (newDiff_int <= diff)){
      color1 += static_cast<int>(std::ceil(m / 2.0));
      color2 -= static_cast<int>(std::floor(m / 2.0));
    }
    diff = newDiff_int;
  } else {
    // Do not count red and green if values have the possibility to overflow/overlap
    encodable = false;
  }
}

bool ImageStego::extractChar(std::queue<bool> &bits, std::string &text){
  // Return true if a printable character is found
  // Return false if null character is found
  while (bits.size() >= 8){
    std::bitset<8> character(0);
    for (int i = 7; i >= 0; i--){
      if (bits.front()) character.set(i);
      bits.pop();
    }
    unsigned char letter = static_cast<unsigned char>(character.to_ulong());
    if (letter == '\0') return false;
    text += letter;
  }
  return true;
}

void ImageStego::decodeColors(std::queue<bool> &bits, const int &color1, const int &color2){
  // Calculate larger and smaller values of the two colors.
  int large = (color1 >= color2) ? color1 : color2;
  int small = (color1 >= color2) ? color2 : color1;
  // Difference of green and red
  int diff = large - small;
  // Get bit sizes of the difference
  int bitRange = 0;
  for (int k = 0; k < 7; k++){
    if (diffRange[k] > diff){
      bitRange = k-1;
      break;
    }
  }
  if ((255 - large >= diffRange[bitRange+1] - diffRange[bitRange]) 
      && (small >= diffRange[bitRange+1] - diffRange[bitRange])){
    std::bitset<8> diffBits(diff);
    for (int k = bitSize[bitRange]-1 ; k >= 0; k--){
      bits.push(diffBits[k]);
    }
  }
}
