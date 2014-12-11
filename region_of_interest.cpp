#include "region_of_interest.hpp"


#define MIN_COMPONANT_AREA 200

#define BLACK_PIX 1
#define WHITE_PIX 2

RegionOfInterest::RegionOfInterest(Mat sourceImage, vector<vector<Point>> contours, int contourIndex, vector<Vec4i> hierarchy) {

	_sourceImage = sourceImage;
	_contours = contours;
	_contourIndex = contourIndex;
	_hierarchy = hierarchy;

	// Get inner and outer bounds for the region

	vector<Point> contoursPoly;
	approxPolyDP( Mat(contours[_contourIndex]), contoursPoly, 3, true );
	_outerBounds = boundingRect(Mat(contoursPoly));

	int child = hierarchy[_contourIndex][2];
	while(child >= 0) {
		if(contourArea(contours[child]) > MIN_COMPONANT_AREA) {

			approxPolyDP( Mat(contours[child]), contoursPoly, 3, true );

			if(_innerBounds.area() == 0) {
				_innerBounds = boundingRect(Mat(contoursPoly));
			} else {
				_innerBounds |= boundingRect(Mat(contoursPoly));
			}
		}
		child = hierarchy[child][0];
	}
}

Rect RegionOfInterest::innerBounds() {

	return _innerBounds;
}

Rect RegionOfInterest::outerBounds() {

	return _outerBounds;
}

Mat RegionOfInterest::whiteObjPix() {

	return _objPix(WHITE_PIX);
}

Mat RegionOfInterest::blackObjPix() {

	return _objPix(BLACK_PIX);
}

int RegionOfInterest::panelCount() {

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	findContours(whiteObjPix(), contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE );

	int	child = hierarchy[0][2];

	int count = 0;
	while(child >= 0) {
		if(contourArea(contours[child])>50) {
			count++;
		}
		child = hierarchy[child][0];
	}

	return count;
}

Mat RegionOfInterest::_objPix(int color) {

	Mat fullSizeMask = Mat::zeros(_sourceImage.size(), CV_8UC1);
	int child = _hierarchy[_contourIndex][2];

	while(child >= 0) {

		if(contourArea(_contours[child]) > MIN_COMPONANT_AREA) {
			drawContours(fullSizeMask, _contours, child, Scalar(255), CV_FILLED, 8, _hierarchy, 0);
		}
		child = _hierarchy[child][0];
	}

	Mat grayImage;
	cvtColor( _sourceImage, grayImage, CV_BGR2GRAY);

	Mat boundedImage(grayImage,_innerBounds);
	Mat boundedMask(fullSizeMask,_innerBounds);

	Mat searchImage;
	threshold( boundedImage, boundedImage, -1, 255, ( color==WHITE_PIX ? THRESH_BINARY : THRESH_BINARY_INV ) | THRESH_OTSU );

	boundedImage.copyTo(searchImage, boundedMask);
	bitwise_not(searchImage,searchImage);
	resize(searchImage,searchImage,Size(60,60));
	copyMakeBorder( searchImage, searchImage, 25, 25, 30, 30, BORDER_CONSTANT, Scalar::all(255) );
	threshold(searchImage, searchImage, -1, 255, THRESH_BINARY | THRESH_OTSU );

	return searchImage;

}
