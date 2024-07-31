#pragma once

#include <FreeImage.h>
#include <iostream>
#include <string>

class ImageStego{
public:
  ImageStego(const std::string &imageName);
  ~ImageStego();
  void encode(const std::string &textEncode,const std::string &outputName);
  void decode();
private:
  FIBITMAP *image;
  FREE_IMAGE_FORMAT filetype;
  const static int diffRange[];
  const static int bitSize[]; 
};
