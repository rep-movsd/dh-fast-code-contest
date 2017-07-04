#include <cmath>
#include <iomanip>
#include <fstream>
#include <chrono>
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

// Given a color, finds which of the 256 in tha palette is closest by pythagorean distance
// Saves the absolue error in fErr
int getNearestPaletteColor(RGB &rgb, float &fMinDist)
{
  // Min dist is inited to the highest possible i.e. sqrt(255 * 255 * 3) + 1
  fMinDist = 443;
  float fDist;   
  int iIndexMin = 0;
  
  // Scan through the 256 palette colors
  for(int i = 0; i < 256; ++i)
  {
    // Get the differences between the pixel and this palette color for r g and b
    RGB &rgbPal = g_palette[i];
    int dr = (rgbPal.r - rgb.r);
    int dg = (rgbPal.g - rgb.g);
    int db = (rgbPal.b - rgb.b);
    
    // Use pythagorean distancve formula 
    fDist = sqrtf(dr * dr + dg * dg + db * db);
    
    // If this color is the best so far, keep it in mind
    if(fDist < fMinDist)
    {
      fMinDist = fDist;
      iIndexMin = i;
    }
  }
  
  // By now we have (one of) the best color(s) index, return it
  return iIndexMin;
}


int main(int argc, char **argv)
{
  if(argc < 3)
  {
    cerr << "palt <palette-file> <image-file> <output-file>\n"
    "Reads the 256 colors in palette-file and for each pixel in <image-file> "
    "writes the closest palette colors index to output-file " << endl;
    return -1;
  }  
  
  
  // Start timer and run it
  auto tmStart = chrono::high_resolution_clock::now();
  
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
  ofstream fileOut(argv[3]);
  int nPixels = 0;
  float fDist;
  double fTotalDist = 0;
  while(getRGB(filePixels, rgb))
  {
    ++nPixels;
    fileOut << getNearestPaletteColor(rgb, fDist) << endl;
    fTotalDist += fDist;
  }
  
  auto tmElapsed = chrono::high_resolution_clock::now() - tmStart;

  long long nano = chrono::duration_cast<std::chrono::nanoseconds>(tmElapsed).count();
  double ms = nano/1000000.0F;
  cerr << ms << " ms elapsed" << endl;
  cerr << ((double)nano / nPixels)  << " nanoseconds per pixel" << endl;
  
  cerr << setprecision(2) << (100.0 * ((fTotalDist / nPixels) / 255))  << "% RGB error" << endl;
}



