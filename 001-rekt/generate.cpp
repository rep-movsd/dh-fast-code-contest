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

using namespace std;

struct Point
{
  int32_t rank;
  float x;
  float y;
};

struct Rect
{
  float lx;
  float ly;
  float hx;
  float hy;
};

typedef unsigned int uint;
static const float pi = 3.141592653589793238462643383279502884197169399375105820974944f;
static float sqr(const float f) { return f * f; }
static inline uint l(const uint t, const uint k) { return t ^ (t << k); }
static inline uint r(const uint t, const uint k) { return t ^ (t >> k); }
struct S_seed { uint x, y, z, w, v; };
static S_seed seed = { 123456789, 362436069, 521288629, 88675123, 886756453, }; 

static void randomize_seed()
{
  time_t t = time(nullptr);
  srand(t);
  seed.x = seed.x ^ rand();
  seed.y = seed.y ^ rand();
  seed.z = seed.z ^ rand();
  seed.w = seed.w ^ rand();
  seed.v = seed.v ^ rand();
}

static uint xrandom() 
{
  const uint n=l(seed.v,6)^l(r(seed.x,7),13);
  seed.x=seed.y; seed.y=seed.z; seed.z=seed.w; seed.w=seed.v; seed.v=n;
  return (seed.y+seed.y+1)*seed.v;
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
    "\t<GUID> is a HEX GUID in the format XXXXXXXX-XXXXXXXX-XXXXXXXX-XXXXXXXX-XXXXXXXX\n"
    "\tSpecify - as GUID to automatically seed the RNG\n\n"
    "Output files are named as <GUID>-points.txt <GUID>-rects.txt and <GUID>-result.txt\n";
    "If R or P is 0 then existing files are not overwritten\n";
    
    return 0;
  }
    
  string sGUID = argv[3];
  int nPoints=stoi(argv[1]);
  int nRects=stoi(argv[2]);
  
  // Parse GUID if amy
  if(sGUID == "-")
  {
    char s[256];
    randomize_seed();
    sprintf(s, "%X-%X-%X-%X-%X", seed.x, seed.y, seed.z, seed.w, seed.v);
    sGUID = s;
  }
  else
  {
    sscanf(sGUID.c_str(), "%X-%X-%X-%X-%X", &seed.x, &seed.y, &seed.z, &seed.w, &seed.v);
  }
   
  printf("Random seed : %X-%X-%X-%X-%X\n", seed.x, seed.y, seed.z, seed.w, seed.v);
    
  const float r = 1e3;
  Rect rcMain = xrandom(-r, +r, -r, +r);
  
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
    
  if(nPoints)
  {
    printf("Generating %u points\n", nPoints);

    Point* arrPoints = new Point[nPoints];
    Point* pPointsBeg = arrPoints;
    Point* pPointsEnd = arrPoints + nPoints;
    
    vector<vector<int>> arrResults(nRects);

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
    
    if(nRects)
    {
      // Reload our own file
      ifstream ifs(sGUID + "-points.txt");
      i = pPointsBeg;
      for(; i != pPointsEnd; ++i)
      {
        ifs >> i->x >> i->y >> i->rank;
      }
      
      auto start = std::chrono::high_resolution_clock::now();
     
      
      // Calculate all the solutions after sorting by rank
      sort(pPointsBeg, pPointsEnd, [](const Point &p1, const Point &p2){return p1.rank < p2.rank;});
      
      int id;
      for(id = 0,i = pPointsBeg; i != pPointsEnd; ++i, ++id)
      {
        
        // See if this point is part of solution for any rect
        for(int n = 0; n < nRects; ++n)
        {
          const auto &rc = arrRects[n];
          if(arrResults[n].size() < 20)
          {
            bool isIn = rc.lx <= i->x && i->x <= rc.hx && rc.ly <= i->y && i->y <= rc.hy;
            if(isIn)
            {
              arrResults[n].push_back(id);
            }
          }
        }
      }
      
      auto elapsed = std::chrono::high_resolution_clock::now() - start;
      long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
      
      long ms = microseconds/1000;
      cout << microseconds/1000 << " ms elapsed" << endl;
      cout << float(ms)/nRects << " ms per rect" << endl;
      
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
