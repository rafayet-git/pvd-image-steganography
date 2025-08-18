#include "ImageStego.hpp"

// Initialize const arrays
const int ImageStego::diffRange[] = {0,8,16,32,64,128,256};
const int ImageStego::bitSize[] = {3,3,4,5,6,7};

ImageStego::ImageStego(const std::string &imageName){
  cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_ERROR);

  // Load image
  image = cv::imread(imageName, cv::IMREAD_UNCHANGED);
  
  // Check if the image was loaded successfully
  if (image.empty()) {
    std::cerr << "Unable to access file, or incompatible filetype provided. Exiting." << std::endl;
    exit(3);
  }
  
  // Store original channel count for format preservation
  origChannels = image.channels();
  
  // Convert to 4-channel BGRA for consistent processing and alpha preservation
  if (origChannels == 3) {
    // BGR/RGB -> BGRA 
    cv::cvtColor(image, image, cv::COLOR_BGR2BGRA);
  } else if (origChannels == 1) {
    // Grayscale -> BGRA
    cv::cvtColor(image, image, cv::COLOR_GRAY2BGRA);
  }
}

void ImageStego::encode(const std::string &textEncode, std::filesystem::path &outputName){
  // Initialize a queue to place bits 
  int charIndex = 0;
  refillBits(textEncode, charIndex);
  
  // Go through every pixel
  int width = image.cols;
  int height = image.rows;
  for (int i = 0; i < width; i++){
    for (int j = 0; j < height; j++){
      // Initialize color
      cv::Vec4b& pixel = image.at<cv::Vec4b>(j, i);
      int newB = pixel[0];
      int newGR = pixel[1];
      int newGB = pixel[1];
      int newR = pixel[2];

      // Start encoding red and green
      encodeColors(newR, newGR);
      // Refill bits queue
      if (bits.size() < 7) refillBits(textEncode, charIndex);

      // Start encoding green and blue
      encodeColors(newGB, newB);
      // Refill bits queue
      if (bits.size() < 7) refillBits(textEncode, charIndex);

      // Encode values into the pixel
      int colorGreen = ((newGR + newGB)/2);
      int colorRed = newR - (newGR - colorGreen);
      int colorBlue = newB - (newGB - colorGreen);
      // temp fix - Works, but prob inefficient.
      if (colorRed < 0){
        colorGreen += -colorRed;
        colorBlue += -colorRed;
        colorRed = 0;
      }
      if (colorBlue < 0){
        colorRed += -colorBlue;
        colorGreen += -colorBlue;
        colorBlue = 0;
      }

      // Clamp values to 0-255 range (code should have already accounted for this, but included just incase)
      colorRed = std::min(255, std::max(0, colorRed));
      colorGreen = std::min(255, std::max(0, colorGreen));
      colorBlue = std::min(255, std::max(0, colorBlue));

      // Set new values
      pixel[0] = static_cast<uchar>(colorBlue);
      pixel[1] = static_cast<uchar>(colorGreen);
      pixel[2] = static_cast<uchar>(colorRed);
    }
  }
  
  // Check if the file type is lossy
  std::string extension = outputName.extension().string();
  std::ranges::transform(extension, extension.begin(), ::tolower);  
  
  static const std::unordered_set<std::string> lossyFormats = {
    ".jpg", ".jpeg", ".j2k", ".jp2"
  };
  bool ifLossy = lossyFormats.count(extension) > 0;

  if (ifLossy){
    // Convert to png
    std::cout << "Converting image to PNG" << std::endl;
    outputName.replace_extension(".png");
  }

  // Save the image
  cv::Mat outputImage;
  if (origChannels == 3) {
    // Convert BGRA back to BGR (remove alpha channel)
    cv::cvtColor(image, outputImage, cv::COLOR_BGRA2BGR);
  } else {
    outputImage = image;
  }

  if (!cv::imwrite(outputName.string(), outputImage)){
    std::cerr << "Failed to save image: " << outputName << std::endl;
    exit(4);
  }
  std::cout << "Success! Saved image with encoded text to " << outputName << std::endl;
}

void ImageStego::decode(){
  // Initialize variables
  std::string text = "";
  bool finishedDecode = false;

  // Go through every pixel
  int width = image.cols;
  int height = image.rows;
  for (int i = 0; i < width; i++){
    for (int j = 0; j < height; j++){
      // Initialize color
      cv::Vec4b pixel = image.at<cv::Vec4b>(j, i);
      int blue = pixel[0];
      int green = pixel[1];
      int red = pixel[2];
      
      // Start decoding red and green values
      decodeColors(red, green);

      // Start decoding green and blue values
      decodeColors(green, blue);

      // Extract characters in the queue
      // Check if null characters are in the string
      if (!extractChar(text)){
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

void ImageStego::refillBits(const std::string &text, int &index){
  // Call this function whenever there is less than 8 bits in the queue
  // For characters: go from bitset 7 to 0
  if (index < text.size()){
    std::bitset<8> bitsChar(text[index]);
    for (int i = 7; i >= 0; i--){
      bits.push(bitsChar[i]);
    }
    index++;
  } else {
    for (int i = 0; i < 8; i++){
      bits.push(0);
    }
  }
}

void ImageStego::encodeColors(int &color1, int &color2){
  // Difference of green and red
  int diff = std::abs(color1 - color2);
  // Get bit sizes of the difference
  int bitRange = 0;
  for (int k = 0; k < 7; k++){
    if (diffRange[k] > diff){
      bitRange = k-1;
      break;
    }
  }
  // if ((255 - large >= diffRange[bitRange+1] - diffRange[bitRange]) && (small >= diffRange[bitRange+1] - diffRange[bitRange])){
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

}

bool ImageStego::extractChar(std::string &text){
  // Return true if a printable character is found
  // Return false if null character is found, indicating end of string
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

void ImageStego::decodeColors(const int &color1, const int &color2){  
  // Calculate larger and smaller values of the two colors.
  // Difference of green and red
  int diff = std::abs(color1 - color2);
  // Get bit sizes of the difference
  int bitRange = 0;
  for (int k = 0; k < 7; k++){
    if (diffRange[k] > diff){
      bitRange = k-1;
      break;
    }
  }
  std::bitset<8> diffBits(diff);
  for (int k = bitSize[bitRange]-1 ; k >= 0; k--){
    bits.push(diffBits[k]);
  }
}