#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

class BackProjectionPacker {
public:
	static Mat pack(const char* filename);
};
