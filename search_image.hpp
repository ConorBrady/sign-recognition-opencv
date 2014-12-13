#include "region_of_interest.hpp"

#include <vector>
#include <string>

class SearchImage {
private:
	Mat _image;
	string _ident;
	Mat _getBackProjection(Mat backProjectionData);
public:
	SearchImage(const char * filename);

	vector<RegionOfInterest> roisForBackProjection(Mat backprojection);
	Mat image();
	string ident();

};
