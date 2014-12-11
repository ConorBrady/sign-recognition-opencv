#include "search_image.hpp"

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>

using namespace cv;
using namespace std;

Mat getBackProjection(const char* filename, Mat image) {

	Mat src; Mat hsv;
	Mat mask;

	src = imread( filename, 1 );
	cvtColor( src, hsv, COLOR_BGR2HSV );

	Mat hist;
	int h_bins = 20; int s_bins = 20;
	int histSize[] = { h_bins, s_bins };
	float h_range[] = { 0, 180 };
	float s_range[] = { 0, 255 };
	const float* ranges[] = { h_range, s_range };
	int channels[] = { 0, 1 };
	calcHist( &hsv, 1, channels, mask, hist, 2, histSize, ranges, true, false );
	normalize( hist, hist, 0, 255, NORM_MINMAX, -1, Mat());

	cvtColor( image, hsv, COLOR_BGR2HSV );

	Mat backproj;
	calcBackProject( &hsv, 1, channels, hist, backproj, ranges, 1, true );

	dilate(backproj,backproj,Mat());
	threshold(backproj, backproj, 5, 255, cv::THRESH_BINARY);
	return backproj;
}

SearchImage::SearchImage(const char * filename) {
	_image = imread(filename);
}

vector<RegionOfInterest> SearchImage::roisForBackProjection(const char * filename) {

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	vector<RegionOfInterest> roi;

	findContours(getBackProjection(filename, _image), contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE );

	for (int i = 0; i < contours.size(); i++)
	{
		if(hierarchy[i][2] >= 0 && hierarchy[i][3] < 0) { // Has child and no parent
			roi.push_back(RegionOfInterest(_image,contours,i,hierarchy));
		}
	}

	return roi;
}

Mat SearchImage::image() {
	return _image;
}
