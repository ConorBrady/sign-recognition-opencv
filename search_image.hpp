#include "region_of_interest.hpp"

#include <vector>

class SearchImage {
private:
	Mat _image;

public:
	SearchImage(const char * filename);

	vector<RegionOfInterest> roisForBackProjection(const char * filename);
	Mat image();

};
