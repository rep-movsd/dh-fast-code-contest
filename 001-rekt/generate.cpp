//g++ -O3 --std=c++11 -static -static-libgcc -static-libstdc++ -ogenerate generate.cpp

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cmath>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip> 
#include <chrono>
#include <random>
#include <xmmintrin.h>

using namespace std;


struct __attribute__((aligned(16)))  Point
{
  int32_t rank;
  float x;
  float y;
};

struct __attribute__((aligned(16))) Rect
{
  float lx;
  float ly;
  float hx;
  float hy;
};


typedef unsigned int uint;
static const float pi = 3.141592653589793238462643383279502884197169399375105820974944f;
static float sqr(const float f) { return f * f; }

std::mt19937 rnd;

uint xrandom()
{
  return rnd();
}

static float xrandom(const float l, const float h)
{
  const float t = static_cast<float>(xrandom()) / 0x100000000; // [0,+1)
  return  l + t * (h - l);
}

static Rect xrandom(const float lx, const float hx, const float ly, const float hy)
{
  const float cx = xrandom(lx, hx);
  const float cy = xrandom(ly, hy);
  const float sx = (hx - lx) / 2.0f * sqr(xrandom(0.0, 1.0));
  const float sy = (hy - ly) / 2.0f * sqr(xrandom(0.0, 1.0));
  Rect r = { cx - sx, cy - sy, cx + sx, cy + sy };
  return r;
}

int main(int argc, char** argv)
{
  if(argc < 4)
  {
    cout << 
    "Generate random points and rect data set files\n"
    "Usage:"
    "\tgenerate <P> <R> <GUID> \n"
    "\tGenerates P points and R rectangles using GUID as the random seed\n"
    "\t<GUID> is a HEX GUID in the format XXXXXXXX\n"
    "\tSpecify - as GUID to automatically seed the RNG\n\n"
    "Output files are named as <GUID>-points.txt <GUID>-rects.txt and <GUID>-result.txt\n";
    "If R or P is 0 then existing files are not overwritten\n";
    
    return 0;
  }
    
  string sGUID = argv[3];
  int nPoints=stoi(argv[1]);
  int nRects=stoi(argv[2]);
  int seed;
  
  // Parse GUID if amy
  if(sGUID == "-")
  {
    char s[256];
    rnd.seed(time(nullptr));
    seed = rnd();
    
    sprintf(s, "%X", seed);
    sGUID = s;
  }
  else
  {
    sscanf(sGUID.c_str(), "%X", &seed);
  }
  
  rnd.seed(seed);
   
  printf("Random seed : %X\n", seed);
    
  const float r = 1e3;
  Rect rcMain = xrandom(-r, +r, -r, +r);
  
  
    
  if(nPoints)
  {
    printf("Generating %u points\n", nPoints);

    Point* arrPoints = new Point[nPoints];
    Point* pPointsBeg = arrPoints;
    Point* pPointsEnd = arrPoints + nPoints;
    
    vector<vector<int>> arrResults(nRects);
    
    float lx=0, ly=0, hx=0, hy=0;
    int rank = 0;
    int done = 0;
    Point* i = pPointsBeg;
    for(; i != pPointsEnd;)
    {
      // compute origin
      const uint nClusters = xrandom() & 0xffff;
      const float fClusterOrgX = xrandom(rcMain.lx, rcMain.hx);
      const float fClusterOrgY = xrandom(rcMain.ly, rcMain.hy);
      float rr[] = {fClusterOrgX - rcMain.lx, rcMain.hx - fClusterOrgX, fClusterOrgY - rcMain.ly, rcMain.hy - fClusterOrgY};
      sort(rr, rr + 4);
      const float r = rr[0];
      Point* pClusterEnd = i + nClusters;
      pClusterEnd = pClusterEnd > pPointsEnd ? pPointsEnd : pClusterEnd;
      for(; i != pClusterEnd; ++i)
      {
        const float angle = xrandom(0.0, pi);
        const float range = xrandom(0.0, r);

        i->rank = rank++;
        i->x = fClusterOrgX + cos(angle) * range;
        i->y = fClusterOrgY + sin(angle) * range;
      }
    }

    // Modify the last four points to stretch the coordinate range, so that it's even harder to normalize or use a grid efficiently
    if(nPoints >= 4 )
    {
      const float r = 1e10;
      pPointsEnd[-1].x = -r;
      pPointsEnd[-2].x = +r;
      pPointsEnd[-3].y = -r;
      pPointsEnd[-4].y = +r;
    }
    
    // creating a random permutation of ranks:
    for(int i = 0; i < nPoints; ++i)
    {
      const int j = i + xrandom() % (nPoints - i);
      swap(arrPoints[i].rank, arrPoints[j].rank);
    }
    
    // Write the data file - dummy block to close the file
    do
    {
      ofstream ofs(sGUID + "-points.txt");
      i = pPointsBeg;
      for(; i != pPointsEnd; ++i)
      {
        ofs << setprecision(8) << fixed << i->x << " " << i->y << " " << i->rank << "\n";
      }
    }
    while(0);
    
    // reset the seed (with a twist) to ensure you always get the same rects for any number of points
    seed ^= 0xAAAAAAAA;
    rnd.seed(seed);
    
    cerr <<  xrandom() << endl;
    
    vector<Rect> arrRects;
    if(nRects)
    {
      printf("Generating %u rects\n", nRects);
      
      ofstream ofs(sGUID + "-rects.txt");
      for(int i = 0; i < nRects; ++i)
      {
        const Rect &r = xrandom(rcMain.lx, rcMain.hx, rcMain.ly, rcMain.hy);
        arrRects.push_back(r);
        ofs << setprecision(8) << fixed << r.lx << " " << r.hx << " " << r.ly << " " << r.hy << "\n";
      }
    }
    
    if(nRects)
    {
      // Reload our own file
      ifstream ifs(sGUID + "-points.txt");
      i = pPointsBeg;
      for(; i != pPointsEnd; ++i)
      {
        ifs >> i->x >> i->y >> i->rank;
      }
            
      // Calculate all the solutions after sorting by rank
      sort(pPointsBeg, pPointsEnd, [](const Point &p1, const Point &p2){return p1.rank < p2.rank;});
      
      int id;
      for(id = 0,i = pPointsBeg; i != pPointsEnd; ++i, ++id)
      {
        const auto &pt = *i;
        __v4sf v4pt = {pt.x,  pt.x, pt.y,  pt.y};
        
        // See if this point is part of solution for any rect
        for(int n = 0; n < nRects; ++n)
        {
          const auto &rc = arrRects[n];
          __v4sf v4RC = {rc.lx, rc.hx,  rc.ly, rc.hy};
          
          // lxhxlyhy
          // pxpxpypy  >=
          // yynnyynn -> 1010 -> 0x5
          
          if(arrResults[n].size() < 20)
          {
            //bool isIn = rc.lx <= i->x && i->x < rc.hx && rc.ly <= i->y && i->y < rc.hy;
            
            if(0x05 == _mm_movemask_ps(v4pt >= v4RC))
            {
              arrResults[n].push_back(id);
            }
          }
        }
      }
      
      ofstream ofs(sGUID + "-results.txt");
      for(int n = 0; n < nRects; ++n)
      {
        for(const auto &it: arrResults[n])
        {
          ofs << setprecision(8) << fixed << it << " ";
        }
        
        ofs << endl;
      }
    }
  }
    
  return 0;
}
