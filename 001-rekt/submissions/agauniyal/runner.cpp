#include <iostream>
#include <chrono>
#include <sstream>
#include <iterator>
#include <fstream>

#include "rekt.hpp"

using namespace std::chrono;


bool readPoints(const char path[], std::vector<RankedPoint> &points)
{
	std::string guid(path);
	std::ifstream input(guid + "-points.txt");
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
	std::string guid(path);
	std::ifstream input(guid + "-rects.txt");
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

		if (x1 < x2) {
			rectangles.emplace_back(x1, y1, x2, y2);
		} else {
			rectangles.emplace_back(x2, y2, x1, y1);
		}
	}
	return true;
}


bool verify(const char path[], std::vector<std::vector<int>> &ranks)
{
	std::string guid(path);
	std::ifstream input(guid + "-results.txt");
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


void writeResults(const char path[], std::vector<std::vector<int>> &ranks)
{
	std::string guid(path);
	std::ofstream output(guid + "-output.txt");
	if (!output) {
		std::cerr << "Writing points failed\n";
	}

	for (auto &v : ranks) {
		for (auto &r : v) {
			output << r << ' ';
		}
		output << '\n';
	}
}


int main(int argc, char *argv[])
{
	if (argc != 2) {
		std::cerr << "GUID missing in arguments.\n";
		return 1;
	}

	std::vector<RankedPoint> points;
	std::vector<Rectangle> rectangles;

	const auto resp = readPoints(argv[1], points);
	const auto resr = readRectangles(argv[1], rectangles);

	if (!resp || !resr) {
		return 1;
	}

	// Starting execution of user supplied code

	init(points);

	const auto t1 = high_resolution_clock::now();
	auto ranks    = run(rectangles);
	const auto t2 = high_resolution_clock::now();
	std::cout << duration_cast<microseconds>(t2 - t1).count() << "microseconds\n";

  results(ranks);
  
	writeResults("test", ranks);

}
