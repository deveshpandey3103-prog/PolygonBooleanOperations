#include "pch.h"
#include "Polygon.h"
#include <sstream>
#include <stack>
#include <queue>
#include <cmath>

namespace PolygonBoolean {

    // ==================== Point Implementation ====================
    bool Point::operator==(const Point& other) const {
        const double epsilon = 1e-10;
        return std::abs(x - other.x) < epsilon && std::abs(y - other.y) < epsilon;
    }

    bool Point::operator!=(const Point& other) const {
        return !(*this == other);
    }

    Point Point::operator-(const Point& other) const {
        return Point(x - other.x, y - other.y);
    }

    Point Point::operator+(const Point& other) const {
        return Point(x + other.x, y + other.y);
    }

    double Point::distance(const Point& other) const {
        double dx = x - other.x;
        double dy = y - other.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    double Point::cross(const Point& other) const {
        return x * other.y - y * other.x;
    }

    double Point::dot(const Point& other) const {
        return x * other.x + y * other.y;
    }

    // ==================== Vertex Implementation ====================
    Vertex::Vertex(const Point& p)
        : point(p), type(VertexType::NORMAL),
        next(nullptr), prev(nullptr), neighbor(nullptr),
        intersect(false), visited(false), entryExitProcessed(false),
        alpha(0.0) {
    }

    Vertex::Vertex(const Point& p, double alpha)
        : point(p), type(VertexType::UNKNOWN),
        next(nullptr), prev(nullptr), neighbor(nullptr),
        intersect(true), visited(false), entryExitProcessed(false),
        alpha(alpha) {
    }

    Vertex::Vertex(const Vertex& other)
        : point(other.point), type(other.type),
        next(nullptr), prev(nullptr), neighbor(nullptr),
        intersect(other.intersect), visited(other.visited),
        entryExitProcessed(other.entryExitProcessed),
        alpha(other.alpha) {
    }

    Vertex& Vertex::operator=(const Vertex& other) {
        if (this != &other) {
            point = other.point;
            type = other.type;
            next = nullptr;
            prev = nullptr;
            neighbor = nullptr;
            intersect = other.intersect;
            visited = other.visited;
            entryExitProcessed = other.entryExitProcessed;
            alpha = other.alpha;
        }
        return *this;
    }

    Vertex::~Vertex() {
        // Note: We don't delete next/prev to avoid recursive deletion
        // The Polygon class handles the complete list deletion
    }

    bool Vertex::isInside() const {
        return type == VertexType::ENTRY;
    }

    void Vertex::markAsProcessed() {
        visited = true;
        if (neighbor) {
            neighbor->visited = true;
        }
    }

    // ==================== Polygon Implementation ====================

    // Private helper functions
    namespace {
        const double EPSILON = 1e-10;

        bool isZero(double value) {
            return std::abs(value) < EPSILON;
        }

        bool areEqual(double a, double b) {
            return std::abs(a - b) < EPSILON;
        }

        double normalizeAngle(double angle) {
            while (angle < 0) angle += 2 * 3.14;
            while (angle >= 2 * 3.14) angle -= 2 * 3.14;
            return angle;
        }
    }

    // Constructors & Destructor
    Polygon::Polygon()
        : head(nullptr), clockwise(true), directionCalculated(false) {
    }

    Polygon::Polygon(const std::vector<Point>& points)
        : head(nullptr), clockwise(true), directionCalculated(false) {
        for (const auto& p : points) {
            addPoint(p);
        }
    }

    Polygon::Polygon(const Polygon& other)
        : head(nullptr), clockwise(other.clockwise),
        directionCalculated(other.directionCalculated) {
        if (other.head) {
            head = other.copyVertexList();
        }
    }

    Polygon::Polygon(Polygon&& other) noexcept
        : head(other.head), clockwise(other.clockwise),
        directionCalculated(other.directionCalculated) {
        other.head = nullptr;
        other.clockwise = true;
        other.directionCalculated = false;
    }

    Polygon::~Polygon() {
        clearVertices();
    }

    // Assignment operators
    Polygon& Polygon::operator=(const Polygon& other) {
        if (this != &other) {
            clearVertices();
            if (other.head) {
                head = other.copyVertexList();
            }
            clockwise = other.clockwise;
            directionCalculated = other.directionCalculated;
        }
        return *this;
    }

    Polygon& Polygon::operator=(Polygon&& other) noexcept {
        if (this != &other) {
            clearVertices();
            head = other.head;
            clockwise = other.clockwise;
            directionCalculated = other.directionCalculated;

            other.head = nullptr;
            other.clockwise = true;
            other.directionCalculated = false;
        }
        return *this;
    }

    // Basic operations
    void Polygon::addPoint(const Point& p) {
        Vertex* newVertex = new Vertex(p);

        if (!head) {
            head = newVertex;
            head->next = head;
            head->prev = head;
        }
        else {
            Vertex* last = head->prev;
            last->next = newVertex;
            newVertex->prev = last;
            newVertex->next = head;
            head->prev = newVertex;
        }

        directionCalculated = false;
    }

    void Polygon::addPoint(double x, double y) {
        addPoint(Point(x, y));
    }

    void Polygon::clear() {
        clearVertices();
        head = nullptr;
        directionCalculated = false;
    }

    size_t Polygon::vertexCount() const {
        if (!head) return 0;

        size_t count = 0;
        Vertex* current = head;
        do {
            count++;
            current = current->next;
        } while (current != head);

        return count;
    }

    // Polygon properties
    std::vector<Point> Polygon::getPoints() const {
        std::vector<Point> points;
        if (!head) return points;

        Vertex* current = head;
        do {
            points.push_back(current->point);
            current = current->next;
        } while (current != head);

        return points;
    }

    double Polygon::area() const {
        if (!head || vertexCount() < 3) return 0.0;

        double area = 0.0;
        Vertex* current = head;
        do {
            area += current->point.cross(current->next->point);
            current = current->next;
        } while (current != head);

        return std::abs(area) / 2.0;
    }

    bool Polygon::isClockwise() const {
        if (!directionCalculated) {
            calculateDirection();
        }
        return clockwise;
    }

    bool Polygon::isConvex() const {
        if (vertexCount() < 3) return true;

        Vertex* current = head;
        bool hasPositive = false;
        bool hasNegative = false;

        do {
            double cross = crossProduct(current->prev->point,
                current->point,
                current->next->point);

            if (cross > EPSILON) hasPositive = true;
            if (cross < -EPSILON) hasNegative = true;

            if (hasPositive && hasNegative) return false;

            current = current->next;
        } while (current != head);

        return true;
    }

    bool Polygon::isValid() const {
        if (vertexCount() < 3) return false;

        // Check for duplicate consecutive points
        Vertex* current = head;
        do {
            if (current->point == current->next->point) {
                return false;
            }
            current = current->next;
        } while (current != head);

        return true;
    }

    // Transformation methods
    void Polygon::reverse() {
        if (!head || vertexCount() < 2) return;

        Vertex* current = head;
        do {
            std::swap(current->next, current->prev);
            current = current->prev; // Note: prev and next are swapped
        } while (current != head);

        head = head->next;
        clockwise = !clockwise;
    }

    void Polygon::translate(double dx, double dy) {
        if (!head) return;

        Vertex* current = head;
        do {
            current->point.x += dx;
            current->point.y += dy;
            current = current->next;
        } while (current != head);
    }

    void Polygon::scale(double sx, double sy) {
        if (!head) return;

        // Find centroid for scaling
        Point centroid(0, 0);
        size_t count = 0;

        Vertex* current = head;
        do {
            centroid.x += current->point.x;
            centroid.y += current->point.y;
            count++;
            current = current->next;
        } while (current != head);

        if (count > 0) {
            centroid.x /= count;
            centroid.y /= count;
        }

        // Scale relative to centroid
        current = head;
        do {
            current->point.x = centroid.x + (current->point.x - centroid.x) * sx;
            current->point.y = centroid.y + (current->point.y - centroid.y) * sy;
            current = current->next;
        } while (current != head);
    }

    void Polygon::rotate(double angleRadians) {
        if (!head) return;

        // Find centroid for rotation
        Point centroid(0, 0);
        size_t count = 0;

        Vertex* current = head;
        do {
            centroid.x += current->point.x;
            centroid.y += current->point.y;
            count++;
            current = current->next;
        } while (current != head);

        if (count > 0) {
            centroid.x /= count;
            centroid.y /= count;
        }

        double cosA = std::cos(angleRadians);
        double sinA = std::sin(angleRadians);

        // Rotate relative to centroid
        current = head;
        do {
            double dx = current->point.x - centroid.x;
            double dy = current->point.y - centroid.y;

            current->point.x = centroid.x + dx * cosA - dy * sinA;
            current->point.y = centroid.y + dx * sinA + dy * cosA;

            current = current->next;
        } while (current != head);
    }

    // Query methods
    bool Polygon::contains(const Point& p) const {
        return pointInPolygon(p);
    }

    bool Polygon::contains(const Polygon& other) const {
        if (!head || !other.head) return false;

        Vertex* current = other.head;
        do {
            if (!pointInPolygon(current->point)) {
                return false;
            }
            current = current->next;
        } while (current != other.head);

        return true;
    }

    bool Polygon::intersects(const Polygon& other) const {
        if (!head || !other.head) return false;

        // Simple bounding box check first
        // (Implementation omitted for brevity, but should be added)

        // Check edge intersections
        Vertex* currentA = head;
        do {
            Vertex* currentB = other.head;
            do {
                Point intersection;
                double t1, t2;

                if (lineSegmentIntersection(currentA->point, currentA->next->point,
                    currentB->point, currentB->next->point,
                    intersection, t1, t2)) {
                    return true;
                }

                currentB = currentB->next;
            } while (currentB != other.head);

            currentA = currentA->next;
        } while (currentA != head);

        return false;
    }

    // Boolean operations (static methods)
    Polygon Polygon::unionPolygons(const Polygon& A, const Polygon& B) {
        Polygon result = A;
        result.unionWith(B);
        return result;
    }

    Polygon Polygon::intersectionPolygons(const Polygon& A, const Polygon& B) {
        Polygon result = A;
        result.intersectWith(B);
        return result;
    }

    Polygon Polygon::differencePolygons(const Polygon& A, const Polygon& B) {
        Polygon result = A;
        result.subtract(B);
        return result;
    }

    Polygon Polygon::symmetricDifference(const Polygon& A, const Polygon& B) {
        Polygon unionAB = unionPolygons(A, B);
        Polygon intersectionAB = intersectionPolygons(A, B);
        return differencePolygons(unionAB, intersectionAB);
    }

    // Boolean operations (instance methods)
    Polygon Polygon::getUnion(const Polygon& other) const {
        return unionPolygons(*this, other);
    }

    Polygon Polygon::getIntersection(const Polygon& other) const {
        return intersectionPolygons(*this, other);
    }

    Polygon Polygon::getDifference(const Polygon& other) const {
        return differencePolygons(*this, other);
    }

    Polygon Polygon::getSymmetricDifference(const Polygon& other) const {
        return symmetricDifference(*this, other);
    }

    // Combine operations
    void Polygon::unionWith(const Polygon& other) {
        // Implementation of Greiner-Hormann algorithm for union
        if (!head) {
            *this = other;
            return;
        }
        if (!other.head) return;

        // Create working copies
        Polygon polyA = *this;
        Polygon polyB = other;

        // Find intersections
        polyA.findIntersections(polyB);

        // Classify vertices
        polyA.classifyVertices(polyB, 0); // 0 for union

        // Extract result
        std::vector<Polygon> results = polyA.extractResultPolygons();

        if (!results.empty()) {
            *this = results[0];
        }
    }

    void Polygon::intersectWith(const Polygon& other) {
        // Similar to unionWith but with different vertex classification
        if (!head || !other.head) {
            clear();
            return;
        }

        Polygon polyA = *this;
        Polygon polyB = other;

        polyA.findIntersections(polyB);
        polyA.classifyVertices(polyB, 1); // 1 for intersection

        std::vector<Polygon> results = polyA.extractResultPolygons();

        if (!results.empty()) {
            *this = results[0];
        }
        else {
            clear();
        }
    }

    void Polygon::subtract(const Polygon& other) {
        if (!head) return;
        if (!other.head) return;

        Polygon polyA = *this;
        Polygon polyB = other;

        polyA.findIntersections(polyB);
        polyA.classifyVertices(polyB, 2); // 2 for difference

        std::vector<Polygon> results = polyA.extractResultPolygons();

        if (!results.empty()) {
            *this = results[0];
        }
        else {
            clear();
        }
    }

    // ==================== Private Methods ====================

    void Polygon::calculateDirection() const {
        if (!head || vertexCount() < 3) {
            clockwise = true;
            directionCalculated = true;
            return;
        }

        double areaSum = 0.0;
        Vertex* current = head;

        do {
            areaSum += (current->next->point.x - current->point.x) *
                (current->next->point.y + current->point.y);
            current = current->next;
        } while (current != head);

        clockwise = areaSum > 0;
        directionCalculated = true;
    }

    bool Polygon::pointInPolygon(const Point& p) const {
        if (!head || vertexCount() < 3) return false;

        int windingNumber = 0;
        Vertex* current = head;

        do {
            Vertex* next = current->next;

            if (current->point.y <= p.y) {
                if (next->point.y > p.y) {
                    if (crossProduct(current->point, next->point, p) > 0) {
                        windingNumber++;
                    }
                }
            }
            else {
                if (next->point.y <= p.y) {
                    if (crossProduct(current->point, next->point, p) < 0) {
                        windingNumber--;
                    }
                }
            }

            current = current->next;
        } while (current != head);

        return windingNumber != 0;
    }

    double Polygon::crossProduct(const Point& a, const Point& b, const Point& c) const {
        return (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
    }

    bool Polygon::lineSegmentIntersection(const Point& p1, const Point& p2,
        const Point& q1, const Point& q2,
        Point& intersection,
        double& t1, double& t2) const {
        Point r = p2 - p1;
        Point s = q2 - q1;
        Point qp = q1 - p1;

        double rxs = r.cross(s);
        double qpxr = qp.cross(r);

        // Parallel lines
        if (isZero(rxs)) {
            return false;
        }

        t1 = qp.cross(s) / rxs;
        t2 = qpxr / rxs;

        if (t1 < -EPSILON || t1 > 1.0 + EPSILON ||
            t2 < -EPSILON || t2 > 1.0 + EPSILON) {
            return false;
        }

        intersection.x = p1.x + t1 * r.x;
        intersection.y = p1.y + t1 * r.y;

        return true;
    }

    void Polygon::insertVertexAfter(Vertex* position, Vertex* newVertex) {
        if (!position || !newVertex) return;

        newVertex->prev = position;
        newVertex->next = position->next;
        position->next->prev = newVertex;
        position->next = newVertex;
    }

    void Polygon::removeVertex(Vertex* vertex) {
        if (!vertex) return;

        if (vertex == head) {
            head = vertex->next;
            if (head == vertex) { // Only one vertex
                head = nullptr;
            }
        }

        vertex->prev->next = vertex->next;
        vertex->next->prev = vertex->prev;

        delete vertex;
    }

    void Polygon::clearVertices() {
        if (!head) return;

        Vertex* current = head;
        do {
            Vertex* next = current->next;
            delete current;
            current = next;
        } while (current != head);

        head = nullptr;
    }

    Vertex* Polygon::copyVertexList() const {
        if (!head) return nullptr;

        Vertex* newHead = nullptr;
        Vertex* current = head;
        Vertex* lastCopied = nullptr;

        do {
            Vertex* newVertex = new Vertex(*current);

            if (!newHead) {
                newHead = newVertex;
            }
            else {
                lastCopied->next = newVertex;
                newVertex->prev = lastCopied;
            }

            lastCopied = newVertex;
            current = current->next;
        } while (current != head);

        // Close the circular list
        if (newHead && lastCopied) {
            newHead->prev = lastCopied;
            lastCopied->next = newHead;
        }

        return newHead;
    }

    void Polygon::findIntersections(Polygon& other) {
        if (!head || !other.head) return;

        // Implementation of intersection finding
        // This is a simplified version - actual Greiner-Hormann needs more
        Vertex* currentA = head;

        do {
            Vertex* currentB = other.head;

            do {
                Point intersection;
                double t1, t2;

                if (lineSegmentIntersection(currentA->point, currentA->next->point,
                    currentB->point, currentB->next->point,
                    intersection, t1, t2)) {

                    // Create intersection vertices
                    Vertex* intA = new Vertex(intersection, t1);
                    Vertex* intB = new Vertex(intersection, t2);

                    // Link them
                    intA->neighbor = intB;
                    intB->neighbor = intA;

                    // Insert into polygons
                    insertVertexAfter(currentA, intA);
                    insertVertexAfter(currentB, intB);
                }

                currentB = currentB->next;
            } while (currentB != other.head);

            currentA = currentA->next;
        } while (currentA != head);
    }

    void Polygon::classifyVertices(const Polygon& other, int operationType) {
        // Classify vertices as entry or exit based on operation type
        if (!head) return;

        Vertex* current = head;
        do {
            if (!current->intersect) {
                bool inside = other.pointInPolygon(current->point);

                switch (operationType) {
                case 0: // Union
                    current->type = inside ? VertexType::EXIT : VertexType::ENTRY;
                    break;
                case 1: // Intersection
                    current->type = inside ? VertexType::ENTRY : VertexType::EXIT;
                    break;
                case 2: // Difference
                    current->type = inside ? VertexType::EXIT : VertexType::ENTRY;
                    break;
                }
            }

            current = current->next;
        } while (current != head);
    }

    std::vector<Polygon> Polygon::extractResultPolygons() {
        std::vector<Polygon> results;

        // Find starting intersection vertex
        Vertex* start = nullptr;
        Vertex* current = head;

        do {
            if (current->intersect && !current->visited) {
                start = current;
                break;
            }
            current = current->next;
        } while (current != head);

        if (!start) return results;

        // Extract polygons
        while (start) {
            Polygon result;
            Vertex* currentVertex = start;
            bool inCurrentPolygon = true;

            do {
                currentVertex->markAsProcessed();
                result.addPoint(currentVertex->point);

                if (currentVertex->intersect) {
                    currentVertex = currentVertex->neighbor;
                    inCurrentPolygon = !inCurrentPolygon;
                }

                // Move to next vertex based on type and operation
                if (inCurrentPolygon) {
                    currentVertex = (currentVertex->type == VertexType::ENTRY) ?
                        currentVertex->next : currentVertex->prev;
                }
                else {
                    currentVertex = (currentVertex->type == VertexType::ENTRY) ?
                        currentVertex->next : currentVertex->prev;
                }

            } while (currentVertex != start);

            results.push_back(result);

            // Find next unvisited intersection
            start = nullptr;
            current = head;
            do {
                if (current->intersect && !current->visited) {
                    start = current;
                    break;
                }
                current = current->next;
            } while (current != head);
        }

        return results;
    }

    // Utility methods (simplified implementations)
    std::vector<Polygon> Polygon::triangulate() const {
        std::vector<Polygon> triangles;
        // Basic ear-clipping algorithm (simplified)
        if (vertexCount() < 3) return triangles;

        std::vector<Point> points = getPoints();

        // Simple triangle fan for convex polygons
        if (isConvex()) {
            for (size_t i = 1; i + 1 < points.size(); i++) {
                Polygon triangle;
                triangle.addPoint(points[0]);
                triangle.addPoint(points[i]);
                triangle.addPoint(points[i + 1]);
                triangles.push_back(triangle);
            }
        }

        return triangles;
    }

    Polygon Polygon::getConvexHull() const {
        // Graham scan algorithm (simplified)
        std::vector<Point> points = getPoints();
        if (points.size() < 3) return *this;

        // Find lowest point
        size_t lowest = 0;
        for (size_t i = 1; i < points.size(); i++) {
            if (points[i].y < points[lowest].y ||
                (points[i].y == points[lowest].y && points[i].x < points[lowest].x)) {
                lowest = i;
            }
        }

        // Sort by polar angle
        std::swap(points[0], points[lowest]);
        Point pivot = points[0];

        std::sort(points.begin() + 1, points.end(),
            [pivot](const Point& a, const Point& b) {
                double cross = (a - pivot).cross(b - pivot);
                if (isZero(cross)) {
                    return pivot.distance(a) < pivot.distance(b);
                }
                return cross > 0;
            });

        // Build hull
        std::vector<Point> hull;
        for (const auto& p : points) {
            while (hull.size() >= 2 &&
                (hull[hull.size() - 1] - hull[hull.size() - 2]).cross(p - hull[hull.size() - 2]) <= 0) {
                hull.pop_back();
            }
            hull.push_back(p);
        }

        return Polygon(hull);
    }

    Polygon Polygon::getOffset(double distance) const {
        // Simple offset (for convex polygons)
        Polygon result = *this;

        if (isConvex() && distance != 0) {
            // Scale polygon to achieve offset
            double scaleFactor = 1.0 + distance / (area() / perimeter());
            result.scale(scaleFactor, scaleFactor);
        }

        return result;
    }

    Polygon Polygon::getBoundary() const {
        return *this; // Simple implementation
    }

    bool Polygon::isSimple() const {
        // Check for self-intersections
        if (vertexCount() < 3) return true;

        Vertex* current = head;
        do {
            Vertex* test = current->next->next;
            while (test != current->prev) {
                Point intersection;
                double t1, t2;

                if (lineSegmentIntersection(current->point, current->next->point,
                    test->point, test->next->point,
                    intersection, t1, t2)) {
                    return false;
                }

                test = test->next;
            }

            current = current->next;
        } while (current != head);

        return true;
    }

    void Polygon::validate() const {
        if (!head) return;

        Vertex* current = head;
        do {
            if (!current->next || !current->prev) {
                throw std::runtime_error("Invalid polygon: broken vertex links");
            }

            if (current->next->prev != current || current->prev->next != current) {
                throw std::runtime_error("Invalid polygon: inconsistent vertex links");
            }

            current = current->next;
        } while (current != head);
    }

    std::string Polygon::toString() const {
        std::stringstream ss;
        ss << "Polygon with " << vertexCount() << " vertices: ";

        Vertex* current = head;
        do {
            ss << "(" << current->point.x << ", " << current->point.y << ") ";
            current = current->next;
        } while (current != head);

        return ss.str();
    }

    // Static utility methods
    bool Polygon::pointsEqual(const Point& a, const Point& b, double epsilon) {
        return std::abs(a.x - b.x) < epsilon && std::abs(a.y - b.y) < epsilon;
    }

    double Polygon::pointDistance(const Point& a, const Point& b) {
        return a.distance(b);
    }

    bool Polygon::isPointOnSegment(const Point& p, const Point& a, const Point& b) {
        if (pointsEqual(p, a) || pointsEqual(p, b)) return true;

        double cross = (b - a).cross(p - a);
        if (!isZero(cross)) return false;

        double dot = (p - a).dot(b - a);
        if (dot < 0) return false;

        double squaredLength = (b - a).dot(b - a);
        if (dot > squaredLength) return false;

        return true;
    }

    int Polygon::pointInPolygon(const Point& p, const std::vector<Point>& polygon) {
        if (polygon.size() < 3) return 0;

        int windingNumber = 0;

        for (size_t i = 0; i < polygon.size(); i++) {
            size_t j = (i + 1) % polygon.size();

            if (polygon[i].y <= p.y) {
                if (polygon[j].y > p.y) {
                    if (crossProduct(polygon[i], polygon[j], p) > 0) {
                        windingNumber++;
                    }
                }
            }
            else {
                if (polygon[j].y <= p.y) {
                    if (crossProduct(polygon[i], polygon[j], p) < 0) {
                        windingNumber--;
                    }
                }
            }
        }

        return windingNumber != 0 ? 1 : 0;
    }

    // ==================== BooleanOperations Implementation ====================

    std::vector<Polygon> BooleanOperations::compute(const Polygon& A, const Polygon& B, Operation op) {
        std::vector<Polygon> results;

        switch (op) {
        case UNION:
            results.push_back(Polygon::unionPolygons(A, B));
            break;
        case INTERSECTION:
            results.push_back(Polygon::intersectionPolygons(A, B));
            break;
        case DIFFERENCE:
            results.push_back(Polygon::differencePolygons(A, B));
            break;
        case SYMMETRIC_DIFFERENCE: {
            Polygon unionAB = Polygon::unionPolygons(A, B);
            Polygon interAB = Polygon::intersectionPolygons(A, B);
            results.push_back(Polygon::differencePolygons(unionAB, interAB));
            break;
        }
        }

        return results;
    }

    Polygon BooleanOperations::computeSingle(const Polygon& A, const Polygon& B, Operation op) {
        auto results = compute(A, B, op);
        return results.empty() ? Polygon() : results[0];
    }

    std::vector<Polygon> BooleanOperations::compute(const std::vector<Polygon>& polygons, Operation op) {
        if (polygons.empty()) return {};
        if (polygons.size() == 1) return polygons;

        std::vector<Polygon> result = { polygons[0] };

        for (size_t i = 1; i < polygons.size(); i++)
