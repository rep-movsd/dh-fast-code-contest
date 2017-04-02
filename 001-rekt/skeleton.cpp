// gcc -O3 -shared -o librekt.so -fPIC skeleton.cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstring>
using namespace std;

#pragma pack(push)

#pragma pack(1)
struct TPoint
{
  int32_t rank;
  float x;
  float y;
};

struct TRect
{
  float lx;
  float ly;
  float hx;
  float hy;
};

#pragma pack(pop)

vector<TPoint> arrPts;
vector<vector<int>> arrResults;


extern "C" void init(const char *pszFileName)
{
  cerr << "Loading points from "  << pszFileName << endl;
  
  TPoint pt;
  ifstream ifs(pszFileName);
  while(ifs >> pt.x >> pt.y >> pt.rank)
  {
    arrPts.push_back(pt);
  }
  
  cerr << "Loaded " << arrPts.size() << " points" << endl;
  
  cerr << "Sorting...";
  sort(begin(arrPts), end(arrPts), [](const TPoint &p1, const TPoint &p2){return p1.rank < p2.rank;});
  cerr << " done!" << endl;
  
  arrResults.reserve(10000);
}

extern "C" void run(const TRect* pRects, size_t nRects)
{
  cerr << "Processing ..." << endl;
  arrResults.resize(nRects);

  auto pPointsBeg = begin(arrPts), i = pPointsBeg;
  auto pPointsEnd = end(arrPts);
  
  for(int id = 0; i != pPointsEnd; ++i, ++id)
  {
    
    // See if this point is part of solution for any rect
    for(int n = 0; n < nRects; ++n)
    {
      const auto &rc = pRects[n];
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
  
  cerr << "Done ..." << endl;
}

extern "C" void results(char *pBuf)
{
  ostringstream oss;
  for(int n = 0; n < arrResults.size(); ++n)
  {
    for(const auto &it: arrResults[n])
    {
      oss << it << " ";
    }
    
    oss << endl;
  }
  
  strcpy(pBuf, oss.str().c_str());
}
