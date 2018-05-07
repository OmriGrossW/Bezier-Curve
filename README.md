# Bezier-Curve

Bezier Curve approximation using OpenGL - by Piecewise Cubic Bezier Curve drawing and de Casteljau's algorithms.

In the default mode the user left-clicks inside the window to define a series of control points.
Each left-click creates a new control point in the position the mouse points at, after which
the program should draw a red point in that position.

Starting with the second selected point the program should draw a red line between the current and previous points.
By pressing 'd', the program should render an approximation of the Piecewise Bezier Curve
according to the following process:

  1. Insert the implicit control points at the appropriate places: the mid-points on the
  lines that join internal control points from different curves (see figure attached. Lines
  joining same Bezier curves share the same color; points marked as implicit are only
  added after the user finished defining the curve).
  2. Create a list of cubic Bezier curves from the created series of points
  3. Subdivide the curves n times according to de Casteljau's algorithm (slide 27 in the
  parametric curves practical session). n should be specified as a command line
  argument to the program.
  4. Draw green lines between the start and end-points of all the curves in order to get
  an approximation of the original curves.

By pressing 'c', the program should clear the window and return to the default mode.

The program will not draw the last curve if it is quadratic, will only do so if n=0 and no Casteljau division is made.

Control Keys Summary:
To clear screen press 'C' , to Render 'D'.
If you don't enter n value in command line then it is initialized to 0,
then 'space' key is active and used to increment n value by 1 for each 'space' key press.


