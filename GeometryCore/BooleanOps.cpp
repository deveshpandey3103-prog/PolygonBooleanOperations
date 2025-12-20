#include "pch.h"
#include "BooleanOps.h"

bool BooleanOps::KeepSegment(bool inA, bool inB, BoolOp op)
{
    switch (op) {
    case BoolOp::Union:        return inA || inB;
    case BoolOp::Intersection:return inA && inB;
    case BoolOp::AminusB:     return inA && !inB;
    case BoolOp::BminusA:     return inB && !inA;
    }
    return false;
}

std::vector<Polygon> BooleanOps::ComputeBoolean(
    const Polygon& A,
    const Polygon& B,
    BoolOp operation)
{
    std::vector<Polygon> result;
    Polygonutility util;

    switch (operation)
    {
        // ============================================
        // INTERSECTION (CORRECT IMPLEMENTATION)
        // ============================================
    case BoolOp::Intersection:
    {
        // No overlap at all
        if (!util.PolygonsOverlap(A, B))
            return result;

        // Case 1: A fully inside B
        bool allInside = true;
        for (const Point& p : A.outer.vertices)
        {
            if (!util.PointInPolygon(p, B))
            {
                allInside = false;
                break;
            }
        }
        if (allInside)
        {
            result.push_back(A);
            return result;
        }

        // Case 2: B fully inside A
        allInside = true;
        for (const Point& p : B.outer.vertices)
        {
            if (!util.PointInPolygon(p, A))
            {
                allInside = false;
                break;
            }
        }
        if (allInside)
        {
            result.push_back(B);
            return result;
        }

        // Case 3: Partial overlap (CLIPPING)
        std::vector<Point> clipped =
            util.ClipPolygon(A.outer.vertices, B.outer.vertices, util);

        if (clipped.size() < 3)
            return result;

        Polygon intersectionPoly;
        intersectionPoly.outer.vertices = clipped;

        result.push_back(intersectionPoly);
        return result;
    }

    // ============================================
    // UNION (STUB – IMPLEMENT LATER)
    // ============================================
    case BoolOp::Union:
    {
        // No overlap → union is both polygons
        if (!util.PolygonsOverlap(A, B))
        {
            result.push_back(A);
            result.push_back(B);
            return result;
        }

        // A fully inside B → union is B
        bool allInside = true;
        for (const Point& p : A.outer.vertices)
        {
            if (!util.PointInPolygon(p, B))
            {
                allInside = false;
                break;
            }
        }
        if (allInside)
        {
            result.push_back(B);
            return result;
        }

        // B fully inside A → union is A
        allInside = true;
        for (const Point& p : B.outer.vertices)
        {
            if (!util.PointInPolygon(p, A))
            {
                allInside = false;
                break;
            }
        }
        if (allInside)
        {
            result.push_back(A);
            return result;
        }

        // Partial overlap (approximate union)
        // Strategy: return both boundaries (visual union)
        result.push_back(A);
        result.push_back(B);
        return result;
    }


    // ============================================
    // A − B (STUB – IMPLEMENT LATER)
    // ============================================
    case BoolOp::AminusB:
    {
        // No overlap → A remains unchanged
        if (!util.PolygonsOverlap(A, B))
        {
            result.push_back(A);
            return result;
        }

        // A fully inside B → empty
        bool allInside = true;
        for (const Point& p : A.outer.vertices)
        {
            if (!util.PointInPolygon(p, B))
            {
                allInside = false;
                break;
            }
        }
        if (allInside)
            return result; // empty

        // Partial overlap → subtract B from A
        std::vector<Point> clipped =
            util.ClipPolygonOutside(A.outer.vertices, B.outer.vertices, util);

        if (clipped.size() < 3)
            return result;

        Polygon diff;
        diff.outer.vertices = clipped;

        result.push_back(diff);
        return result;
    }


    // ============================================
    // B − A (STUB – IMPLEMENT LATER)
    // ============================================
    case BoolOp::BminusA:
    {
        // No overlap → B remains unchanged
        if (!util.PolygonsOverlap(A, B))
        {
            result.push_back(B);
            return result;
        }

        // B fully inside A → empty
        bool allInside = true;
        for (const Point& p : B.outer.vertices)
        {
            if (!util.PointInPolygon(p, A))
            {
                allInside = false;
                break;
            }
        }
        if (allInside)
            return result; // empty

        // Partial overlap → subtract A from B
        std::vector<Point> clipped =
            util.ClipPolygonOutside(B.outer.vertices, A.outer.vertices, util);

        if (clipped.size() < 3)
            return result;

        Polygon diff;
        diff.outer.vertices = clipped;

        result.push_back(diff);
        return result;
    }


    default:
        return result;
    }
}


