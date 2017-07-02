#include <iostream>
#include "png/png.hpp"
using namespace std;


// generate <image24bit.png>
int main(int argc, char **argv) 
{
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
  
  return 0;
}
