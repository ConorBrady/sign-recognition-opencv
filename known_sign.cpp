#include "known_sign.hpp"

float chamferMatch(vector<Point> templatePoints, Mat chamferedImage, Size matchingSpace) {

	assert(chamferedImage.type() == CV_32FC1);

	float min = -1;
	for(int j = 0; j < matchingSpace.width; j++) {
		for(int k = 0; k < matchingSpace.height; k++) {
			float newValue = 0;
			for(int i = 0; i < templatePoints.size(); i ++ ) {
				newValue += chamferedImage.at<float>(templatePoints[i]+Point(j,k));
			}
			if(min==-1) {
				min = newValue;
			} else {
				min = fmin(newValue,min);
			}
		}
	}

	return min;
}

KnownSign::KnownSign(const char* filename, float whiteNormDiv, float blackNormDiv) {

	_image = imread(filename);
	_whiteNormDiv = whiteNormDiv;
	_blackNormDiv = blackNormDiv;

	Mat temp;

	cvtColor(_image,temp,CV_BGR2GRAY);

	for(int j = 0; j < temp.rows; j++) {
		for(int k = 0; k < temp.cols; k++) {
			if(temp.at<unsigned char>(j,k) > 200) { // Is white point?
				_whitePoints.push_back(Point(k,j));
			}
			if(temp.at<unsigned char>(j,k) < 50) { // Is black point?
				_blackPoints.push_back(Point(k,j));
			}
		}
	}

	threshold(temp, temp, 200, 255, THRESH_BINARY); // Threshold for connected components

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	findContours(temp, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE );

	int child = 0;
	_panelCount = 0;

	while(child >= 0) {

		if(contourArea(contours[child])>50) { // Check for noise and irrelavent panels
			_panelCount++;
		}
		child = hierarchy[child][0];
	}
}

int KnownSign::panelCount() {
	return _panelCount;
}

float KnownSign::match(Mat whiteObjPix, Mat blackObjPix) {

	distanceTransform( whiteObjPix, whiteObjPix, CV_DIST_L2, 3);
	distanceTransform( blackObjPix, blackObjPix, CV_DIST_L2, 3);

	float max = -1;

	float whiteMatch = chamferMatch(_whitePoints,whiteObjPix,whiteObjPix.size()-_image.size())/_whiteNormDiv;
	float blackMatch = chamferMatch(_blackPoints,blackObjPix,blackObjPix.size()-_image.size())/_blackNormDiv;
	return fmax(whiteMatch,blackMatch);
}

Mat KnownSign::thumbnail() {
	Mat thumbnail;
	resize(_image,thumbnail,Size(30,25));
	return thumbnail;
}
