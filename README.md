# PolygonBooleanOperations



\## Overview

This project implements basic \*\*Boolean operations on 2D polygons\*\* using a custom geometry engine written from scratch, without relying on any third-party geometry libraries.



The system is structured as a \*\*three-layer architecture\*\*:



\- \*\*WPF (C#)\*\* – Interactive UI for drawing polygons and visualizing results  

\- \*\*C++/CLI\*\* – Interoperability layer (managed ↔ native conversion)  

\- \*\*Native C++\*\* – Core computational geometry algorithms  



The goal of this project is to demonstrate understanding of \*\*computational geometry fundamentals\*\*, \*\*polygon representations\*\*, and \*\*Boolean operation concepts\*\* commonly used in CAD/CAM and graphics software.



---



\## Features Implemented



\### ✅ Polygon Drawing (UI)

\- Interactive polygon drawing using mouse clicks (MS-Paint style)

\- Separate polygons \*\*A\*\* and \*\*B\*\*

\- Real-time visualization of input geometry



\### ✅ Boolean Operations (Core)

\- \*\*Intersection (A ∩ B)\*\*  

\- \*\*Union (A ∪ B)\*\* \*(approximate)\*  

\- \*\*Difference (A − B, B − A)\*\* \*(limited support)\*  



All algorithms are implemented manually using basic geometric primitives.



---



\## Mathematical Approach



\### Geometry Primitives

\- Point, segment, ring, and polygon representations

\- Ray-casting algorithm for \*\*Point-in-Polygon\*\*

\- Line–line intersection using linear equations and segment bounds

\- Orientation detection using \*\*signed polygon area (CW / CCW)\*\*



\### Intersection

\- Implemented using a \*\*polygon clipping approach\*\* inspired by the

&nbsp; \*Sutherland–Hodgman\* algorithm

\- Works reliably for \*\*simple and convex clipping cases\*\*

\- Orientation normalization (CCW) is required before clipping



---



\## ⚠️ Known Limitations (Important)



\### ❌ Intersection

\- In \*\*complex or concave configurations\*\*, intersection may produce

&nbsp; incorrect or empty results

\- Edge-touching and numerical precision cases are not fully robust

\- Algorithm is not a full topology-aware Boolean kernel



\### ❌ A − B and B − A

\- \*\*Difference operations are NOT fully implemented\*\*

\- The following cases are \*\*not supported\*\*:

&nbsp; - Polygons with holes

&nbsp; - Multiple disjoint output regions

&nbsp; - Complex concave subtraction

 - intersecting polygons.

\- In many overlapping cases, the result may be empty or incorrect



> \*\*Reason:\*\*  

> Proper implementation of A−B and B−A requires advanced graph-based

> Boolean algorithms (e.g. Greiner–Hormann, Weiler–Atherton), which are

> beyond the scope of this project.



\### Union

\- Union is implemented in a \*\*visual / approximate manner\*\*

\- Full boundary merging is not performed for overlapping polygons



---



\## Design Decision (Honest Statement)



This project focuses on \*\*algorithmic understanding\*\* rather than building

a production-ready geometry kernel.



The implementation intentionally avoids:

\- Third-party geometry libraries

\- Half-edge / DCEL data structures

\- Robust floating-point tolerance handling



As a result, the Boolean operations are \*\*educational and demonstrative\*\*,

not exhaustive.



---



\## Build \& Run



\### Requirements

\- Visual Studio (Windows)

\- .NET (WPF)

\- C++/CLI support enabled



\### Steps

1\. Open the solution in Visual Studio

2\. Build the solution

3\. Run the WPF project

4\. Draw Polygon A and Polygon B

5\. Apply Boolean operations using UI buttons



---



\## Summary



✔ Clean layered architecture (UI → CLI → Native)  

✔ Core computational geometry implemented from scratch  

✔ Demonstrates understanding of polygon Boolean concepts  

⚠️ Difference operations (A−B, B−A) are incomplete  

⚠️ Intersection fail



---



\## Future Improvements

\- Implement Greiner–Hormann or Weiler–Atherton Boolean algorithm

\- Support holes and multiple output regions

\- Add robust numerical tolerance handling

\- Restrict UI input to convex polygons for guaranteed correctness

