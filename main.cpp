#include <iostream>
#include <getopt.h>
#include <string>

int main(int argc, char *argv[]){
  // Options
  std::string imageName; // Name or location of image (--image)
  std::string textEncode; // Text to encode (--encode)
  std::string outputName; // Text to save file as (--output, default = imageName_mod.*)
  bool decode = false; // Option to decode text
  bool grayscale = false; // Option for grayscaling image
  
  // Getopt long options
  static struct option long_options[] = {
    {"image", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    {"encode", required_argument, 0, 'e'},
    {"decode", no_argument, 0, 'd'},
    {"grayscale", no_argument, 0, 'g'},
    {"help", no_argument, 0, 'h'},
    {0,0,0,0}
  };

  // Parse args
  int opt;
  int opt_index = 0;
  while ((opt = getopt_long(argc, argv, "i:e:o:dgh", long_options, &opt_index)) != -1){
    switch (opt){
      case 'i':
        imageName = optarg;
        // Add filetype check here maybe?
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
      case 'g':
        grayscale = true;
        break;
      case 'h':
      case '?':
      default:
        std::cout << 
          "Usage:\n"
          "[-i|--image <filename>]:   Image file to process.\n"
          "[-o|--output <filename>]:  Image location to write to. (optional)\n"
          "[-e|--encode <text>]:      Set encode mode and text\n"
          "[-d|--decode]:             Set decode mode\n"
          "[-g|--grayscale]:          Set image to grayscale. (optional)\n"
          "[-h|--help]:               Show program options and help" << std::endl;
        return 1;
    }
  }
  if (imageName.empty()){
    std::cerr << "Image name was not provided. Exiting." << std::endl;
    return 2;
  }
  if (!decode and textEncode.empty()){
    std::cerr << "No mode selected. Exiting." << std::endl;
    return 2;
  }
  





  return 0;
}
