// gcc -O3 -shared -o librekt.so -fPIC skeleton.cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cassert>
#include <climits>

using namespace std;

struct TPoint
{
  int rank;
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

const int MAGIC = 20000;
vector<TPoint> arrPts;
vector<vector<int>> res;
int N;
int *X, *Y;
vector<vector<pair<int, int> > > X_list, Y_list;
vector<float> X_map;
vector<float> Y_map;

// quadtree node
struct node {
  node *UL;  // upper left
  node *UR;  // upper right
  node *DL;  // bottom left
  node *DR;  // bottom right
  vector<int> A;
  vector<pair<int, pair<int, int> > > all;
};

node* root;

void insert_into_arr(vector<int>& A, int r) {
  A.push_back(r);
  sort(A.begin(), A.end());
  if(A.size() > 20) A.pop_back();
}

int count(int X_L, int X_R, int Y_L, int Y_R) {
  int ans = 0;
  if((X_R - X_L) <= (Y_R - Y_L)) {
    for(int i = X_L; i <= X_R; i++) {
      ans += upper_bound(X_list[i].begin(), X_list[i].end(), make_pair(Y_R + 1, 0)) - lower_bound(X_list[i].begin(), X_list[i].end(), make_pair(Y_L, 0));
    }
  }
  else {
    for(int i = Y_L; i <= Y_R; i++) {
      ans += upper_bound(Y_list[i].begin(), Y_list[i].end(), make_pair(X_R + 1, 0)) - lower_bound(Y_list[i].begin(), Y_list[i].end(), make_pair(X_L, 0));
    }
  }
  return ans;
}

void build(node*& root, int X_L, int X_R, int Y_L, int Y_R) {
  if(X_L > X_R) return;
  if(Y_L > Y_R) return;
  
  if(root == NULL) root = new node();
  
  if(count(X_L, X_R, Y_L, Y_R) <= MAGIC) {
    for(int i = X_L; i <= X_R; i++) {
      auto lim = upper_bound(X_list[i].begin(), X_list[i].end(), make_pair(Y_R + 1, 0));
      for(auto it = lower_bound(X_list[i].begin(), X_list[i].end(), make_pair(Y_L, 0)); it != lim; it++) {
        root->all.push_back({arrPts[it->second].rank, {i, it->first}});
      }
    }
    sort(root->all.begin(), root->all.end());
    for(int i = 0; i < min(20, (int)root->all.size()); i++) {
      root->A.push_back(root->all[i].first);
    }
    return;
  }
  
  if(X_L == X_R && Y_L == Y_R) return;
  
  int mid_x = (X_L + X_R)/2;
  int mid_y = (Y_L + Y_R)/2;
  
  build(root->UL, X_L, mid_x, Y_L, mid_y);
  build(root->UR, X_L, mid_x, mid_y + 1, Y_R);
  build(root->DL, mid_x + 1, X_R, Y_L, mid_y);
  build(root->DR, mid_x + 1, X_R, mid_y + 1, Y_R);
  
  vector<pair<int, pair<int, int> > > tmp_A(root->UL->all.size() + root->UR->all.size());
  vector<pair<int, pair<int, int> > > tmp_B(root->DL->all.size() + root->DR->all.size());
  merge(root->UL->all.begin(), root->UL->all.end(), root->UR->all.begin(), root->UR->all.end(), tmp_A.begin());
  merge(root->DL->all.begin(), root->DL->all.end(), root->DR->all.begin(), root->DR->all.end(), tmp_B.begin());
  root->all.resize(tmp_A.size() + tmp_B.size());
  merge(tmp_A.begin(), tmp_A.end(), tmp_B.begin(), tmp_B.end(), root->all.begin());
  
  for(int i = 0; i < min(20, (int)root->all.size()); i++) {
    root->A.push_back(root->all[i].first);
  }
}

extern "C" void init(const char *pszFileName)
{
  TPoint pt;
  ifstream ifs(pszFileName);
  while(ifs >> pt.x >> pt.y >> pt.rank)
  {
    arrPts.push_back(pt);
    X_map.push_back(pt.x);
    Y_map.push_back(pt.y);
  }
  
  N = arrPts.size();
  X = new int[N];
  Y = new int[N];
  
  sort(X_map.begin(), X_map.end());
  X_map.resize(unique(X_map.begin(), X_map.end()) - X_map.begin());
  
  sort(Y_map.begin(), Y_map.end());
  Y_map.resize(unique(Y_map.begin(), Y_map.end()) - Y_map.begin());
  
  for(int i = 0; i < N; i++) {
    X[i] = lower_bound(X_map.begin(), X_map.end(), arrPts[i].x) - X_map.begin();
    if(X[i] == X_map.size()) assert(false);
    
    Y[i] = lower_bound(Y_map.begin(), Y_map.end(), arrPts[i].y) - Y_map.begin();
    if(Y[i] == Y_map.size()) assert(false);
  }
  
  X_list.resize(X_map.size());
  Y_list.resize(Y_map.size());
  
  for(int i = 0; i < N; i++) {
    X_list[X[i]].push_back({Y[i], i});
    Y_list[Y[i]].push_back({X[i], i});
  }
  
  for(int i = 0; i < X_map.size(); i++) {
    sort(X_list[i].begin(), X_list[i].end());
  }
  for(int i = 0; i < Y_map.size(); i++) {
    sort(Y_list[i].begin(), Y_list[i].end());
  }
  
  res.reserve(10000);
  
  cerr << X_map.size() << endl;
  cerr << Y_map.size() << endl;
  cerr << "Building Tree Starts" << endl;
  build(root, 0, X_map.size() - 1, 0, Y_map.size() - 1);
}

vector<int> query(node*& root, int X_L, int X_R, int Y_L, int Y_R, int q_x_l, int q_x_r, int q_y_l, int q_y_r) {
  if(root == NULL
    || X_L > q_x_r || X_R < q_x_l
    || Y_L > q_y_r || Y_R < q_y_l) return {};
  
  if(q_x_l <= X_L && X_R <= q_x_r
    && q_y_l <= Y_L && Y_R <= q_y_r) return root->A;
  
  if(root->all.size() <= MAGIC) {
    vector<int> curr;
    curr.reserve(20);
    for(auto& it: root->all) {
      int r = it.first, x = it.second.first, y = it.second.second;
      if((x >= q_x_l) & (x <= q_x_r) && (y >= q_y_l) & (y <= q_y_r)) {
        curr.push_back(r);
        if(curr.size() == 20) break;
      }
    }
    return curr;
  }
  
  int mid_x = (X_L + X_R)/2;
  int mid_y = (Y_L + Y_R)/2;
  
  vector<int> res_0 = query(root->UL, X_L, mid_x, Y_L, mid_y, q_x_l, q_x_r, q_y_l, q_y_r);
  vector<int> res_1 = query(root->UR, X_L, mid_x, mid_y + 1, Y_R, q_x_l, q_x_r, q_y_l, q_y_r);
  vector<int> res_2 = query(root->DL, mid_x + 1, X_R, Y_L, mid_y, q_x_l, q_x_r, q_y_l, q_y_r);
  vector<int> res_3 = query(root->DR, mid_x + 1, X_R, mid_y + 1, Y_R, q_x_l, q_x_r, q_y_l, q_y_r);
  
  int sz = min(int(res_0.size() + res_1.size() + res_2.size() + res_3.size()), 20);
  vector<int> res(sz);
  int ind[4] = {0, 0, 0, 0}, j = 0;
  while(j < sz) {
    pair<int, int> tmp = {INT_MAX, 0};
    if(ind[0] < res_0.size() && res_0[ind[0]] < tmp.first) tmp = {res_0[ind[0]], 0};
    if(ind[1] < res_1.size() && res_1[ind[1]] < tmp.first) tmp = {res_1[ind[1]], 1};
    if(ind[2] < res_2.size() && res_2[ind[2]] < tmp.first) tmp = {res_2[ind[2]], 2};
    if(ind[3] < res_3.size() && res_3[ind[3]] < tmp.first) tmp = {res_3[ind[3]], 3};
    res[j++] = tmp.first;
    ind[tmp.second]++;
  }
  return res;
}

extern "C" void run(const TRect* pRects, size_t nRects)
{
  cerr << "Answering Queries Starts" << endl;
  res.resize(nRects);
  
  for(int i = 0; i < nRects; i++) {
    if(pRects[i].lx > *X_map.rbegin() || pRects[i].hx < *X_map.begin()
      || pRects[i].ly > *Y_map.rbegin() || pRects[i].hy < *Y_map.begin()) {
      continue;
      }
      
      int x_l = lower_bound(X_map.begin(), X_map.end(), pRects[i].lx) - X_map.begin();
    int x_r = lower_bound(X_map.begin(), X_map.end(), pRects[i].hx) - X_map.begin();
    
    int y_l = lower_bound(Y_map.begin(), Y_map.end(), pRects[i].ly) - Y_map.begin();
    int y_r = lower_bound(Y_map.begin(), Y_map.end(), pRects[i].hy) - Y_map.begin();
    
    x_r--;
    y_r--;
    
    if(x_l > x_r || y_l > y_r) continue;
    
    res[i] = query(root, 0, X_map.size() - 1, 0, Y_map.size() - 1, x_l, x_r, y_l, y_r);
  }
}

// Dump the results to the output
extern "C" void results(char *pBuf)
{
  ostringstream oss;
  for(int n = 0; n < res.size(); ++n)
  {
    for(const auto &it: res[n])
    {
      oss << it << " ";
    }
    
    oss << endl;
  }
  
  strcpy(pBuf, oss.str().c_str());
}
