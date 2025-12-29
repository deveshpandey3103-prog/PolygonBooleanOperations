#include "pch.h"
#include "BooleanEngine.h"

// Native geometry headers
#include "../GeometryCore/Polygonutility.h"
#include "../GeometryCore/BooleanOps.h"

using namespace GeometryCLI;
using namespace System::Collections::Generic;

// =======================================================
// Helper: Managed → Native
// =======================================================

static Point ToNative(Point2D p)
{
    return Point{ p.X, p.Y };
}

static Ring ToNativeRing(List<Point2D>^ managedRing)
{
    Ring r;
    for each (Point2D p in managedRing)
        r.vertices.push_back(ToNative(p));
    return r;
}

static Polygon ToNativePolygon(
    List<Point2D>^ outer,
    List<List<Point2D>^>^ holes)
{
    Polygon poly;
    poly.outer = ToNativeRing(outer);

    for each (List<Point2D> ^ h in holes)
        poly.holes.push_back(ToNativeRing(h));

    return poly;
}

// =======================================================
// Helper: Native → Managed
// =======================================================

static Point2D ToManaged(const Point& p)
{
    Point2D mp;
    mp.X = p.x;
    mp.Y = p.y;
    return mp;
}

static List<Point2D>^ ToManagedRing(const Ring& r)
{
    auto list = gcnew List<Point2D>();
    for (const Point& p : r.vertices)
        list->Add(ToManaged(p));
    return list;
}

// =======================================================
// MAIN ENTRY POINT
// =======================================================

List<List<Point2D>^>^ BooleanEngine::Compute(
    List<Point2D>^ outerA,
    List<List<Point2D>^>^ holesA,
    List<Point2D>^ outerB,
    List<List<Point2D>^>^ holesB,
    BooleanOperation operation)
{
    // Convert managed → native
    Polygon polyA = ToNativePolygon(outerA, holesA);
    Polygon polyB = ToNativePolygon(outerB, holesB);

    BoolOp nativeOp;
    switch (operation)
    {
    case BooleanOperation::Union:
        nativeOp = BoolOp::Union; break;
    case BooleanOperation::Intersection:
        nativeOp = BoolOp::Intersection; break;
    case BooleanOperation::AminusB:
        nativeOp = BoolOp::AminusB; break;
    case BooleanOperation::BminusA:
        nativeOp = BoolOp::BminusA; break;
    default:
        throw gcnew ArgumentException("Invalid Boolean Operation");
    }

	BooleanOps boolEngine;
    // Call native Boolean engine
    std::vector<Polygon> result =
        boolEngine.ComputeBoolean2(polyA, polyB, nativeOp);

    // Convert native → managed
    auto managedResult = gcnew List<List<Point2D>^>();

    for (const Polygon& poly : result)
    {
        // Outer ring
        managedResult->Add(ToManagedRing(poly.outer));

        // Holes (each ring separately)
        for (const Ring& hole : poly.holes)
            managedResult->Add(ToManagedRing(hole));
    }

    return managedResult;
}
