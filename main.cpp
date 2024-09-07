#include <getopt.h>
#include <string>
#include <filesystem>
#include "ImageStego.hpp"

int main(int argc, char *argv[]){
  // Options
  std::string imageName; // Name or location of image (--image)
  std::string textEncode; // Text to encode (--encode)
  std::filesystem::path outputName; // Text to save file as (--output, default = imageName_mod.*)
  bool decode = false; // Option to decode text
  
  // Getopt long options
  static struct option long_options[] = {
    {"input", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    {"encode", required_argument, 0, 'e'},
    {"decode", no_argument, 0, 'd'},
    {"help", no_argument, 0, 'h'},
    {0,0,0,0}
  };

  // Parse args
  int opt;
  int opt_index = 0;
  while ((opt = getopt_long(argc, argv, "i:e:o:dh", long_options, &opt_index)) != -1){
    switch (opt){
      case 'i':
        imageName = optarg;
        break;
      case 'o':
        outputName = optarg;
        break;
      case 'e':
        if (decode){
          std::cerr << "Cannot pass both encode and decode arguments. Exiting." << std::endl;
          return 2;
        }
        textEncode = optarg;
        break;
      case 'd':
        if (!textEncode.empty()){
          std::cerr << "Cannot pass both encode and decode arguments. Exiting." << std::endl;
          return 2;
        }
        decode = true;
        break;
      case 'h':
      case '?':
      default:
        std::cout << 
          "Usage: imgstego -i|--input <filename> [options]\n"
          "Options:\n"
          "\t-o|--output <filename>  Specify output image file (optional)\n"
          "\t-e|--encode <text>      Encode the image with provided text\n"
          "\t-d|--decode             Decode the image and display text\n"
          "\t-h|--help               Display program options" << std::endl;
        return 1;
    }
  }
  
  // Parse default/ floating arguments, mainly the input file and textEncode
  // I did this to make it slightly easier when encoding en masse
  int i = optind;
  if(i > 0) {
    if(i < argc && imageName.empty())
      imageName = argv[i++];
    if(i < argc && textEncode.empty() && !decode) 
      textEncode = argv[i++];
    if(i < argc && outputName.empty())
      outputName = argv[i++];
    if(i < argc){
      while(i<argc) {
        std::cout << "Invalid option -- " << argv[i++] << std::endl;
      }
    }
  }

  // Check for invalid input
  if (imageName.empty()){
    std::cerr << "Image name was not provided. Exiting." << std::endl;
    return 2;
  }
  if (!decode and textEncode.empty()){
    std::cerr << "No mode selected. Exiting." << std::endl;
    return 2;
  }
  if (outputName.empty()){
    outputName = imageName;
    outputName.replace_filename(outputName.stem().string() + "_mod" + outputName.extension().string());
  }

  // Parse image
  ImageStego ciph(imageName);
  if (decode){
    std::cout << "Decoding image " << imageName << std::endl;
    ciph.decode();
  }else{
    std::cout << "Encoding image " << imageName << " with text: \"" << textEncode << "\"" << std::endl;
    ciph.encode(textEncode, outputName);
  }


  return 0;
}
