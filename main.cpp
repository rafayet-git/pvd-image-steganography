#include <getopt.h>
#include <string>
#include <filesystem>
#include "ImageStego.hpp"

int main(int argc, char *argv[]){
  // Options
  std::string imageName; // Name or location of image (--image)
  std::string textEncode; // Text to encode (--encode)
  std::string outputName; // Text to save file as (--output, default = imageName_mod.*)
  bool decode = false; // Option to decode text
  
  // Getopt long options
  static struct option long_options[] = {
    {"image", required_argument, 0, 'i'},
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
          "Usage:\n"
          "[-i|--image <filename>]:   Image file to process.\n"
          "[-o|--output <filename>]:  Image location to write to. (optional)\n"
          "[-e|--encode <text>]:      Set encode mode and text\n"
          "[-d|--decode]:             Set decode mode\n"
          "[-h|--help]:               Show program options and help" << std::endl;
        return 1;
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
    std::filesystem::path save(imageName);
    outputName = save.parent_path().string() + "/" + save.stem().string() + "_mod" + save.extension().string();
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
