#include "pch.h"
#include "PolygonUtilityExtension.h"
#include <cmath>
#include <algorithm>

static double EPS = 1e-9;

// =====================================================
// Helper: Memory Cleanup (Destructor substitute)
// =====================================================
void Cleanup(Node* poly) {
    if (!poly) return;
    Node* curr = poly;
    do {
        Node* next = curr->next;
        delete curr;
        curr = next;
    } while (curr != poly);
}

// =====================================================
// Build circular doubly linked polygon
// =====================================================
Node* PolygonUtilityExtension::BuildPolygon(const std::vector<Point>& pts)
{
    if (pts.empty()) return nullptr;
    Node* first = nullptr;
    Node* prev = nullptr;

    for (const auto& p : pts)
    {
        Node* n = new Node();
        n->p = p;
        n->isIntersection = false;
        n->visited = false;

        if (!first) first = n;
        if (prev)
        {
            prev->next = n;
            n->prev = prev;
        }
        prev = n;
    }
    prev->next = first;
    first->prev = prev;
    return first;
}

// =====================================================
// Sorted Intersection Insertion
// =====================================================
void PolygonUtilityExtension::InsertInOrder(Node* startNode, Node* newNode) {
    Node* curr = startNode;
    // Walk along the segment to find correct spot based on alpha
    while (curr->next->isIntersection && curr->next->alpha < newNode->alpha) {
        curr = curr->next;
    }
    newNode->next = curr->next;
    newNode->prev = curr;
    curr->next->prev = newNode;
    curr->next = newNode;
}

// =====================================================
// Segment intersection
// =====================================================
bool PolygonUtilityExtension::SegmentIntersect(
    const Point& p1, const Point& p2,
    const Point& q1, const Point& q2,
    Point& ip, double& aP, double& aQ)
{
    double rdx = p2.x - p1.x;
    double rdy = p2.y - p1.y;
    double sdx = q2.x - q1.x;
    double sdy = q2.y - q1.y;

    double det = rdx * sdy - rdy * sdx;
    if (fabs(det) < EPS) return false;

    aP = ((q1.x - p1.x) * sdy - (q1.y - p1.y) * sdx) / det;
    aQ = ((q1.x - p1.x) * rdy - (q1.y - p1.y) * rdx) / det;

    if (aP > EPS && aP < 1.0 - EPS && aQ > EPS && aQ < 1.0 - EPS) {
        ip.x = p1.x + aP * rdx;
        ip.y = p1.y + aP * rdy;
        return true;
    }
    return false;
}

// =====================================================
// Find all intersections
// =====================================================
void PolygonUtilityExtension::FindIntersections(Node* A, Node* B)
{
    Node* a = A;
    do {
        Node* aNext = a->next;
        
        if (a->isIntersection) { a = aNext; continue; }

        Node* b = B;
        do {
            Node* bNext = b->next;
            if (b->isIntersection) { b = bNext; continue; }

            Point ip;
            double ta, tb;
            if (SegmentIntersect(a->p, aNext->p, b->p, bNext->p, ip, ta, tb))
            {
                Node* na = new Node();
                na->p = ip; na->isIntersection = true; na->alpha = ta;

                Node* nb = new Node();
                nb->p = ip; nb->isIntersection = true; nb->alpha = tb;

                na->neighbor = nb;
                nb->neighbor = na;

                InsertInOrder(a, na);
                InsertInOrder(b, nb);
            }
            b = bNext;
        } while (b != B);
        a = aNext;
    } while (a != A);
}

bool PolygonUtilityExtension::PointInsidePolygon(const std::vector<Point>& poly, const Point& p)
{
    if (poly.empty()) return false;

    bool inside = false;
    size_t n = poly.size();

    for (size_t i = 0, j = n - 1; i < n; j = i++)
    {
        if (((poly[i].y > p.y) != (poly[j].y > p.y)) &&
            (p.x < (poly[j].x - poly[i].x) * (p.y - poly[i].y) /
                (poly[j].y - poly[i].y) + poly[i].x))
        {
            inside = !inside; 
        }
    }
    return inside;
}

// =====================================================
// Entry / Exit marking (Fixed Toggle Logic)
// =====================================================
void PolygonUtilityExtension::MarkEntryExit(Node* poly, const std::vector<Point>& other, GHOp op, bool isA)
{
    Node* startNode = poly;
    while (startNode->isIntersection) startNode = startNode->next;

    bool inside = PointInsidePolygon(other, startNode->p);
    Node* curr = startNode;

    do {
        if (curr->isIntersection) {
            // FIXED TOGGLE LOGIC:
            if (op == GHOp::Union) {
                curr->entry = !inside; // Entry if we are moving from outside to inside
            }
            else if (op == GHOp::Intersection) {
                curr->entry = inside;  // Entry if we are moving from inside to outside
            }
            else {
                // For Difference, entry depends on which polygon we are on
                curr->entry = isA ? !inside : inside;
            }
            inside = !inside;
        }
        curr = curr->next;
    } while (curr != startNode);
}

// =====================================================
// Trace output polygon (With Direction switching)
// =====================================================
std::vector<Point> PolygonUtilityExtension::TraceResult(Node* start, GHOp op, bool startOnA)
{
    std::vector<Point> result;
    Node* cur = start;
    bool onPolyA = startOnA;
    bool forward = true; // Default direction

    while (cur != nullptr && !cur->visited)
    {
        cur->visited = true;
        if (cur->neighbor) cur->neighbor->visited = true;

        result.push_back(cur->p);

        if (cur->isIntersection)
        {
            // Switch to neighbor node
            cur = cur->neighbor;
            onPolyA = !onPolyA;

            // DIRECTION LOGIC:
            if (op == GHOp::Intersection || op == GHOp::Union) {
                // For Intersection/Union, we stay forward 
                // but switching polygons handles the clipping.
                forward = true;
            }
            else {
                // For Difference (A-B): A is forward, B is backward.
                forward = onPolyA;
            }
        }

        cur = forward ? cur->next : cur->prev;
        if (cur == start) break;
    }
    return result;
}

// =====================================================
// Helper function for Winding Order
// =====================================================
void PolygonUtilityExtension::EnsureCCW(std::vector<Point>& pts) {
    if (pts.size() < 3) return;
    double area = 0;
    for (size_t i = 0; i < pts.size(); i++) {
        size_t j = (i + 1) % pts.size();
        area += (pts[i].x * pts[j].y) - (pts[j].x * pts[i].y);
    }
    if (area < 0) std::reverse(pts.begin(), pts.end());
}
// =====================================================
// MAIN COMPUTE
// =====================================================
// =====================================================
// FIXED COMPUTE FOR INTERSECTION
// =====================================================
std::vector<std::vector<Point>> PolygonUtilityExtension::Compute(
    const std::vector<Point>& Apts,
    const std::vector<Point>& Bpts,
    GHOp operation)
{
    // Winding order  
    std::vector<Point> A_fixed = Apts;
    std::vector<Point> B_fixed = Bpts;
    EnsureCCW(A_fixed);
    EnsureCCW(B_fixed);

    Node* A = BuildPolygon(A_fixed);
    Node* B = BuildPolygon(B_fixed);

    // Find and Insert Intersections (Sorted by Alpha)
    FindIntersections(A, B);

    // Mark Entry/Exit 
    MarkEntryExit(A, B_fixed, operation, true);
    MarkEntryExit(B, A_fixed, operation, false);

    // Reset Visited Flags
    auto ResetVisited = [](Node* poly) {
        if (!poly) return;
        Node* n = poly;
        do {
            n->visited = false;
            n = n->next;
        } while (n != poly);
        };
    ResetVisited(A);
    ResetVisited(B);

    std::vector<std::vector<Point>> result;

    // 5. Choose start polygon
    Node* startPoly = (operation == GHOp::DifferenceBA) ? B : A;
    bool startOnA = (operation != GHOp::DifferenceBA);

    // 6. Traversal
    // 
    Node* n = startPoly;
    do {
        if (n->isIntersection && !n->visited && n->entry)
        {
            auto loop = TraceResult(n, operation, startOnA);
            if (loop.size() >= 3)
                result.push_back(loop);
        }
        n = n->next;
    } while (n != startPoly);

    // 0Fallback for no intersections
    if (result.empty()) {
        bool A_in_B = PointInsidePolygon(B_fixed, A_fixed[0]);
        bool B_in_A = PointInsidePolygon(A_fixed, B_fixed[0]);
        // ... (standard fallback logic)
    }

    return result;
}