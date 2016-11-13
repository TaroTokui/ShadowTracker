#pragma once

#include <iostream>

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxOpenCv.h"

static const int MAX_ACTIVE_BLOBS = 50;
static const int MAX_STATIC_BLOBS = 50;
static const int MAX_TOUCH_BLOBS = 50;
static const int HEAT_MAP_RESOLUTION = 10000;

class CImageProcessing {
public:
	CImageProcessing()
	{
		// activeObjectsのメモリを確保
		for (int i = 0; i < MAX_ACTIVE_BLOBS; i++)
		{
			activeObjects.push_back(ofVec3f(0, 0, 0));
		}
		// staticObjectsのメモリを確保
		for (int i = 0; i < MAX_STATIC_BLOBS; i++)
		{
			staticObjects.push_back(ofVec3f(0, 0, 0));
		}
		// touchObjectsのメモリを確保
		for (int i = 0; i < MAX_TOUCH_BLOBS; i++)
		{
			touchObjects.push_back(ofVec3f(0, 0, 0));
		}
	}

	~CImageProcessing()
	{
	}

	bool setup(int w, int h);
	void update(cv::Mat* newFrame); 
	void draw_test();
	void show_bg_image(int _x = 0, int _y = 0);
	void show_mask_image();
	void show_front_image(int _x = 0, int _y = 0);
	void show_denoised_front_image();
	void show_contours_image();
	void show_debug_image(int _x=0, int _y=0);
	void show_static_map_image(int _x = 0, int _y = 0);

	void calcBackground();
	void resetBackground();

	void findActiveObject();
	void findStaticObject();
	void findTouchObject();

	// kinectのdepth画像の座標系
	bool isTouchArea(int depth_x, int depth_y);

	void setMaskImage(uchar* pixels);

	vector<ofVec3f> getActiveObjects() { return activeObjects; }
	vector<ofVec3f> getStaticObjects() { return staticObjects; }
	vector<ofVec3f> getTouchObjects() { return touchObjects; }

	void startDebugMode() { bDebugMode = true; }
	void stopDebugMode() { bDebugMode = false; }

	ofParameterGroup params;
private:
	int depthWidth, depthHeight;
	unsigned int depthBufferSize;

	bool bDebugMode;

	// buffer
	cv::Mat *depthMat16;// (depthHeight, depthWidth, CV_16SC1);
	cv::Mat *depthMat;// (depthHeight, depthWidth, CV_8UC1);
	cv::Mat *bgMat16;
	cv::Mat *maskMat;
	cv::Mat *dstMat16;
	cv::Mat *frontMat;
	cv::Mat *destMat;
	cv::Mat *tmpMat;
	cv::Mat *staticMat16;
	cv::Mat *staticMat;
	cv::Mat *touchMat;
	//int* staticPixels;
	cv::Mat erosion_element;
	ofxCvGrayscaleImage grayscaleImage;
	ofxCvGrayscaleImage depthImage;
	ofxCvGrayscaleImage bgImage;

	// cv objects
	ofxCv::ContourFinder contourFinder;
	//ofVec3f activeObjects[MAX_ACTIVE_BLOBS];	// x, y, radius
	//ofVec3f staticObjects[MAX_STATIC_BLOBS];
	vector<ofVec3f> activeObjects;
	vector<ofVec3f> staticObjects;
	vector<ofVec3f> touchObjects;

	// GUI params
	// sensing params
	ofParameter<int> cut_off_threshold;
	ofParameter<int> bg_diff;
	ofParameter<int> sensing_range;
	ofParameter<int> touch_range;

	//// tracking object params
	//ofParameter<int> minAreaRadius;
	//ofParameter<int> maxAreaRadius;

	//// heat map params
	//ofParameter<int> heatmapIncrease;
	//ofParameter<float> heatmapDecrease;

	void findObject(cv::Mat *inputMat, vector<ofVec3f> &outputObjects, int object_size);
};
