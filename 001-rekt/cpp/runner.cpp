#include <iostream>
#include <chrono>
#include <sstream>
#include <iterator>
#include <string>
#include <fstream>

#include "rekt.hpp"

using namespace std::chrono;


bool readPoints(const char path[], std::vector<RankedPoint> &points)
{
	std::ifstream input(path);
	if (!input) {
		std::cerr << "Reading points failed\n";
		return false;
	}

	Float x, y;
	int rank;
	while (input >> x >> y >> rank) {
		if (input.fail()) {
			std::cerr << "Reading points failed\n";
			return false;
		}
		points.emplace_back(x, y, rank);
	}
	return true;
}


bool readRectangles(const char path[], std::vector<Rectangle> &rectangles)
{
	std::ifstream input(path);
	if (!input) {
		std::cerr << "Reading rectangles failed\n";
		return false;
	}

	Float x1, x2, y1, y2;
	while (input >> x1 >> x2 >> y1 >> y2) {
		if (input.fail()) {
			std::cerr << "Reading rectangles failed\n";
			return false;
		}
		rectangles.emplace_back(x1, y1, x2, y2);
	}
	return true;
}


bool verify(const char path[], std::vector<std::vector<int>> &ranks)
{
	std::ifstream input(path);
	if (!input) {
		std::cerr << "Reading points failed\n";
		return false;
	}

	std::vector<std::vector<int>> data;
	std::vector<int> temp;

	std::string line;
	while (std::getline(input, line)) {
		std::istringstream is(line);
		temp.clear();
		std::copy(std::istream_iterator<int>(is), std::istream_iterator<int>(),
		          std::back_inserter(temp));
		data.push_back(temp);
	}
	return data == ranks;
}

int main(int argc, char *argv[])
{
	if (argc != 4) {
		std::cerr << "Arguments: <points.txt> <rects.txt> <results.txt>\n";
		return 1;
	}

	std::vector<RankedPoint> points;
	std::vector<Rectangle> rectangles;

	const auto resp = readPoints(argv[1], points);
	const auto resr = readRectangles(argv[2], rectangles);

	if (!resp || !resr) {
		return 1;
	}

	// Starting execution of user supplied code

	init(points);

	const auto t1 = high_resolution_clock::now();
	auto ranks    = run(rectangles, points);
	const auto t2 = high_resolution_clock::now();
	std::cout << duration_cast<microseconds>(t2 - t1).count() << std::endl;

	results(ranks);

	// Ending execution of user supplied code

	if (verify(argv[3], ranks)) {
		return 0;
	} else {
		std::cerr << "Data doesn't matches with results.txt\n";
		return 1;
	}
}
