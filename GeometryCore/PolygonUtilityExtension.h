#pragma once
#include <vector>
#include "Polygonutility.h"


enum class GHOp
{
    Intersection,
    Union,
    DifferenceAB,
    DifferenceBA
};

struct Node
{
    Point p;
    bool isIntersection = false;
    bool entry = false;
    bool visited = false;
    double alpha = 0.0;

    Node* next = nullptr;
    Node* prev = nullptr;
    Node* neighbor = nullptr;
};

class PolygonUtilityExtension
{
public:
    void EnsureCCW(std::vector<Point>& pts);
    std::vector<std::vector<Point>> Compute(
        const std::vector<Point>& A,
        const std::vector<Point>& B,
        GHOp operation);

private:
    // Core steps
    Node* BuildPolygon(const std::vector<Point>& pts);
    void InsertInOrder(Node* startNode, Node* newNode);
    void FindIntersections(Node* A, Node* B);

    void MarkEntryExit(Node* poly, const std::vector<Point>& other, GHOp op, bool isA);
    std::vector<Point> TraceResult(Node* start, GHOp op, bool startOnA);

    // Geometry helpers
    bool SegmentIntersect(
        const Point& p1, const Point& p2,
        const Point& q1, const Point& q2,
        Point& ip, double& alphaP, double& alphaQ);

    bool PointInsidePolygon(const std::vector<Point>& poly, const Point& p);
};
