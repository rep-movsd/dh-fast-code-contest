#include <iostream>
#include "png/png.hpp"
using namespace std;

// generate <image24bit.png>
int main(int argc, char **argv) 
{
  // Read the 24 bpp PNG
  png::image< png::index_pixel > image(argv[1]);
  
  // Dump palette values
  const auto &pal = image.get_palette();
  for(const auto &clr: pal)
  {
    cout << int(clr.red) << " " << int(clr.green) << " " << int(clr.blue) << endl;
  }
  
  return 0;
}
