#pragma once

using namespace System;
using namespace System::Collections::Generic;

namespace GeometryCLI
{
    // ============================
    // Managed Point (for WPF)
    // ============================
    public value struct Point2D
    {
        double X;
        double Y;
    };

    // ============================
    // Boolean Operation Enum
    // ============================
    public enum class BooleanOperation
    {
        Union = 0,
        Intersection = 1,
        AminusB = 2,
        BminusA = 3
    };

    // ============================
    // Boolean Engine (CLI Bridge)
    // ============================
    public ref class BooleanEngine
    {
    public:
        static List<List<Point2D>^>^ Compute(
            List<Point2D>^ outerA,
            List<List<Point2D>^>^ holesA,
            List<Point2D>^ outerB,
            List<List<Point2D>^>^ holesB,
            BooleanOperation operation
        );
    };
}
