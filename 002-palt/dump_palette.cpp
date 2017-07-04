#include <iostream>
#include "png/png.hpp"
using namespace std;

// generate <image24bit.png>
int main(int argc, char **argv) 
{
  if(argc < 2)
  {
    cerr << "dump_palette <PNGfile> \n"
    "Dumps the 256 palette entries of a 8 bit PNG image as text to stdout - each line has one pixel as R G B" << endl;
    return -1;
  }
  
  // Read the 24 bpp PNG
  png::image< png::index_pixel > image(argv[1]);
  
  // Dump palette values
  const auto &pal = image.get_palette();
  for(const auto &clr: pal)
  {
    cout << int(clr.red) << " " << int(clr.green) << " " << int(clr.blue) << endl;
  }
  
  cerr << "Done" << endl;
  return 0;
}
