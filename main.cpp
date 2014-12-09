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

	Mat src; Mat hsv;
	Mat mask;

	vector<Mat> knownSigns(knownSignFiles.size());
	vector<Mat> grayKnownSigns;
	for (int i = 0; i < knownSignFiles.size(); i++) {
		knownSigns[i] = imread(knownSignFiles[i]);
	}


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
		Mat invGrayImage;
		cvtColor( image, invGrayImage, CV_BGR2GRAY);
		bitwise_not(invGrayImage,invGrayImage);
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

				invGrayImage.copyTo(maskedImage, mask);
				Mat searchImage(maskedImage, bounds);
				bitwise_not(maskedImage,maskedImage);

				// Scalar colour( rand()&0xFF,rand()&0xFF,rand()&0xFF );
				// drawContours( image, contours, i, colour, CV_FILLED, 8, hierarchy );
				// rectangle(image,boundRects[boundRects.size()-1],colour,3);
				// Mat searchImage = Mat(image,boundsRects[boundRects.size()-1]);
				// imshow("colorsearch", searchImage);
				resize(searchImage,searchImage,Size(75,75));
				copyMakeBorder( searchImage, searchImage, 20, 20, 30, 30, BORDER_CONSTANT, Scalar::all(255) );

				Mat edge_image;
				Mat chamfer_image;
				Canny( searchImage, edge_image, 50, 100);
				threshold( edge_image, edge_image, 127, 255, THRESH_BINARY_INV );

				distanceTransform( edge_image, chamfer_image, CV_DIST_L2, 3);
				//normalize(chamfer_image, chamfer_image, 0.0, 1.0, NORM_MINMAX, CV_8U);
				//chamfer_image.convertTo(chamfer_image, CV_8U);
				//
				imshow("searchimage",searchImage);
				//
				int closest = -1;
				double closestVal = 1000000;
				for(int k = 0; k < knownSigns.size(); k++) {
					float bestForSign = 0;
					Mat matching_space;
					matching_space.create(searchImage.cols-knownSigns[k].cols+1,searchImage.rows-knownSigns[k].rows+1, CV_8U );

					Mat temp;
					Mat matcher;

					cvtColor(knownSigns[k],matcher,CV_BGR2GRAY);
					Canny(matcher,matcher,100,200);

					matchTemplate( chamfer_image, matcher, matching_space, CV_TM_CCORR_NORMED );
					double min, max;
					minMaxLoc(matching_space,&min,&max);
					if( min < bestForSign ) {
						bestForSign = min;
					}



					if(bestForSign < closestVal) {
						closest = k;
						closestVal = bestForSign;
					}
				}
				cout << closestVal << endl;
				imshow("bestmatch",knownSigns[closest]);
				waitKey(0);

			}
		}
	}
	return 0;
}
