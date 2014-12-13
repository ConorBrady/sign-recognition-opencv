#include "search_image.hpp"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <boost/filesystem.hpp>

#include <iostream>

using namespace cv;
using namespace std;

Mat SearchImage::_getBackProjection(Mat backProjection) {

	Mat src; Mat hsv;
	Mat mask;

	cvtColor( backProjection, hsv, COLOR_BGR2HSV );

	Mat hist;
	int h_bins = 20; int s_bins = 20;
	int histSize[] = { h_bins, s_bins };
	float h_range[] = { 0, 180 };
	float s_range[] = { 0, 255 };
	const float* ranges[] = { h_range, s_range };
	int channels[] = { 0, 1 };
	calcHist( &hsv, 1, channels, mask, hist, 2, histSize, ranges, true, false );
	normalize( hist, hist, 0, 255, NORM_MINMAX, -1, Mat());

	cvtColor( _image, hsv, COLOR_BGR2HSV );

	Mat backproj;
	calcBackProject( &hsv, 1, channels, hist, backproj, ranges, 1, true );

	dilate(backproj,backproj,Mat());
#ifdef GEN
	imwrite(string("gen/")+_ident+"-backproject.png",backproj);
#endif
	threshold(backproj, backproj, 5, 255, cv::THRESH_BINARY);
	return backproj;
}

SearchImage::SearchImage(const char * filename) {
	_image = imread(filename);
	_ident = boost::filesystem::path(filename).stem().string();
}

vector<RegionOfInterest> SearchImage::roisForBackProjection(Mat backProjection) {

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	vector<RegionOfInterest> rois;

	findContours(_getBackProjection(backProjection), contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE );

#ifdef GEN
	Mat genImage = _image.clone();
#endif

	for (int i = 0; i < contours.size(); i++)
	{
		if(hierarchy[i][2] >= 0 && hierarchy[i][3] < 0) { // Has child and no parent
			RegionOfInterest roi(_image,contours,i,hierarchy,_ident+"-"+to_string(i));
			if(roi.innerBounds().area() > 0) {
				rois.push_back(roi);
#ifdef GEN
				drawContours( genImage, contours, i, roi.identColor(), CV_FILLED, 8, hierarchy );
#endif

			}
		}
	}
#ifdef GEN
	char buffer[50];
	sprintf(buffer, "gen/%s-roisForBackProjection.png", _ident.c_str());
	imwrite(buffer,genImage);
#endif

	return rois;
}

Mat SearchImage::image() {
	return _image;
}

string SearchImage::ident() {
	return _ident;
}
