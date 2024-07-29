#include "image.hpp"

ImageStego::ImageStego(const std::string &imageName){
  // Initialize library
  FreeImage_Initialise();
  filetype = FreeImage_GetFileType(imageName.c_str());
  if (filetype == FIF_UNKNOWN){
    filetype = FreeImage_GetFIFFromFilename(imageName.c_str());
  }
  if (filetype == FIF_UNKNOWN || !FreeImage_FIFSupportsReading(filetype)){
    std::cerr << "Unable to access file, or incompatible filetype provided. Exiting." << std::endl;
    exit(3);
  }
  image = FreeImage_Load(filetype, imageName.c_str());
}

ImageStego::~ImageStego(){
  FreeImage_Unload(image);
  FreeImage_DeInitialise();
}

void ImageStego::encode(const std::string &textEncode, const std::string &outputName){

}

void ImageStego::decode(){
  
}

