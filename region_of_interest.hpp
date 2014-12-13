#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

class RegionOfInterest {

private:
	Rect _outerBounds;
	Rect _innerBounds;
	Mat _sourceImage;
	vector<vector<Point>> _contours;
	int _contourIndex;
	vector<Vec4i> _hierarchy;
	string _ident;

	Scalar _identColor;

	Mat _objPix(int color);

public:

	RegionOfInterest(Mat sourceImage, vector<vector<Point>> contours, int contourIndex, vector<Vec4i> hierarchy, string ident);

	Rect innerBounds();
	Rect outerBounds();

	Scalar identColor();
	Mat whiteObjPix();
	Mat blackObjPix();

	int panelCount();
};
