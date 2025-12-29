#pragma once
#ifndef POLYGON_H
#define POLYGON_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <stdexcept>

namespace PolygonBoolean {

    // Forward declarations
    enum class VertexType;
    class Vertex;
    class Polygon;

    // Point structure
    struct Point {
        double x, y;

        Point() : x(0), y(0) {}
        Point(double x, double y) : x(x), y(y) {}

        bool operator==(const Point& other) const;
        bool operator!=(const Point& other) const;
        Point operator-(const Point& other) const;
        Point operator+(const Point& other) const;

        double distance(const Point& other) const;
        double cross(const Point& other) const;
        double dot(const Point& other) const;
    };

    // Vertex type enumeration
    enum class VertexType {
        NORMAL,
        ENTRY,
        EXIT,
        UNKNOWN
    };

    // Vertex class for linked list representation
    class Vertex {
    public:
        Point point;
        VertexType type;
        Vertex* next;
        Vertex* prev;
        Vertex* neighbor;
        bool intersect;
        bool visited;
        bool entryExitProcessed;
        double alpha; // Parameter for intersection point (0-1)

        Vertex(const Point& p);
        Vertex(const Point& p, double alpha);

        // Copy constructor
        Vertex(const Vertex& other);

        // Assignment operator
        Vertex& operator=(const Vertex& other);

        ~Vertex();

        bool isInside() const;
        bool isIntersection() const { return intersect; }
        void markAsProcessed();
    };

    // Main Polygon class
    class Polygon {
    private:
        Vertex* head;
        mutable bool clockwise;
        mutable bool directionCalculated;

        // Internal helper methods
        void calculateDirection() const;
        bool pointInPolygon(const Point& p) const;
        double crossProduct(const Point& a, const Point& b, const Point& c) const;

        // Intersection methods
        bool lineSegmentIntersection(const Point& p1, const Point& p2,
            const Point& q1, const Point& q2,
            Point& intersection,
            double& t1, double& t2) const;

        // List management
        void insertVertexAfter(Vertex* position, Vertex* newVertex);
        void removeVertex(Vertex* vertex);
        void clearVertices();
        Vertex* copyVertexList() const;

        // Boolean operation helpers
        void findIntersections(Polygon& other);
        void classifyVertices(const Polygon& other, int operationType);
        std::vector<Polygon> extractResultPolygons();

    public:
        // Constructors & Destructor
        Polygon();
        explicit Polygon(const std::vector<Point>& points);
        Polygon(const Polygon& other);
        Polygon(Polygon&& other) noexcept;
        ~Polygon();

        // Assignment operators
        Polygon& operator=(const Polygon& other);
        Polygon& operator=(Polygon&& other) noexcept;

        // Basic operations
        void addPoint(const Point& p);
        void addPoint(double x, double y);
        void clear();
        bool isEmpty() const { return head == nullptr; }
        size_t vertexCount() const;

        // Polygon properties
        std::vector<Point> getPoints() const;
        double area() const;
        bool isClockwise() const;
        bool isConvex() const;
        bool isValid() const;

        // Transformation methods
        void reverse();
        void translate(double dx, double dy);
        void scale(double sx, double sy);
        void rotate(double angleRadians);

        // Query methods
        bool contains(const Point& p) const;
        bool contains(const Polygon& other) const;
        bool intersects(const Polygon& other) const;

        // Boolean operations (static methods)
        static Polygon unionPolygons(const Polygon& A, const Polygon& B);
        static Polygon intersectionPolygons(const Polygon& A, const Polygon& B);
        static Polygon differencePolygons(const Polygon& A, const Polygon& B);
        static Polygon symmetricDifference(const Polygon& A, const Polygon& B);

        // Boolean operations (instance methods)
        Polygon getUnion(const Polygon& other) const;
        Polygon getIntersection(const Polygon& other) const;
        Polygon getDifference(const Polygon& other) const;
        Polygon getSymmetricDifference(const Polygon& other) const;

        // Combine operations (modify current polygon)
        void unionWith(const Polygon& other);
        void intersectWith(const Polygon& other);
        void subtract(const Polygon& other);

        // Utility methods
        std::vector<Polygon> triangulate() const;
        Polygon getConvexHull() const;
        Polygon getOffset(double distance) const;

        // Boundary operations
        Polygon getBoundary() const;
        bool isSimple() const;

        // Debug and validation
        void validate() const;
        std::string toString() const;

        // Static utility methods
        static bool pointsEqual(const Point& a, const Point& b, double epsilon = 1e-10);
        static double pointDistance(const Point& a, const Point& b);
        static bool isPointOnSegment(const Point& p, const Point& a, const Point& b);
        static int pointInPolygon(const Point& p, const std::vector<Point>& polygon);

    private:
        // Friend functions for internal operations
        friend void linkIntersectionVertices(Vertex* v1, Vertex* v2);
        friend bool compareVertices(const Vertex* a, const Vertex* b);
    };

    // Boolean operations wrapper class
    class BooleanOperations {
    public:
        enum Operation {
            UNION,
            INTERSECTION,
            DIFFERENCE,      // A - B
            SYMMETRIC_DIFFERENCE
        };

        // Compute operation and return result as vector of polygons
        static std::vector<Polygon> compute(const Polygon& A, const Polygon& B, Operation op);

        // Compute operation and return as single polygon (first result)
        static Polygon computeSingle(const Polygon& A, const Polygon& B, Operation op);

        // Compute operation on multiple polygons
        static std::vector<Polygon> compute(const std::vector<Polygon>& polygons, Operation op);

        // Merge multiple polygons (cascading union)
        static Polygon mergeAll(const std::vector<Polygon>& polygons);

        // Clip polygon against another (like difference but returns multiple pieces)
        static std::vector<Polygon> clip(const Polygon& subject, const Polygon& clip);
    };

} // namespace PolygonBoolean

#endif // POLYGON_H

