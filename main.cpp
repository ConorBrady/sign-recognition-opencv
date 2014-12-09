#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>

using namespace cv;
using namespace std;

string type2str(int type) {
	string r;

	uchar depth = type & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + (type >> CV_CN_SHIFT);

	switch ( depth ) {
		case CV_8U:  r = "8U"; break;
		case CV_8S:  r = "8S"; break;
		case CV_16U: r = "16U"; break;
		case CV_16S: r = "16S"; break;
		case CV_32S: r = "32S"; break;
		case CV_32F: r = "32F"; break;
		case CV_64F: r = "64F"; break;
		default:     r = "User"; break;
	}

	r += "C";
	r += (chans+'0');

	return r;
}

void printImage(Mat chamfer_image) {

	for(int i = 0; i < chamfer_image.rows; i++) {
		for(int j = 0; j < chamfer_image.cols; j++) {
			float val = chamfer_image.at<float>(i, j);
			if(val == 0) {

				printf("\x1b[31m0\x1b[0m ") ;
			} else {
				printf("1 ") ;
			}

		}
		cout << endl;
	}
	cout << endl;
}

vector<float> maxes(9);
vector<float> mins(9);

float chamfer_match(vector<Point> templatePoints, Mat chamferedImage, Size matchingSpace) {

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

	return min/(templatePoints.size()-400);
}

int main( int, char** argv )
{

	vector<string> unknownSignFiles {
		"unknown_signs/1.png",
		"unknown_signs/2.png",
		"unknown_signs/3.png",
		"unknown_signs/C1.png",
		"unknown_signs/C2.png"
	};

	vector<string> knownSignFiles {
		"known_signs/left.png",
		"known_signs/no_left.png",
		"known_signs/no_parking.png",
		"known_signs/no_right.png",
		"known_signs/no_straight.png",
		"known_signs/parking.png",
		"known_signs/right.png",
		"known_signs/straight.png",
		"known_signs/yield.png"
	};

	vector<Mat> knownSigns(knownSignFiles.size());
	vector<vector<Point>> knownSignsTemplatePoints;

	for (int i = 0; i < knownSignFiles.size(); i++) {
		maxes[i] = 0;
		mins[i] = 10000000;
		knownSigns[i] = imread(knownSignFiles[i]);

		Mat temp;

		cvtColor(knownSigns[i],temp,CV_BGR2GRAY);
		Canny(temp,temp,100,200);

		knownSignsTemplatePoints.push_back(vector<Point>());

		for(int j = 0; j < knownSigns[i].rows; j++) {
			for(int k = 0; k < knownSigns[i].cols; k++) {
				if(temp.at<unsigned char>(j,k) != 0) {
					knownSignsTemplatePoints[i].push_back(Point(k,j));
				}
			}
		}
		imshow("bestmatch",temp);
		// waitKey(0);
	}

	Mat src; Mat hsv;
	Mat mask;

	src = imread( "back_project_data.png", 1 );
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

	for(int i = 0; i < unknownSignFiles.size(); i++) {
		Mat image = imread(unknownSignFiles[i]);
		cvtColor( image, hsv, COLOR_BGR2HSV );

		Mat backproj;
		calcBackProject( &hsv, 1, channels, hist, backproj, ranges, 1, true );
		dilate(backproj,backproj,Mat());
		threshold(backproj, backproj, 5, 255, cv::THRESH_BINARY);
		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;

		findContours( backproj, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE );

		for (int j = 0; j < contours.size(); j++)
		{
			if(hierarchy[j][2] >= 0 && hierarchy[j][3] < 0) { // Has child and no parent

				vector<Point> contoursPoly;
				approxPolyDP( Mat(contours[j]), contoursPoly, 3, true );
				Rect bounds = boundingRect(Mat(contoursPoly));

				Mat mask = Mat::zeros(image.size(), CV_8UC1);
				drawContours(mask, contours, j, Scalar(255), CV_FILLED, 8, hierarchy, 0);

				Mat maskedImage;
				Mat invGrayImage;
				cvtColor( image, invGrayImage, CV_BGR2GRAY);
				bitwise_not(invGrayImage,invGrayImage);
				invGrayImage.copyTo(maskedImage, mask);
				Mat searchImage(maskedImage, bounds);
				bitwise_not(maskedImage,maskedImage);

				resize(searchImage,searchImage,Size(80,80));
				copyMakeBorder( searchImage, searchImage, 15, 15, 20, 20, BORDER_CONSTANT, Scalar::all(255) );

				Mat edge_image;
				Mat chamfer_image;
				Canny( searchImage, edge_image, 50, 100);
				threshold( edge_image, chamfer_image, 127, 255, THRESH_BINARY_INV );
				imshow("edges",edge_image);
				distanceTransform( chamfer_image, chamfer_image, CV_DIST_L2, 3);

				// chamfer_image.convertTo(chamfer_image, CV_8U);
				//
				imshow("searchimage",searchImage);

				int closest = -1;
				double closestVal = 1000000;
				for(int k = 0; k < knownSigns.size(); k++) {

					float match = chamfer_match(knownSignsTemplatePoints[k],chamfer_image,Size(chamfer_image.size().width-knownSigns[i].size().width,chamfer_image.size().height-knownSigns[i].size().height));
					maxes[k] = fmax(maxes[k],match);
					mins[k] = fmin(mins[k],match);
					//float backwardsMatch = chamfer_match(edge_image,chamferedKnownSigns[k]);
					cout << "Image " << knownSignFiles[k] << ": " << match << endl;
					// match += backwardsMatch;
					if(match < closestVal) {
						closest = k;
						closestVal = match;
					}
				}
				cout << closestVal << endl;
				cout << closest << endl;
				imshow("bestmatch",knownSigns[closest]);
				waitKey(0);

			}
		}

	}
	for (int i = 0; i < knownSigns.size(); i++) {
		cout << i << " Max: " << maxes[i] << " Min: " << mins[i] << " Size: " << knownSignsTemplatePoints[i].size() << endl;
	}
	return 0;
}
