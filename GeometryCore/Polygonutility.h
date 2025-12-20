#pragma once
#include <vector>

struct Point {
	double x, y;
};

struct Ring {
	std::vector<Point> vertices; // closed loop
};

struct Polygon {
	Ring outer;
	std::vector<Ring> holes;
};

class Polygonutility
{
public:
	bool PointInRing(const Point& p, const Ring& r);

	bool PointInPolygon(const Point& p, const Polygon& poly);

	bool OnSegment(const Point& a, const Point& b, const Point& p);

	bool SegmentIntersect(
		const Point& p1, const Point& p2,
		const Point& q1, const Point& q2,
		Point& ip);

	bool PolygonsOverlap(
		const Polygon& A,
		const Polygon& B);

	void CollectIntersectionPoints(
		const Polygon& A,
		const Polygon& B,
		std::vector<Point>& outPts);

	bool SamePoint(const Point& a, const Point& b);

	void RemoveDuplicates(std::vector<Point>& pts);

	Point ComputeCentroid(const std::vector<Point>& pts);

	void SortCCW(std::vector<Point>& pts);

	bool Inside(const Point& p, const Point& a, const Point& b, bool clipCCW);

	std::vector<Point> ClipPolygon(
		const std::vector<Point>& subject,
		const std::vector<Point>& clip,
		Polygonutility& util);

	std::vector<Point> ClipPolygonOutside(
		const std::vector<Point>& subject,
		const std::vector<Point>& clip,
		Polygonutility& util);
};