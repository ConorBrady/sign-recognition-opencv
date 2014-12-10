#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>

using namespace cv;
using namespace std;


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

float maxMultiChamferMatch(vector<vector<Point>>templatePoints, vector<Mat> chamferedImages, Size matchingSpace, vector<float> normalisationDivisors) {
	float max = -1;
	for(int i = 0; i < chamferedImages.size(); i++) {
		float newVal = chamferMatch(templatePoints[i],chamferedImages[i],matchingSpace)/normalisationDivisors[i];
		max = fmax(newVal,max);
	}
	return max;
}

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

int getPanelCount(Mat image, bool top) {

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;



	findContours(image, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE );

	int child;
	if(top) {
		child = 0;
	} else {
		child = hierarchy[0][2];
	}
	int idx = child;
	Mat temp(image.size(),CV_8UC3);
	for( ; idx >= 0; idx = hierarchy[idx][0] )
	{
		Scalar color( rand()&255, rand()&255, rand()&255 );
		drawContours( temp, contours, idx, color, CV_FILLED, 8, hierarchy );
	}


	int count = 0;
	while(child >= 0) {
		if(contourArea(contours[child])>50) {
			count++;
		}
		child = hierarchy[child][0];
	}

	return count;
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


	vector<float> chamferWhiteNormalisationDivisors = {
		90.0f,
		65.0f,
		35.0f,
		65.0f,
		55.0f,
		75.7f,
		90.0f,
		90.0f,
		70.0f
	};

	vector<float> chamferBlackNormalisationDivisors = {
		30.0f,
		85.0f,
		80.0f,
		85.0f,
		30.0f,
		45.0f,
		30.0f,
		30.0f,
		20.0f
	};

	vector<Mat> knownSigns(knownSignFiles.size());
	vector<vector<Point>> knownSignsWhiteTemplatePoints;
	vector<vector<Point>> knownSignsBlackTemplatePoints;
	vector<int> signPanelCounts;

	for (int i = 0; i < knownSignFiles.size(); i++) {

		knownSigns[i] = imread(knownSignFiles[i]);

		Mat temp;

		cvtColor(knownSigns[i],temp,CV_BGR2GRAY);

		knownSignsWhiteTemplatePoints.push_back(vector<Point>());
		knownSignsBlackTemplatePoints.push_back(vector<Point>());

		for(int j = 0; j < knownSigns[i].rows; j++) {
			for(int k = 0; k < knownSigns[i].cols; k++) {
				if(temp.at<unsigned char>(j,k) > 200) {
					knownSignsWhiteTemplatePoints[i].push_back(Point(k,j));
				}
				if(temp.at<unsigned char>(j,k) < 50) {
					knownSignsBlackTemplatePoints[i].push_back(Point(k,j));
				}
			}
		}

		threshold(temp, temp, 200, 255, THRESH_BINARY);
		signPanelCounts.push_back(getPanelCount(temp,true));
	}

	for(int i = 0; i < unknownSignFiles.size(); i++) {

		Mat image = imread(unknownSignFiles[i]);

		Mat backproj = getBackProjection("back_project_data.png", image);

		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;

		findContours( backproj, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE );

		for (int j = 0; j < contours.size(); j++)
		{
			if(hierarchy[j][2] >= 0 && hierarchy[j][3] < 0) { // Has child and no parent

				#define MIN_COMPONANT_AREA 200

				vector<Point> contoursPoly;
				approxPolyDP( Mat(contours[j]), contoursPoly, 3, true );
				Rect outerBounds = boundingRect(Mat(contoursPoly));

				Rect bounds;

				int child = hierarchy[j][2];
				while(child >= 0) {
					if(contourArea(contours[child]) > MIN_COMPONANT_AREA) {
						vector<Point> contoursPoly;
						approxPolyDP( Mat(contours[child]), contoursPoly, 3, true );
						if(bounds.area() == 0) {
							bounds = boundingRect(Mat(contoursPoly));
						} else {
							bounds |= boundingRect(Mat(contoursPoly));
						}
					}
					child = hierarchy[child][0];
				}

				if(bounds.area() > 0) {

					Mat mask = Mat::zeros(image.size(), CV_8UC1);
					int child = hierarchy[j][2];
					while(child >= 0) {
						if(contourArea(contours[child]) > MIN_COMPONANT_AREA) {
							drawContours(mask, contours, child, Scalar(255), CV_FILLED, 8, hierarchy, 0);
						}
						child = hierarchy[child][0];
					}


					Mat maskedImage;
					Mat invGrayImage;
					cvtColor( image, invGrayImage, CV_BGR2GRAY);

					Mat unmasked(invGrayImage,bounds);
					Mat boundedMask(mask,bounds);

					vector<Mat> chamferImages;

					int panelCount;

					for(int k = 0; k < 2; k++) {

						Mat searchImage;
						threshold( unmasked, unmasked, -1, 255, ( k==0 ? THRESH_BINARY : THRESH_BINARY_INV ) | THRESH_OTSU );

						unmasked.copyTo(searchImage, boundedMask);
						bitwise_not(searchImage,searchImage);
						resize(searchImage,searchImage,Size(60,60));
						copyMakeBorder( searchImage, searchImage, 25, 25, 30, 30, BORDER_CONSTANT, Scalar::all(255) );
						threshold(searchImage, searchImage, -1, 255, THRESH_BINARY | THRESH_OTSU );
						if(k == 0) {
							panelCount = getPanelCount(searchImage, false);
						}
						distanceTransform( searchImage, searchImage, CV_DIST_L2, 3);
						chamferImages.push_back(searchImage.clone());
					}

					int closest = -1;
					double closestVal = 1000000;
					for(int k = 0; k < knownSigns.size(); k++) {
						if( ( panelCount == 1 && signPanelCounts[k] == 1 ) || ( panelCount > 1 && signPanelCounts[k] > 1 ) ) {
							vector<vector<Point>> templatePoints = {
								knownSignsWhiteTemplatePoints[k],
								knownSignsBlackTemplatePoints[k]
							};

							vector<float> normalisationDivisors = {
								chamferWhiteNormalisationDivisors[k],
								chamferBlackNormalisationDivisors[k]
							};

							float match = maxMultiChamferMatch( templatePoints, chamferImages, chamferImages[0].size()-knownSigns[k].size(), normalisationDivisors);

							if(match < closestVal) {
								closest = k;
								closestVal = match;
							}
						}
					}

					if(closestVal<10) {
						Mat thumbnail;
						resize(knownSigns[closest],thumbnail,Size(30,25));
						Scalar color( 0, 0, 255 );
						rectangle( image, outerBounds, color, 2 );
						cv::Rect roi( cv::Point( outerBounds.x, outerBounds.y ), thumbnail.size() );
						thumbnail.copyTo( image( roi ) );
					}

				}

			}
		}
		imshow("Image",image);
		waitKey(0);
	}

	return 0;
}
