#include <dlfcn.h>

#include <cstdlib>

#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <chrono>
#include <string>
#include <streambuf>


using namespace std;

#pragma pack(push)
#pragma pack(1)
struct TRect 
{
  float lx;
  float ly;
  float hx;
  float hy;
};
#pragma pack(pop)

typedef void (*TFnInit)(const char *pszFileName);
typedef void (*TFnRun)(const TRect* pRects, size_t nRects);
typedef void (*TFnResults)(char *pBuf);


// Assume 20 MB buffer is enough
char g_pcOutBuf[20 * 1048576];

int main(int argc, char *argv[])
{
  if (argc < 4) 
  {
    cout << 
    "Usage : " << argv[0] << " <GUID> <sofile> <resultsfile>\n"
    "\n"
    "<sofile> is a linux shared object file that contains the following three functions: ;\n"
    "    void init(const char *pszFileName);\n"
    "    void run(const TRect* pRects, size_t nRects);\n"
    "    void results(char *pOutBuf);\n"
    "\n"
    "    TRect is defined as a packed struct: \n"
    "        struct TRect \n"
    "        {\n"
    "            float lx;\n"
    "            float ly;\n"
    "            float hx;\n"
    "            float hy;\n"
    "        };;\n"
    "    floats are standard 32 bits;\n"
    "\n"
    "Make sure <GUID>-points.txt, <GUID>-rects.txt  exist in current directory.\n"
    "resultsfile is the full pathname of the results file to compare against : Specify - to load ./<GUID>-results.txt\n"
    "\n"
    "init() should load the points from <GUID>-points.txt \n"
    "run() should calculate the results\n"
    "results() should write the results into the supplied buffer as a plain NUL terminated string in the same format as <GUID>-results.txt\n"
    "run() is timed, but init() and results() are not timed\n";
    
    return -1;
  }
  
  string sGUID = argv[1];
  string sFileRects = sGUID + "-rects.txt";
  string sFilePoints = sGUID + "-points.txt";
  string sFileSO = argv[2];
  string sFileResults = argv[3];
  
  if(sFileResults == "-")
  {
    sFileResults = sGUID + "-results.txt";
  }
  
  // Load the .so and get the functions
  void *pSO = dlopen(argv[2], RTLD_NOW);
  TFnInit init = (TFnInit)dlsym(pSO, "init");
  TFnRun run = (TFnRun)dlsym(pSO, "run");
  TFnResults results = (TFnResults)dlsym(pSO, "results");
    
  init(sFilePoints.c_str());
  
  vector<TRect> arrRects;
  ifstream fileRect(sFileRects);
  TRect rect;
  while(fileRect >> rect.lx >> rect.hx >> rect.ly >> rect.hy)
  {
    arrRects.push_back(rect);
  }
  
  size_t nRects = arrRects.size();
  
  // Start timer and run it
  auto tmStart = chrono::high_resolution_clock::now();
  run(&arrRects[0], nRects);
  auto tmElapsed = chrono::high_resolution_clock::now() - tmStart;

  // get results
  results(g_pcOutBuf);
  cerr << "Checking results..." << endl;

  ifstream fileRes(sFileResults);
  string sTest((istreambuf_iterator<char>(fileRes)), istreambuf_iterator<char>());
  string sResults(g_pcOutBuf);

  if(sTest == sResults)
  {
    cerr << "Results OK" << endl;
    long long microseconds = chrono::duration_cast<std::chrono::microseconds>(tmElapsed).count();
    long ms = microseconds/1000;
    cerr << microseconds/1000 << " ms elapsed" << endl;
    cerr << float(ms)/nRects << " ms per rect" << endl;
  }
  else
  {
    cerr << "Results NOT OK" << endl;
  }
  
  return 0;
}
