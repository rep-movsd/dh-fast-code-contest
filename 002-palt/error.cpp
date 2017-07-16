#include <cmath>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

// Represents an RGB color - we use int for convenience, even though 
// the values are between 0 to 255 inclusive
struct RGB
{
  int r, g, b;
};

// 256 colors representing an indexed palette
vector<RGB> g_palette;

// Reads a RGB from a file with the format
bool getRGB(ifstream& ifs, RGB &rgb)
{
  return bool(ifs >> rgb.r >> rgb.g >> rgb.b);
}

// Given a color, finds which of the 256 in that palette is closest by pythagorean distance
// Saves the absolue error in fErr
void getError(RGB &rgb, float &fDist, int idx)
{
  // Get the differences between the pixel and this palette color for r g and b
  RGB &rgbPal = g_palette[idx];
  int dr = (rgbPal.r - rgb.r);
  int dg = (rgbPal.g - rgb.g);
  int db = (rgbPal.b - rgb.b);
  
  // Use pythagorean distancve formula 
  fDist = sqrtf(dr * dr + dg * dg + db * db);
}


int main(int argc, char **argv)
{
  if(argc < 3)
  {
    cerr << "error <palette-file> <image-file> <result-file>\n"
    "computes the average error in palette mapping" << endl;
    return -1;
  }  
  
  
  // read the palette file, save the 256 colors in g_palette
  ifstream filePalette(argv[1]);
  RGB rgb;
  for(int i = 0; i < 256; ++i)
  {
    getRGB(filePalette, rgb);
    g_palette.push_back(rgb);
  }
  
  // Read the pixel file, and output the best palette color index for each pixel
  ifstream filePixels(argv[2]);
  ifstream fileResults(argv[3]);
  int idx;
  long long int nPixels = 0;
  float fDist;
  double fTotalDist = 0;
  while(bool(fileResults >> idx))
  {
    ++nPixels;
    getRGB(filePixels, rgb);
    getError(rgb, fDist, idx);
    fTotalDist += fDist;
  }
  
  cerr << setprecision(2) << (100.0 * ((fTotalDist / nPixels) / 255))  << "% RGB error" << endl;
}



