// 🔥 CLI layer
using GeometryCLI;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Shapes;

namespace GeometryUI
{
    public partial class MainWindow : Window
    {
        // ============================
        // Data storage
        // ============================
        private List<Point> currentPoints = new List<Point>();
        private List<Point> polygonA = new List<Point>();
        private List<Point> polygonB = new List<Point>();

        public MainWindow()
        {
            InitializeComponent();
        }

        // ============================
        // Mouse click → add point
        // ============================
        private void DrawCanvas_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            Point p = e.GetPosition(DrawCanvas);
            currentPoints.Add(p);

            Ellipse dot = new Ellipse
            {
                Width = 6,
                Height = 6,
                Fill = Brushes.Black
            };

            Canvas.SetLeft(dot, p.X - 3);
            Canvas.SetTop(dot, p.Y - 3);

            DrawCanvas.Children.Add(dot);
        }

        // ============================
        // Finish Polygon A
        // ============================
        private void FinishPolygonA_Click(object sender, RoutedEventArgs e)
        {
            if (currentPoints.Count < 3)
            {
                MessageBox.Show("Polygon A needs at least 3 points");
                return;
            }

            polygonA = new List<Point>(currentPoints);
            DrawPolygon(polygonA, Brushes.Black);

            currentPoints.Clear();
        }

        // ============================
        // Finish Polygon B
        // ============================
        private void FinishPolygonB_Click(object sender, RoutedEventArgs e)
        {
            if (currentPoints.Count < 3)
            {
                MessageBox.Show("Polygon B needs at least 3 points");
                return;
            }

            polygonB = new List<Point>(currentPoints);
            DrawPolygon(polygonB, Brushes.Blue);

            currentPoints.Clear();
        }

        // ============================
        // Draw outline polygon
        // ============================
        private void DrawPolygon(List<Point> points, Brush stroke)
        {
            Polygon poly = new Polygon
            {
                Stroke = stroke,
                StrokeThickness = 2,
                Fill = Brushes.Transparent
            };

            foreach (var p in points)
                poly.Points.Add(p);

            DrawCanvas.Children.Add(poly);
        }

        // ============================
        // Clear everything
        // ============================
        private void Clear_Click(object sender, RoutedEventArgs e)
        {
            DrawCanvas.Children.Clear();
            currentPoints.Clear();
            polygonA.Clear();
            polygonB.Clear();
        }


        private void Union_Click(object sender, RoutedEventArgs e)
        {
            if (polygonA.Count < 3 || polygonB.Count < 3)
            {
                MessageBox.Show("Draw both Polygon A and Polygon B first");
                return;
            }

            // Convert WPF → CLI
            var outerA = ToPoint2DList(polygonA);
            var outerB = ToPoint2DList(polygonB);

            var holesA = new List<List<Point2D>>(); // no holes yet
            var holesB = new List<List<Point2D>>();

            // 🔥 CLI CALL
            var result = BooleanEngine.Compute(
                outerA,
                holesA,
                outerB,
                holesB,
                BooleanOperation.Union
            );

            DrawResult(result);
        }


        private void AminusB_Click(object sender, RoutedEventArgs e)
        {
            if (polygonA.Count < 3 || polygonB.Count < 3)
            {
                MessageBox.Show("Draw both Polygon A and Polygon B first");
                return;
            }

            // Convert WPF → CLI
            var outerA = ToPoint2DList(polygonA);
            var outerB = ToPoint2DList(polygonB);

            var holesA = new List<List<Point2D>>();
            var holesB = new List<List<Point2D>>();

            // 🔥 CLI CALL
            var result = BooleanEngine.Compute(
                outerA,
                holesA,
                outerB,
                holesB,
                BooleanOperation.AminusB
            );

            DrawResult(result);
        }


        private void BminusA_Click(object sender, RoutedEventArgs e)
        {
            if (polygonA.Count < 3 || polygonB.Count < 3)
            {
                MessageBox.Show("Draw both Polygon A and Polygon B first");
                return;
            }

            // Convert WPF → CLI
            var outerA = ToPoint2DList(polygonA);
            var outerB = ToPoint2DList(polygonB);

            var holesA = new List<List<Point2D>>();
            var holesB = new List<List<Point2D>>();

            // 🔥 CLI CALL
            var result = BooleanEngine.Compute(
                outerA,
                holesA,
                outerB,
                holesB,
                BooleanOperation.BminusA
            );

            DrawResult(result);
        }


        // =========================================================
        // 🔥 INTERSECTION BUTTON → CLI LAYER CONNECTION
        // =========================================================
        private void Intersection_Click(object sender, RoutedEventArgs e)
        {
            if (polygonA.Count < 3 || polygonB.Count < 3)
            {
                MessageBox.Show("Draw both Polygon A and Polygon B first");
                return;
            }

            // Convert WPF → CLI
            var outerA = ToPoint2DList(polygonA);
            var outerB = ToPoint2DList(polygonB);

            var holesA = new List<List<Point2D>>(); // no holes yet
            var holesB = new List<List<Point2D>>();

            // 🔥 CLI CALL (THIS IS THE CONNECTION)
            var result = BooleanEngine.Compute(
                outerA,
                holesA,
                outerB,
                holesB,
                BooleanOperation.Intersection
            );

            DrawResult(result);
        }

        // ============================
        // Convert WPF → CLI
        // ============================
        private List<Point2D> ToPoint2DList(List<Point> points)
        {
            var list = new List<Point2D>();
            foreach (var p in points)
            {
                list.Add(new Point2D { X = p.X, Y = p.Y });
            }
            return list;
        }

        // ============================
        // Draw result from CLI
        // ============================
        private void DrawResult(List<List<Point2D>> rings)
        {
            if (rings == null || rings.Count == 0)
            {
                MessageBox.Show("No intersection result");
                return;
            }

            PathGeometry geometry = new PathGeometry
            {
                FillRule = FillRule.EvenOdd // holes supported
            };

            foreach (var ring in rings)
            {
                if (ring.Count < 3)
                    continue;

                PathFigure fig = new PathFigure
                {
                    StartPoint = new Point(ring[0].X, ring[0].Y),
                    IsClosed = true,
                    IsFilled = true
                };

                for (int i = 1; i < ring.Count; i++)
                {
                    fig.Segments.Add(
                        new LineSegment(
                            new Point(ring[i].X, ring[i].Y), true));
                }

                geometry.Figures.Add(fig);
            }

            Path path = new Path
            {
                Data = geometry,
                Fill = Brushes.LightGreen,
                Stroke = Brushes.DarkGreen,
                StrokeThickness = 2
            };

            DrawCanvas.Children.Add(path);
        }
    }
}
