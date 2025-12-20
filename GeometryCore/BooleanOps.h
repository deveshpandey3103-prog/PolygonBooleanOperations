#pragma once
#include <vector>
#include "Polygonutility.h"

enum class BoolOp {
    Union,
    Intersection,
    AminusB,
    BminusA
};

class BooleanOps
{
public:
    bool KeepSegment(bool inA, bool inB, BoolOp op);
    std::vector<Polygon> ComputeBoolean(
        const Polygon& A,
        const Polygon& B,
        BoolOp operation);
};

