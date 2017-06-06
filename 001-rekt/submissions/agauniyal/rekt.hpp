#ifndef REKT_HPP_HEADER
#define REKT_HPP_HEADER

#include <vector>
#include <algorithm>

using Float = float;

struct Point {
	Float x;
	Float y;
	Point(Float _x, Float _y) : x(_x), y(_y) {}
};

struct Rectangle {
	Point min_corner;
	Point max_corner;
	Rectangle(Point _min, Point _max) : min_corner(_min), max_corner(_max) {}
	Rectangle(Float _x1, Float _y1, Float _x2, Float _y2)
	    : min_corner(_x1, _y1), max_corner(_x2, _y2)
	{
	}
};

struct RankedPoint {
	Point point;
	int rank;
	RankedPoint(Point _p, int _rank) : point(_p), rank(_rank) {}
	RankedPoint(Float _x, Float _y, int _rank) : point(_x, _y), rank(_rank) {}
};

void init(std::vector<RankedPoint> &);

std::vector<std::vector<int>> run(std::vector<Rectangle> &);

void results(std::vector<std::vector<int>> &);
#endif
