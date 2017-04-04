# rekt runner for cpp

## Instructions for the writing your cpp implementation

#### All required declarations are under `rekt.hpp` header file.

Your file `rekt.cpp` needs to include `rekt.hpp` and contain these 3 functions:

1. `void init(std::vector<RankedPoint> &points)`
    - You'll get a `vector` of `RankedPoint` as reference parameter, no `const`.

2. `std::vector<std::vector<int>> run(std::vector<Rectangle> &rectangles,
                                  std::vector<RankedPoint> &points)`
    - You'll get a `vector` of `Rectangle` and the previous points `vector`, both as reference param, no `const`.
	- You'll `return` a `vector<vector<int>>` which will be passed to next method call `results()`.

3. `void results(std::vector<std::vector<int>> &ranksVec)`
    - You'll get a `vector<vector<int>>` which must contain correct ranks for each rectangles.
	- Although you can do everything in `run()` function as well but this method is for offloading formatting, permuting, deletion tasks. Note that you cannot add new elements or use some other clever tricks inside this function, keep them for `run()`.

## Instructions for testing/running your python module

    g++ runner.cpp rekt.cpp -std=c++11 -o runner.app
	./runner.app points.txt rects.txt results.txt

The output is either an error message or number of microseconds `run()` took or a combination of both.
