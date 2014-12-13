#define ACCEPTANCE_MARK 10

#include "known_sign.hpp"
#include "search_image.hpp"
#include "back_projection_packer.hpp"

int main()
{

	vector<KnownSign> knownSigns {
		KnownSign("known_signs/left.png", 90, 30),
		KnownSign("known_signs/no_left.png", 65, 85),
		KnownSign("known_signs/no_parking.png", 35, 80),
		KnownSign("known_signs/no_right.png", 65, 85),
		KnownSign("known_signs/no_straight.png", 55, 30),
		KnownSign("known_signs/parking.png", 75, 45),
		KnownSign("known_signs/right.png", 90, 30),
		KnownSign("known_signs/straight.png", 90, 30),
		KnownSign("known_signs/yield.png", 70, 20)
	};

	vector<SearchImage> searchImages {
		SearchImage("unknown_signs/1.png"),
		SearchImage("unknown_signs/2.png"),
		SearchImage("unknown_signs/3.png"),
		SearchImage("unknown_signs/C1.png"),
		SearchImage("unknown_signs/C2.png")
	};

	Mat backProjectData = BackProjectionPacker::pack("signs.png");

#ifdef GEN
	imwrite(string("gen/backprojectdata.png"),backProjectData);
#endif
	for( SearchImage& searchImage : searchImages ) {

		for ( RegionOfInterest& roi : searchImage.roisForBackProjection(backProjectData) ) {

			KnownSign* closest = nullptr;
			float closestVal;

			for( KnownSign& knownSign : knownSigns ) {

				if( ( roi.panelCount() == 1 && knownSign.panelCount() == 1 ) ||
					( roi.panelCount() >  1 && knownSign.panelCount() >  1 ) ) {

					float match = knownSign.match(roi.whiteObjPix(), roi.blackObjPix());

					if(closest == nullptr || match < closestVal) {
						closest = &knownSign;
						closestVal = match;
					}
				}
			}

			if( closestVal < ACCEPTANCE_MARK ) {

				Mat image = searchImage.image();
				rectangle( image, roi.outerBounds(), Scalar( 0, 0, 255 ), 2 );

				Rect thumbnailPlace( Point( roi.outerBounds().x, roi.outerBounds().y ), closest->thumbnail().size() );
				closest->thumbnail().copyTo( searchImage.image()(thumbnailPlace) );
			}
		}
#ifdef GEN
		imwrite(string("gen/")+searchImage.ident()+".png",searchImage.image());
#else
		imshow("Image",searchImage.image());
		waitKey(0);
#endif
	}

	return 0;
}
