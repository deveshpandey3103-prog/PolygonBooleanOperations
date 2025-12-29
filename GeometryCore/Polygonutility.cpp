#include "pch.h"
#include "Polygonutility.h"
#include <cmath>
#include <algorithm>

bool Polygonutility::PointInRing(const Point& p, const Ring& r)
{
	bool inside = false;
	int n = r.vertices.size();

	for (int i = 0, j = n - 1; i < n; j = i++) {
		auto& a = r.vertices[i];
		auto& b = r.vertices[j];

		if ((a.y > p.y) != (b.y > p.y)) {
			double x = (b.x - a.x) * (p.y - a.y) / (b.y - a.y) + a.x;
			if (p.x < x)
				inside = !inside;
		}
	}
	return inside;
}

bool Polygonutility::PointInPolygon(const Point& p, const Polygon& poly)
{
	//  Outside outer boundary → NOT inside polygon
	if (!PointInRing(p, poly.outer))
		return false;

	//  Inside any hole → NOT inside polygon
	for (const Ring& hole : poly.holes)
	{
		if (PointInRing(p, hole))
			return false;
	}

	// Inside outer & outside all holes → inside
	return true;
}


bool Polygonutility::OnSegment(const Point& a, const Point& b, const Point& p)
{
	return p.x >= std::min(a.x, b.x) - 1e-9 &&
		p.x <= std::max(a.x, b.x) + 1e-9 &&
		p.y >= std::min(a.y, b.y) - 1e-9 &&
		p.y <= std::max(a.y, b.y) + 1e-9;
}

bool Polygonutility::SegmentIntersect(const Point& p1, const Point& p2, 
									const Point& q1, const Point& q2, Point& ip)
{
	double A1 = p2.y - p1.y;
	double B1 = p1.x - p2.x;
	double C1 = A1 * p1.x + B1 * p1.y;

	double A2 = q2.y - q1.y;
	double B2 = q1.x - q2.x;
	double C2 = A2 * q1.x + B2 * q1.y;

	double det = A1 * B2 - A2 * B1;
	if (fabs(det) < 1e-9) return false;

	ip.x = (B2 * C1 - B1 * C2) / det;
	ip.y = (A1 * C2 - A2 * C1) / det;

	return OnSegment(p1, p2, ip) && OnSegment(q1, q2, ip);
}

bool Polygonutility::PolygonsOverlap(const Polygon& A, const Polygon& B)
{
	//  Any vertex of A inside B
	for (const Point& p : A.outer.vertices)
		if (PointInPolygon(p, B))
			return true;

	//  Any vertex of B inside A
	for (const Point& p : B.outer.vertices)
		if (PointInPolygon(p, A))
			return true;

	//  Edge-edge intersection (MISSING PART)
	const auto& aV = A.outer.vertices;
	const auto& bV = B.outer.vertices;

	for (size_t i = 0; i < aV.size(); i++)
	{
		Point a1 = aV[i];
		Point a2 = aV[(i + 1) % aV.size()];

		for (size_t j = 0; j < bV.size(); j++)
		{
			Point b1 = bV[j];
			Point b2 = bV[(j + 1) % bV.size()];

			Point ip;
			if (SegmentIntersect(a1, a2, b1, b2, ip))
				return true;
		}
	}

	return false;
}

void Polygonutility::CollectIntersectionPoints(
	const Polygon& A,
	const Polygon& B,
	std::vector<Point>& outPts)
{
	// A vertices inside B
	for (const Point& p : A.outer.vertices)
	{
		if (PointInPolygon(p, B))
			outPts.push_back(p);
	}

	//  B vertices inside A
	for (const Point& p : B.outer.vertices)
	{
		if (PointInPolygon(p, A))
			outPts.push_back(p);
	}

	//  Edge–edge intersections
	const auto& aV = A.outer.vertices;
	const auto& bV = B.outer.vertices;

	for (size_t i = 0; i < aV.size(); i++)
	{
		Point a1 = aV[i];
		Point a2 = aV[(i + 1) % aV.size()];

		for (size_t j = 0; j < bV.size(); j++)
		{
			Point b1 = bV[j];
			Point b2 = bV[(j + 1) % bV.size()];

			Point ip;
			if (SegmentIntersect(a1, a2, b1, b2, ip))
				outPts.push_back(ip);
		}
	}
}

bool Polygonutility::SamePoint(const Point& a, const Point& b)
{
	double eps = 1e-6;
	return std::fabs(a.x - b.x) < eps &&
		std::fabs(a.y - b.y) < eps;
}

void Polygonutility::RemoveDuplicates(std::vector<Point>& pts)
{
	std::vector<Point> unique;
	for (const Point& p : pts)
	{
		bool found = false;
		for (const Point& u : unique)
		{
			if (SamePoint(p, u))
			{
				found = true;
				break;
			}
		}
		if (!found)
			unique.push_back(p);
	}
	pts = unique;
}

Point Polygonutility::ComputeCentroid(const std::vector<Point>& pts)
{
	Point c{ 0, 0 };
	for (const Point& p : pts)
	{
		c.x += p.x;
		c.y += p.y;
	}
	c.x /= pts.size();
	c.y /= pts.size();
	return c;
}

void Polygonutility::SortCCW(std::vector<Point>& pts)
{
	Point c = ComputeCentroid(pts);

	std::sort(pts.begin(), pts.end(),
		[&](const Point& a, const Point& b)
		{
			double angA = atan2(a.y - c.y, a.x - c.x);
			double angB = atan2(b.y - c.y, b.x - c.x);
			return angA < angB;
		});
}

double SignedArea(const std::vector<Point>& pts)
{
	double area = 0.0;
	for (size_t i = 0; i < pts.size(); i++)
	{
		const Point& p = pts[i];
		const Point& q = pts[(i + 1) % pts.size()];
		area += (p.x * q.y - q.x * p.y);
	}
	return area * 0.5;
}

bool IsCCW(const std::vector<Point>& pts)
{
	return SignedArea(pts) > 0;
}

bool Polygonutility::Inside(const Point& p, const Point& a, const Point& b, bool clipCCW)
{
	double cross =
		(b.x - a.x) * (p.y - a.y) -
		(b.y - a.y) * (p.x - a.x);

	// CCW → left side is inside
	// CW  → right side is inside
	return clipCCW ? (cross >= 0) : (cross <= 0);
}

std::vector<Point> Polygonutility::ClipPolygon(const std::vector<Point>& subject, const std::vector<Point>& clip, Polygonutility& util)
{
	std::vector<Point> output = subject;

	bool clipCCW = IsCCW(clip);

	for (size_t i = 0; i < clip.size(); i++)
	{
		Point A = clip[i];
		Point B = clip[(i + 1) % clip.size()];

		std::vector<Point> input = output;
		output.clear();

		if (input.empty())
			break;

		Point S = input.back();

		for (const Point& E : input)
		{
			bool Ein = Inside(E, A, B, clipCCW);
			bool Sin = Inside(S, A, B, clipCCW);

			if (Ein)
			{
				if (!Sin)
				{
					Point ip;
					util.SegmentIntersect(S, E, A, B, ip);
					output.push_back(ip);
				}
				output.push_back(E);
			}
			else if (Sin)
			{
				Point ip;
				util.SegmentIntersect(S, E, A, B, ip);
				output.push_back(ip);
			}

			S = E;
		}
	}
	return output;
}

std::vector<Point> Polygonutility::ClipPolygonOutside(const std::vector<Point>& subject, const std::vector<Point>& clip, Polygonutility& util)
{
	std::vector<Point> output = subject;
	bool clipCCW = IsCCW(clip);

	for (size_t i = 0; i < clip.size(); i++)
	{
		Point A = clip[i];
		Point B = clip[(i + 1) % clip.size()];

		std::vector<Point> input = output;
		output.clear();

		if (input.empty())
			break;

		Point S = input.back();

		for (const Point& E : input)
		{
			bool Ein = Inside(E, A, B, clipCCW);
			bool Sin = Inside(S, A, B, clipCCW);

			// KEEP OUTSIDE instead of inside
			if (!Ein)
			{
				if (Sin)
				{
					Point ip;
					util.SegmentIntersect(S, E, A, B, ip);
					output.push_back(ip);
				}
				output.push_back(E);
			}
			else if (!Sin)
			{
				Point ip;
				util.SegmentIntersect(S, E, A, B, ip);
				output.push_back(ip);
			}

			S = E;
		}
	}
	return output;
}
