#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

class KnownSign {
private:
	float _whiteNormDiv;
	float _blackNormDiv;
	vector<Point> _blackPoints;
	vector<Point> _whitePoints;
	int _panelCount;
	Mat _image;

public:
	KnownSign(const char* filename, float whiteNormDiv, float blackNormDiv);

	int panelCount();
	float match(Mat whiteObjPix, Mat blackObjPix);

	Mat thumbnail();
};
