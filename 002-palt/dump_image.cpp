#include <iostream>
#include "png/png.hpp"
using namespace std;

// generate <image24bit.png>
int main(int argc, char **argv) 
{
  if(argc < 2)
  {
    cerr << "dump_image <PNG24file> \n"
      "Dumps an image as text to stdout - each line has one pixel as R G B" << endl;
    cerr << "Image must be a 24bpp PNG image!" << endl;
    return -1;
  }
  
  // Read the 24 bpp PNG
  png::image< png::rgb_pixel > image(argv[1]);
  
  // Dump pixel values
  for(int y = 0; y < image.get_height(); ++y)
  {
    auto row = image[y];
    for(int x = 0; x < image.get_width(); ++x)
    {
      auto px = row[x];
      cout << int(px.red) << " " << int(px.green) << " " << int(px.blue) << endl;
    }
  }
  
  cerr << "Dumped " << image.get_width() << " x " << image.get_height() << " = " << 
  (image.get_width() * image.get_height()) << " pixels" << endl;
  
  return 0;
}
