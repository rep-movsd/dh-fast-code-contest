// gcc -O3 -shared -o librekt.so -fPIC skeleton.cpp

#include <limits.h>
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstring>
#include <utility>
#include <xmmintrin.h>
#include <immintrin.h>
#include "/usr/include/valgrind/callgrind.h"

//#define INLINE
#define INLINE __inline
//#define INLINE __always_inline

#define vec vector
#define vint vector<int>
#define pb push_back
#define eb emplace_back

#define sz(x) int(x.size())
#define beg(x) begin(x)
#define all(x) begin(x), end(x)
#define pbeg(x) &x[0]
#define pend(x) (&x[0] + sz(x))test

#define FOR(I,N) for(int I = 0; I < N; ++I)
#define FORV(I,V) for(int I = 0; I < (int)V.size(); ++I)
#define LOOP(I, J, K) for(int I = J; I < K; ++I)

#define EASY_SIZE 256
#define SLICE 2048

using namespace std;

struct __attribute__((aligned(16))) TPoint 
{
  uint32_t rank;
  float x;
  float y;
};

struct __attribute__((aligned(16))) TRect
{
  float lx;
  float ly;
  float hx;
  float hy;
};

// 32 byte alignment
template<typename T> T *newalign(size_t nElem)
{
  int nBytes = nElem * sizeof(T);
  intptr_t address = (intptr_t)calloc(1, nBytes + 32);
  address += 32;
  address -= address % 32;
  return (T*)address;
}


typedef vec<TPoint> TPoints;

// Points sorted by rank
TPoints arrPtsRank;

// Points sorted by X & Y in several slices each twice the size of the previous
vec<float*> matPtsXX, matPtsYY;

// Same data but corresponding X and Y coords
vec<float*> matPtsXY, matPtsYX;

// Same data but ranks
vec<uint*> matPtsXR, matPtsYR;

vec<int> arrPtsSize;



struct TResultsArray
{
  uint res[20];
  TResultsArray()
  {
    fill(res, res + 20, 0xFFFFFFFF);
  }
  
  INLINE void addResult(uint x)
  {
    #define TEST(b) if (x < res[b-1]) { res[b] = res[b-1]; } else { res[b] = x; return;}
    
    if(x < res[19]) 
    {
      TEST(19); 
      TEST(18); 
      TEST(17); 
      TEST(16);  
      TEST(15); 
      TEST(14); 
      TEST(13); 
      TEST(12); 
      TEST(11); 
      TEST(10); 
      TEST(9);  
      TEST(8); 
      TEST(7); 
      TEST(6); 
      TEST(5);    
      TEST(4); 
      TEST(3); 
      TEST(2); 
      TEST(1);
      res[0] = x;
    }
  }
  
  INLINE void add8(__v8su &ranks)
  {
    addResult(ranks[0]);
    addResult(ranks[1]);
    addResult(ranks[2]);
    addResult(ranks[3]);
    addResult(ranks[4]);
    addResult(ranks[5]);
    addResult(ranks[6]);
    addResult(ranks[7]);
  }
  
  __always_inline uint done()
  {
    return res[19] != 0xFFFFFFFF;
  }
};

struct TResultsHeap
{
public:
 
  uint res[20];
  uint iBeg, iEnd;
  TResultsHeap(): iBeg(), iEnd() 
  {
  }
  
  bool done() 
  {
    return iEnd == 20;
  }
    
  void addSorted(uint x)
  {
    res[iEnd++] = x;
    iBeg = iEnd;
  }
    
  void add(uint x) throw() 
  {
    if(iEnd == 20) 
    {
      if(x < res[iBeg]) 
      {
        pop_heap(res + iBeg, res + iEnd);
        res[19] = x;
        push_heap(res + iBeg, res + iEnd);
      }
    } 
    else 
    {
      res[iEnd++] = x;
      push_heap(res + iBeg, res + iEnd);
    }
  }
  
  void sort() throw()
  {
    sort_heap(res + iBeg, res + iEnd);
    iBeg = iEnd;
  }
};

typedef TResultsHeap TResults;


vector<TResults> arrResults;


// Get the indices of the range [i, j) of floats in sorted array arrX where 
// lx <= arrX[i] and arrX[j] < hx
INLINE void getBoundIters(const float *arr, int n, int &i, int &j, float l, float h)
{
  // Find the first point that has x >= lx and first point that has x >= hx
  auto it1 = lower_bound(arr, arr + n, l);
  auto it2 = lower_bound(it1, arr + n, h);
  
  i = it1 - arr;
  j = it2 - arr;
}


INLINE bool getBestBoundIter(const int i, int &start, int &stop, const TRect &rc)
{
  int iY1, iY2, iX1, iX2;
  getBoundIters(matPtsXX[i], arrPtsSize[i], iX1, iX2, rc.lx, rc.hx);
  getBoundIters(matPtsYY[i], arrPtsSize[i], iY1, iY2, rc.ly, rc.hy);
  
  int nY = iY2 - iY1;
  int nX = iX2 - iX1;
  
  start = nX < nY ? iX1 : iY1;
  stop = nX < nY ? iX2 : iY2;
  
  return nX < nY;
}


INLINE bool easyScan(TResults &ret, const TRect &rc)
{
  __v4sf v4RC = {rc.lx, rc.hx,  rc.ly, rc.hy};
  
  // lxhxlyhy
  // pxpxpypy  >=
  // yynnyynn -> 1010 -> 0x5
  
  // Try the first few ranked points
  for(int i = 0; i < EASY_SIZE; ++i)
  {
    const auto &pt = arrPtsRank[i];
    __v4sf v4pt = {pt.x,  pt.x, pt.y,  pt.y};
    
    if(0x05 == _mm_movemask_ps(v4pt >= v4RC))
    {
      ret.addSorted(pt.rank);
      if(ret.done())
        return true;
    }
  }
  
  return false;
}

INLINE void doNonEight(int nRem, bool useX, const TRect &rc, int iStart, float *ps, uint *ranks, TResults &ret)
{
  auto ll = useX ? rc.ly : rc.lx;
  auto hh = useX ? rc.hy : rc.hx;
  
  for(int k = 0; k < nRem; ++k, ++iStart)
  {
    auto pp = ps[iStart];
    if((pp >= ll) & (pp < hh))
    {
      ret.add(ranks[iStart]);
    }
  }
}



INLINE void runRect(TResults &ret, const TRect &rc)
{
  #define EIGHT(X) X,X,X,X,X,X,X,X,
  __v8sf lx = {EIGHT(rc.lx)};
  __v8sf ly = {EIGHT(rc.ly)};
  __v8sf hx = {EIGHT(rc.hx)};
  __v8sf hy = {EIGHT(rc.hy)};

  
  int iStart, iStop;
  
  // if we dont yet have 20 points go through each mipmap
  for(int i = 0; !ret.done() && i < matPtsXX.size(); ++i)
  {
    bool useX = getBestBoundIter(i, iStart, iStop, rc);
    
    float* ps = useX ? matPtsXY[i] : matPtsYX[i];
    uint* ranks = useX ? matPtsXR[i] : matPtsYR[i];
    int nTotal = iStop - iStart;
    
    if(nTotal < 32)
    {
      doNonEight(nTotal, useX, rc, iStart, ps, ranks, ret);
    }
    else
    {
      int nOddStart = 8 - (iStart % 8);
      doNonEight(nOddStart, useX, rc, iStart, ps, ranks, ret);
      
      iStart += nOddStart;
      int nEights = (iStop - iStart) / 8;
      
      auto l = useX ? ly : lx;
      auto h = useX ? hy : hx;
      
      FOR(j, nEights)
      {
        __v8sf __attribute__((aligned(32))) pp = *((__v8sf*)(ps + iStart));
        __m256 lo = (pp >= l); 
        __m256 hi = (pp < h); 
        
        if(!_mm256_testz_ps(lo, hi))
        {
          auto mask = (pp >= l) & (pp < h); 
          if(mask[0]) ret.add(ranks[iStart+0]);
          if(mask[1]) ret.add(ranks[iStart+1]);
          if(mask[2]) ret.add(ranks[iStart+2]);
          if(mask[3]) ret.add(ranks[iStart+3]);
          if(mask[4]) ret.add(ranks[iStart+4]);
          if(mask[5]) ret.add(ranks[iStart+5]);
          if(mask[6]) ret.add(ranks[iStart+6]);
          if(mask[7]) ret.add(ranks[iStart+7]);
        }
        iStart += 8;
      }
      
      int nOddEnd = (iStop - iStart);
      if(nOddEnd)
      {
        doNonEight(nOddEnd, useX, rc, iStart, ps, ranks, ret);
      }
    }
    
    ret.sort();
  }    
}

extern "C" void run(const TRect* pRects, size_t nRects)
{
  
  CALLGRIND_TOGGLE_COLLECT;
  cerr << "Processing " << nRects << " rects..." << endl; 
 
  arrResults.resize(nRects);
  
  for(int n = 0; n < nRects; ++n)
  {
    easyScan(arrResults[n], pRects[n]);
  }
  
  for(int n = 0; n < nRects; ++n)
  {
    if(!arrResults[n].done())
    {
      runRect(arrResults[n], pRects[n]);
    }
  }

  CALLGRIND_TOGGLE_COLLECT;
}



// Dump the results to the output
extern "C" void results(char *pBuf)
{
  ofstream  ofs("out.txt");
  
  ostringstream oss;
  for(int n = 0; n < arrResults.size(); ++n)
  {
    TResults &result = arrResults[n];
    for(int i = 0; i < result.iEnd; ++i)
    {
      oss << result.res[i] << " ";
      ofs << result.res[i] << " ";
    }
    
    oss << endl;
    ofs << endl;
  }
  
  strcpy(pBuf, oss.str().c_str());
}


// Export this as C style calling convention
extern "C" void init(const char *pszFileName)
{
  TResults t;
  //t.testit();return;
  
  cerr << "Reading ..." << endl;
  
  // Read the points and sort
  TPoint pt;
  ifstream ifs(pszFileName);
  while(ifs >> pt.x >> pt.y >> pt.rank)
  {
    if(arrPtsRank.size() % 1000 == 0)
    {
      cerr << arrPtsRank.size() << " points loaded     " << "\r";
    }
    
    arrPtsRank.eb(pt);
  }
  
  cerr << endl;
  
  
  cerr << "Sorting ..." << endl;
  sort(all(arrPtsRank), [](const TPoint &p1, const TPoint &p2){return p1.rank < p2.rank;});
  
  int n = sz(arrPtsRank);
  
  cerr << "Slicing ..." << endl;
  for(int i = EASY_SIZE, slice = SLICE; i < n; slice = slice * 2)
  {
    // Populate the slices
    TPoints arrPtsX;
    TPoints arrPtsY;
    for(int j = 0; j < slice && i < n; ++i, ++j)
    {
      arrPtsX.eb(arrPtsRank[i]);
    }
    
    arrPtsY = arrPtsX;
    
    // Sort these by X and Y
    sort(all(arrPtsY), [](const TPoint &p1, const TPoint &p2){return p1.y < p2.y;});
    sort(all(arrPtsX), [](const TPoint &p1, const TPoint &p2){return p1.x < p2.x;});
    
    float *arrXX, *arrYY, *arrXY, *arrYX;
    uint *arrXR, *arrYR;
    
    int nPts = arrPtsX.size();
    
    arrXX = newalign<float>(nPts);
    arrXY = newalign<float>(nPts);
    arrYY = newalign<float>(nPts);
    arrYX = newalign<float>(nPts);
    arrXR = newalign<uint>(nPts);
    arrYR = newalign<uint>(nPts);
    
    FOR(k, nPts)
    {
      arrXX[k] = arrPtsX[k].x;
      arrXY[k] = arrPtsX[k].y;
      arrXR[k] = arrPtsX[k].rank;
      
      arrYX[k] = arrPtsY[k].x;
      arrYY[k] = arrPtsY[k].y;
      arrYR[k] = arrPtsY[k].rank;
    }
    
    // Add a slice to each X and Y
    matPtsXX.pb(arrXX);
    matPtsYY.pb(arrYY);
    matPtsXY.pb(arrXY);
    matPtsYX.pb(arrYX);
    matPtsXR.pb(arrXR);
    matPtsYR.pb(arrYR);    
    arrPtsSize.pb(nPts);
  }
  
  cerr << "Init done ..." << endl;
  
  arrResults.reserve(10000);
}
