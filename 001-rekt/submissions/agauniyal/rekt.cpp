#include <boost/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/function_output_iterator.hpp>

#include "rekt.hpp"

namespace bg  = boost::geometry;
namespace bgi = boost::geometry::index;


BOOST_GEOMETRY_REGISTER_POINT_2D(RankedPoint, float, cs::cartesian, point.x, point.y)
BOOST_GEOMETRY_REGISTER_POINT_2D(Point, float, cs::cartesian, x, y)
BOOST_GEOMETRY_REGISTER_BOX(Rectangle, Point, min_corner, max_corner)


namespace {

bgi::rtree<RankedPoint, bgi::rstar<16>> rT104;
bgi::rtree<RankedPoint, bgi::rstar<32>> rT105;
bgi::rtree<RankedPoint, bgi::rstar<32>> rT106;
bgi::rtree<RankedPoint, bgi::rstar<32>> rT107;
bgi::rtree<RankedPoint, bgi::rstar<32>> rT108;
bgi::rtree<RankedPoint, bgi::rstar<64>> rT109;
bgi::rtree<RankedPoint, bgi::rstar<64>> rTremain;

std::vector<RankedPoint> rT103;


std::vector<int> ranks;
bool invalidPoints = false;
Float min_X_Point;
Float max_X_Point;
Float min_Y_Point;
Float max_Y_Point;

constexpr auto pow10(const int power)
{
	auto result = 10;
	for (int i = 1; i < power; ++i) {
		result *= 10;
	}
	return power == 0 ? 1 : result;
}

struct Extract_Ranks {
	void operator()(const RankedPoint &p) { ranks.push_back(p.rank); }
} callback;
}


void init(std::vector<RankedPoint> &points)
{
	if (points.size() < 5) {
		invalidPoints = true;
		return;
	}
	points.erase(points.end() - 4, points.end());

	std::sort(points.begin(), points.end(),
	          [](const RankedPoint &p1, const RankedPoint &p2) { return p1.point.x < p2.point.x; });

	min_X_Point = points[0].point.x;
	max_X_Point = points[points.size() - 1].point.x;

	std::sort(points.begin(), points.end(),
	          [](const RankedPoint &p1, const RankedPoint &p2) { return p1.point.y < p2.point.y; });

	min_Y_Point = points[0].point.y;
	max_Y_Point = points[points.size() - 1].point.y;

	std::sort(points.begin(), points.end(),
	          [](const RankedPoint &p1, const RankedPoint &p2) { return p1.rank < p2.rank; });

	rT103.reserve(2000);
	if (points.size() < 2000) {
		rT103.insert(rT103.begin(), points.begin(), points.end());
		points.clear();
	} else {
		rT103.insert(rT103.begin(), points.begin(), points.begin() + 2000);
		points.erase(points.begin(), points.begin() + 2000);
		points.shrink_to_fit();
	}

	if (points.size() < pow10(4)) {
		bgi::rtree<RankedPoint, bgi::rstar<16>> tempRT104(points);
		rT104 = std::move(tempRT104);
		points.clear();
	} else {
		bgi::rtree<RankedPoint, bgi::rstar<16>> tempRT104(points.begin(), points.begin() + pow10(4));
		rT104 = std::move(tempRT104);
		points.erase(points.begin(), points.begin() + pow10(4));
		points.shrink_to_fit();
	}


	if (points.size() < pow10(5)) {
		bgi::rtree<RankedPoint, bgi::rstar<32>> tempRT105(points);
		rT105 = std::move(tempRT105);
		points.clear();
	} else {
		bgi::rtree<RankedPoint, bgi::rstar<32>> tempRT105(points.begin(), points.begin() + pow10(5));
		rT105 = std::move(tempRT105);
		points.erase(points.begin(), points.begin() + pow10(5));
		points.shrink_to_fit();
	}


	if (points.size() < pow10(6)) {
		bgi::rtree<RankedPoint, bgi::rstar<32>> tempRT106(points);
		points.clear();
		rT106 = std::move(tempRT106);
	} else {
		bgi::rtree<RankedPoint, bgi::rstar<32>> tempRT106(points.begin(), points.begin() + pow10(6));
		rT106 = std::move(tempRT106);
		points.erase(points.begin(), points.begin() + pow10(6));
		points.shrink_to_fit();
	}


	if (points.size() < pow10(7)) {
		bgi::rtree<RankedPoint, bgi::rstar<32>> tempRT107(points);
		rT107 = std::move(tempRT107);
		points.clear();
	} else {
		bgi::rtree<RankedPoint, bgi::rstar<32>> tempRT107(points.begin(), points.begin() + pow10(7));
		rT107 = std::move(tempRT107);
		points.erase(points.begin(), points.begin() + pow10(7));
		points.shrink_to_fit();
	}


	if (points.size() < pow10(8)) {
		bgi::rtree<RankedPoint, bgi::rstar<32>> tempRT108(points);
		rT108 = std::move(tempRT108);
		points.clear();
	} else {
		bgi::rtree<RankedPoint, bgi::rstar<32>> tempRT108(points.begin(), points.begin() + pow10(8));
		rT108 = std::move(tempRT108);
		points.erase(points.begin(), points.begin() + pow10(8));
		points.shrink_to_fit();
	}


	if (points.size() < pow10(9)) {
		bgi::rtree<RankedPoint, bgi::rstar<64>> tempRT109(points);
		rT109 = std::move(tempRT109);
		points.clear();
	} else {
		bgi::rtree<RankedPoint, bgi::rstar<64>> tempRT109(points.begin(), points.begin() + pow10(9));
		rT109 = std::move(tempRT109);
		points.erase(points.begin(), points.begin() + pow10(9));
		points.shrink_to_fit();
	}


	if (points.size() > 0) {
		bgi::rtree<RankedPoint, bgi::rstar<64>> tempRTremain(points);
		rTremain = std::move(tempRTremain);
		points.clear();
		points.shrink_to_fit();
	}
}


std::vector<std::vector<int>> run(std::vector<Rectangle> &rectangles)
{
	std::vector<std::vector<int>> ranksVec;
	ranksVec.reserve(rectangles.size());

	if (rectangles.size() == 0 || invalidPoints) {
		return ranksVec;
	}

	Float min_X_Rect = rectangles[0].min_corner.x;
	Float max_X_Rect = rectangles[0].max_corner.x;
	Float min_Y_Rect = rectangles[0].min_corner.y;
	Float max_Y_Rect = rectangles[0].max_corner.y;

	for (const auto &r : rectangles) {
		min_X_Rect = r.min_corner.x < min_X_Rect ? r.min_corner.x : min_X_Rect;
		max_X_Rect = r.max_corner.x > max_X_Rect ? r.max_corner.x : max_X_Rect;
		min_Y_Rect = r.min_corner.y < min_Y_Rect ? r.min_corner.y : min_Y_Rect;
		max_Y_Rect = r.max_corner.y > max_Y_Rect ? r.max_corner.y : max_Y_Rect;
	}


	for (const auto &r : rectangles) {
		ranks.clear();

		if (r.min_corner.x > max_X_Point || r.min_corner.y > max_Y_Point || r.max_corner.x < min_X_Point
		    || r.max_corner.y < min_Y_Point) {
			ranksVec.emplace_back(ranks);
			continue;
		}

		int ranksGathered = 0;
		for (const auto &rp : rT103) {
			if (rp.point.y >= r.min_corner.y && rp.point.y <= r.max_corner.y && rp.point.x >= r.min_corner.x
			    && rp.point.x <= r.max_corner.x) {
				ranks.push_back(rp.rank);
				++ranksGathered;
				if (ranksGathered >= 20) break;
			}
		}

		if (ranks.size() < 20) {
			rT104.query(bgi::covered_by(r), boost::make_function_output_iterator(callback));
		}
		if (ranks.size() < 20) {
			rT105.query(bgi::covered_by(r), boost::make_function_output_iterator(callback));
		}
		if (ranks.size() < 20) {
			rT106.query(bgi::covered_by(r), boost::make_function_output_iterator(callback));
		}
		if (ranks.size() < 20) {
			rT107.query(bgi::covered_by(r), boost::make_function_output_iterator(callback));
		}
		if (ranks.size() < 20) {
			rT108.query(bgi::covered_by(r), boost::make_function_output_iterator(callback));
		}
		if (ranks.size() < 20) {
			rT109.query(bgi::covered_by(r), boost::make_function_output_iterator(callback));
		}
		if (ranks.size() < 20) {
			rTremain.query(bgi::covered_by(r), boost::make_function_output_iterator(callback));
		}
		ranksVec.emplace_back(ranks);
	}

	return ranksVec;
}


void results(std::vector<std::vector<int>> &ranksVec)
{
	for (auto &ranks : ranksVec) {
		std::sort(ranks.begin(), ranks.end());
		if (ranks.size() > 20) {
			ranks.resize(20);
		}
	}
}
