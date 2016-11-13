#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxBezierWarpManager.h"

#include "KinectV2DepthCamera.h"
#include "ImageProcessing.h"
#include "MaskGenerator.h"

constexpr int WINDOW_OFFSET_X = 20;
constexpr int WINDOW_OFFSET_Y = 20;
constexpr int SCREEN_WIDTH = 1920;
constexpr int SCREEN_HEIGHT = 1080;

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void exit();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
private:
	CKinectV2DepthCamera kinectV2;
	CImageProcessing imageProcessing;
	unsigned long long bgCalcStartTime;
	CMaskGenerator maskGenerator;

	// camera size
	int depthWidth, depthHeight;

	// GUI
	ofxPanel gui;
	ofParameterGroup params, params_state;

	// GUI params
	ofParameter<bool> bSetBg;
	ofParameter<bool> bShowMask;
	ofParameter<bool> bShowIrImage;
	ofParameter<int> bgAddDuration;
	ofParameter<bool> bHolizontalMirror;
	bool bShowGui;

	void setup_gui();
	void drawInfo(int x, int y, string m);
	void drawOutline(int x, int y, int w, int h);

	// mapper
	ofxBezierWarpManager bezManager;
	ofFbo fbo;
};
