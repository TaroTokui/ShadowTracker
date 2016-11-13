//
//  MaskGenerator.cpp
//  HomographyImage
//
//  Created by Taro Tokui on 2015/12/28.
//
//

#include "MaskGenerator.h"

using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
void CMaskGenerator::setup(int depth_w, int depth_h) {

	//maskImageOriginal.load("table_shape.png");
	maskImageOriginal.load("mask.png");
	imitate(maskImage, maskImageOriginal);

	loadPointData();

	int w = maskImageOriginal.getWidth();
	int h = maskImageOriginal.getHeight();

	src_point[0] = ofVec2f(0, 0);
	src_point[1] = ofVec2f(w, 0);
	src_point[2] = ofVec2f(w, h);
	src_point[3] = ofVec2f(0, h);

	update_homography();

	warpPerspective(maskImageOriginal, maskImage, homography, CV_INTER_LINEAR);
	maskImage.update();

	maskFbo.allocate(depth_w, depth_h, GL_RGBA);
	maskFbo.begin();
	ofClear(255, 255, 255, 0);
	maskFbo.end();

	update_fbo();
}

//--------------------------------------------------------------
void CMaskGenerator::draw_mask(int _x, int _y)
{
	//maskFbo.draw(_x, _y);

	ofPushStyle();
	ofNoFill();

	// 1点目
	ofSetColor(255, 0, 0);
	ofDrawCircle(dst_point[0] + ofVec2f(_x, _y), HOMOGRAPHY_MARKER_SIZE);
	ofDrawLine(dst_point[0] + ofVec2f(_x, _y), dst_point[1] + ofVec2f(_x, _y));

	// 2点目
	ofSetColor(0, 255, 0);
	ofDrawCircle(dst_point[1] + ofVec2f(_x, _y), HOMOGRAPHY_MARKER_SIZE);
	ofDrawLine(dst_point[1] + ofVec2f(_x, _y), dst_point[2] + ofVec2f(_x, _y));

	// 3点目
	ofSetColor(0, 0, 255);
	ofDrawCircle(dst_point[2] + ofVec2f(_x, _y), HOMOGRAPHY_MARKER_SIZE);
	ofDrawLine(dst_point[2] + ofVec2f(_x, _y), dst_point[3] + ofVec2f(_x, _y));

	// 4点目
	ofSetColor(255, 255, 0);
	ofDrawCircle(dst_point[3] + ofVec2f(_x, _y), HOMOGRAPHY_MARKER_SIZE);
	ofDrawLine(dst_point[3] + ofVec2f(_x, _y), dst_point[0] + ofVec2f(_x, _y));

	ofPopStyle();
}

//--------------------------------------------------------------
uchar* CMaskGenerator::getMaskPixels()
{
	maskFbo.readToPixels(pixels);
	return pixels.getData();
}

//--------------------------------------------------------------
void CMaskGenerator::drag_nearest_corner(ofVec2f _pos)
{
	check_nearest_homography_point(_pos);
	update_homography();

	warpPerspective(maskImageOriginal, maskImage, homography, CV_INTER_LINEAR);
	maskImage.update();

	update_fbo();

}

//--------------------------------------------------------------
void CMaskGenerator::loadPointData()
{
	if (XML.load("HomographySettings.xml")) {
		cout << "HomographySettings.xml loaded!" << endl;
	}
	else {
		XML.addChild("POINTS");
		XML.setTo("POINTS");

		cout << "unable to load mySettings.xml check data/ folder" << endl;
	}
	if (XML.exists("POINTS"))
	{
		XML.setTo("POINTS[0]");

		do {
			if (XML.getName() == "POINTS" && XML.setTo("PT[0]"))
			{
				// get each individual x,y for each point
				do {
					int index = XML.getValue<int>("INDEX");
					if (index > NUM_HOMOGRAPHY_POINTS - 1)
					{
						continue;
					}

					int x = XML.getValue<int>("X");
					int y = XML.getValue<int>("Y");
					dst_point[index] = ofVec2f(x, y);
				} while (XML.setToSibling()); // go the next PT

											 // go back up
				XML.setToParent();
			}

		} while (XML.setToSibling());
	}
}

void CMaskGenerator::savePointData()
{
	XML.clear();

	XML.addChild("HOMOGRAPHY");
	XML.setTo("//HOMOGRAPHY");

	XML.addChild("POINTS");
	XML.setTo("//POINTS");

	for (int i = 0; i<NUM_HOMOGRAPHY_POINTS; i++)
	{
		ofXml point;
		point.addChild("PT");
		point.setTo("PT");

		point.addValue("INDEX", i);
		point.addValue("X", int(dst_point[i].x));
		point.addValue("Y", int(dst_point[i].y));

		XML.addXml(point);
	}

	XML.save("HomographySettings.xml");

	cout << "settings saved to xml!" << endl;
}

//--------------------------------------------------------------
void CMaskGenerator::check_nearest_homography_point(ofVec2f _pos) {

	for (int i = 0; i<NUM_HOMOGRAPHY_POINTS; i++)
	{
		if (calcDistance(dst_point[i].x - _pos.x, dst_point[i].y - _pos.y) < HOMOGRAPHY_MARKER_SIZE * 4)
		{
			dst_point[i] = _pos;
			return;
		}
	}
}

//--------------------------------------------------------------
/* 距離の計算 */
float CMaskGenerator::calcDistance(float dx, float dy) {

	float distance;

	/* d = ( x^2 + y^2 )^(1/2) */
	distance = sqrt(dx*dx + dy*dy);

	return distance;
}

//--------------------------------------------------------------
void CMaskGenerator::update_homography()
{
	vector<Point2f> srcPoints, dstPoints;
	for (int i = 0; i < NUM_HOMOGRAPHY_POINTS; i++) {
		srcPoints.push_back(Point2f(src_point[i].x, src_point[i].y));
		dstPoints.push_back(Point2f(dst_point[i].x, dst_point[i].y));
	}

	homography = findHomography(Mat(srcPoints), Mat(dstPoints));
}

//--------------------------------------------------------------
void CMaskGenerator::update_fbo()
{
	maskFbo.begin();

	ofBackground(0);
	maskImage.draw(0, 0);

	maskFbo.end();
}