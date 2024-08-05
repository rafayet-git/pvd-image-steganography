#include "image.hpp"

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
  image = FreeImage_ConvertTo32Bits(imagedata);
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
  

}

void ImageStego::decode(){
  
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
