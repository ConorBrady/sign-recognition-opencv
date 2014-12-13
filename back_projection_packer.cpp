#include "back_projection_packer.hpp"

Mat BackProjectionPacker::pack(const char* filename) {
	Mat image = cv::imread(filename,-1);
	Mat planes[4];
	split(image,planes);
	threshold(planes[3],planes[3],250,255,cv::THRESH_BINARY);
	int size = (int)sqrt(cv::countNonZero(planes[3]))-10;
	Mat newImage(size,size,CV_8UC4);
	int cnp = 0;
	for(int i = 0; i < image.cols && cnp < size*size; i++) {
		for(int j = 0; j < image.col(i).rows && cnp < size*size; j++) {
			if( image.col(i).row(j).data[3] == 255  ) {

				Vec4b color = image.at<cv::Vec4b>(cv::Point(i,j));
				newImage.at<cv::Vec4b>(cv::Point(cnp/size,cnp%size)) = color;
				cnp ++;
			}
		}
	}
	return newImage;
}
