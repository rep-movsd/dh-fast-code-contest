#include <iostream>
#include <string>
#include <png++/png.hpp>
using namespace std;



int main(int argc, char **argv) {

	if(argc < 5) 
  {
	    cerr << "dump_png <original-image> <test-file> <palette-file> <output-file>\n"
	    "Generates a PNG with the same dimensions as <original-image> replaced by color indexes from <test-file> with palette <palette-file>." << endl;
	    return -1;
	}
	
  string sOrig = argv[1];
  string sTest = argv[2];
  string sPalette = argv[3];
  string sOutFile = argv[4];

  ifstream filePalette(sPalette);
  png::rgb_pixel arrPalette[256];
  for(int i = 0; i < 256; ++i)
  {
    int r, g, b;
    filePalette >> r >> g >> b;
    arrPalette[i] = png::rgb_pixel(r, g, b);
  }
  
  
  ifstream fileTest(sTest);

	png::image< png::rgb_pixel > original_img(sOrig);

	long int oc_height = original_img.get_height();
	long int oc_width = original_img.get_width();

  cerr << "Original image: " << sOrig << endl;
  cerr << oc_width << "x" << oc_height << endl;
  
	png::image<png::rgb_pixel> out_img(oc_width, oc_height);

	int index;

	for (png::uint_32 y = 0; y < oc_height; ++y)
	{
	    for (png::uint_32 x = 0; x < oc_width; ++x)
	    {
        if(fileTest >> index) 
        {
          out_img[y][x] = arrPalette[index];
        } 
        else 
        {
          cerr << "Colors file and image resolution don't match, aborting." << endl;
          return -1;
        }
	    }
	}

	cerr << "Writing image: " << sOutFile << endl;
	out_img.write(sOutFile);
	return 0;
}
