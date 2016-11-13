//
//  MaskGenerator.hpp
//  HomographyImage
//
//  Created by Taro Tokui on 2015/12/28.
//
//

#pragma once

#include "ofMain.h"
#include "ofxCv.h"

static const int NUM_HOMOGRAPHY_POINTS = 4;
static const float HOMOGRAPHY_MARKER_SIZE = 10;

class CMaskGenerator {
public:
	CMaskGenerator() {}
	~CMaskGenerator() {}

	void setup(int depth_w = 512, int depth_h = 424);
	void draw_mask(int _x = 0, int _y = 0);

	uchar* getMaskPixels();

	void drag_nearest_corner(ofVec2f _pos);

	void loadPointData();
	void savePointData();

	ofVec2f* getHomographyPoints() { return dst_point; }

private:

	void check_nearest_homography_point(ofVec2f _pos);
	float calcDistance(float dx, float dy);
	void update_homography();
	void update_fbo();

	// memberïœêî
	ofVec2f src_point[NUM_HOMOGRAPHY_POINTS];
	ofVec2f dst_point[NUM_HOMOGRAPHY_POINTS];
	bool bSavePoints;
	ofXml XML;

	ofImage maskImageOriginal;
	ofImage maskImage;

	cv::Mat homography;

	ofFbo maskFbo;
	ofPixels pixels;

};