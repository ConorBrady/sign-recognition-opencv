#include <opencv2/opencv.hpp>
#include <iostream>
int main(){
	cv::Mat image = cv::imread("signs.png",-1);
	cv::Mat planes[4];
	cv::split(image,planes);
	cv::threshold(planes[3],planes[3],250,255,cv::THRESH_BINARY);
	int size = (int)sqrt(cv::countNonZero(planes[3]))-10;
	cv::Mat newImage(size,size,CV_8UC4);
	int cnp = 0;
	for(int i = 0; i < image.cols && cnp < size*size; i++) {
		for(int j = 0; j < image.col(i).rows && cnp < size*size; j++) {
			if( image.col(i).row(j).data[3] == 255  ) {

				cv::Vec4b color = image.at<cv::Vec4b>(cv::Point(i,j));
				newImage.at<cv::Vec4b>(cv::Point(cnp/size,cnp%size)) = color;
				cnp ++;
			}
		}
	}
	cv::imshow("window",newImage);
	cv::waitKey(0);
	cv::imwrite("../back_project_data.png",newImage);
}
