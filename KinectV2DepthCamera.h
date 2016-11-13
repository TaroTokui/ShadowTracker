#pragma once

#include <iostream>
#include <kinect.h>

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxOpenCv.h"

#pragma comment(lib, "Kinect20.lib")

class CKinectV2DepthCamera {

public:
	CKinectV2DepthCamera()
	{
	}

	~CKinectV2DepthCamera()
	{
	}

	bool setup();
	void draw_test();
	void draw_IR_image(int _x=0, int _y=0);

	void grabNewFrame();
	void grabNewFrameIR();

	// 画像処理が終わってからリリースすること
	void releaseFrame();

	cv::Mat* getDepthData16();
	int getWidth() { return depthWidth; }
	int getHeight() { return depthHeight; }

	bool newFrameIsAvailable();

private:
	HRESULT hResult;

	IDepthFrame* pDepthFrame;
	IInfraredFrame* pInfraredFrame;

	// kinect v2 用の変数
	IKinectSensor* pSensor;
	// depth
	IDepthFrameSource* pDepthSource;
	IDepthFrameReader* pDepthReader;
	IFrameDescription* pDepthDescription;
	ICoordinateMapper* pCoordinateMapper;
	UINT16 *pBuffer;
	// IR
	IInfraredFrameSource* pInfraredSource;
	IInfraredFrameReader* pInfraredReader;
	IFrameDescription* pInfraredDescription;

	int depthWidth, depthHeight;
	unsigned int depthBufferSize;

	// buffer
	cv::Mat *bufferMat;// (depthHeight, depthWidth, CV_16SC1);
	cv::Mat *depthMat;// (depthHeight, depthWidth, CV_8UC1);
	cv::Mat *infraredMat;// (depthHeight, depthWidth, CV_8UC1);
	ofxCvGrayscaleImage grayscaleImage;

};
