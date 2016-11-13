#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	// init kinect
	if (!kinectV2.setup())
	{
		exit();
		return;
	}
	depthWidth = kinectV2.getWidth();
	depthHeight = kinectV2.getHeight();

	// init mask image
	maskGenerator.setup(depthWidth, depthHeight);

	if (!imageProcessing.setup(depthWidth, depthHeight))
	{
		exit();
	}
	imageProcessing.setMaskImage(maskGenerator.getMaskPixels());

	setup_gui();

	// init bg image
	imageProcessing.resetBackground();
	bgCalcStartTime = ofGetElapsedTimeMillis();

	// init fbo
	fbo.allocate(depthWidth, depthHeight);
	bezManager.setup(10); //WarpResolution
	bezManager.addFbo(&fbo);

	bezManager.loadSettings();
}

//--------------------------------------------------------------
void ofApp::update(){

	// set background
	if (bgCalcStartTime + bgAddDuration * 1000 > ofGetElapsedTimeMillis())
	{
		bSetBg.set(true);
	}
	else {
		bSetBg.set(false);
	}

	if (bShowIrImage)
	{
		kinectV2.grabNewFrameIR();
	}
	else
	{
		kinectV2.grabNewFrame();
	}
	
	// calc front image
	if (kinectV2.newFrameIsAvailable())
	{
		imageProcessing.update(kinectV2.getDepthData16());

		if (bSetBg)
		{
			imageProcessing.calcBackground();
		}
		else
		{
			imageProcessing.findActiveObject();
		}
	}

	// update fbo
	fbo.begin();
	ofBackground(0);
	ofSetColor(255);
	ofFill();
	imageProcessing.show_debug_image(0, 0);
	//ofDrawRectangle(0, 0, depthWidth, depthHeight);
	fbo.end();

}

//--------------------------------------------------------------
void ofApp::draw(){
	ofClear(0);
	ofBackground(0);

	int offset_x = WINDOW_OFFSET_X;
	int offset_y = WINDOW_OFFSET_Y;

	// set window position
	int window_x = offset_x;
	int window_y = offset_y;

	if (bShowIrImage)
	{
		kinectV2.draw_IR_image(window_x, window_y);
	}
	else
	{
		// show raw depth image
		imageProcessing.show_debug_image(window_x, window_y);
	}
	

	drawInfo(window_x, window_y, "depth image");
	drawOutline(window_x, window_y, depthWidth, depthHeight);

	if (bShowMask)
	{
		ofPushStyle();
		ofSetColor(255, 128);
		maskGenerator.draw_mask(window_x, window_y);
		ofPopStyle();
	}

	// bg image
	window_x = offset_x;
	window_y = depthHeight + offset_y * 2;

	imageProcessing.show_bg_image(window_x, window_y);

	drawInfo(window_x, window_y, "background image");
	drawInfo(window_x, window_y + 18, "b: reset background image");
	drawOutline(window_x, window_y, depthWidth, depthHeight);

	// front image
	window_x = depthWidth + offset_x * 2;
	window_y = depthHeight + offset_y * 2;

	imageProcessing.show_front_image(window_x, window_y);

	drawInfo(window_x, window_y, "background image");
	drawInfo(window_x, window_y + 18, "b: reset background image");
	drawOutline(window_x, window_y, depthWidth, depthHeight);
	gui.draw();

	// draw bezier
	ofSetColor(255, 255, 255, 255);
	bezManager.draw();

	window_x = depthWidth + offset_x * 2;
	window_y = offset_y;
	drawOutline(window_x, window_y, 768, 432);
}

//--------------------------------------------------------------
void ofApp::exit() {
	gui.saveToFile("settings.xml");
	maskGenerator.savePointData();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	// send key event
	bezManager.keyPressed(key);

	switch (key)
	{
	case 'b':
		imageProcessing.resetBackground();
		bgCalcStartTime = ofGetElapsedTimeMillis();
		break;
	case 'f':
		ofToggleFullscreen();
		break;
	case OF_KEY_RETURN:
		bezManager.toggleGuideVisible();
		break;
	case 's':
		bezManager.saveSettings();
		break;
	case 'l':
		bezManager.loadSettings();
		break;

	default:
		break;
	}

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

	maskGenerator.drag_nearest_corner(ofVec2f(x - WINDOW_OFFSET_X, y - WINDOW_OFFSET_Y));
	imageProcessing.setMaskImage(maskGenerator.getMaskPixels());

	// send drag event
	bezManager.mouseDragged(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

	// send press event
	bezManager.mousePressed(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
void ofApp::setup_gui() {

	// set gui
	params.setName("settings");

	params_state.setName("recognition state");
	//params_state.add(recognition_mode.set("start recognition", false));
	params_state.add(bSetBg.set("set bg", false));
	params_state.add(bShowMask.set("show mask area", true));
	params_state.add(bShowIrImage.set("show ir image", false));
	params_state.add(bgAddDuration.set("calc bg duration", 1, 0, 10));
	params_state.add(bHolizontalMirror.set("horizontal mirror", false));
	//params_state.add(offset_x.set("offset x", 0.0, -0.1, 0.1));
	//params_state.add(offset_y.set("offset y", 0.0, -0.1, 0.1));
	//params_state.add(offset_r.set("offset radius", 0.0, -0.1, 0.1));

	params.add(params_state);
	params.add(imageProcessing.params);

	gui.setup(params);
	bShowGui = true;

	gui.loadFromFile("settings.xml");

	gui.setPosition(ofGetWindowWidth() - gui.getWidth() - 10, 10);
}

//--------------------------------------------------------------
void ofApp::drawInfo(int x, int y, string m)
{
	ofPushStyle();
	ofSetColor(0, 128);
	ofFill();
	ofDrawRectangle(x, y, depthWidth, 18);
	ofPopStyle();

	ofSetColor(255);
	ofDrawBitmapString(m, x + 5, y + 14);
}

//--------------------------------------------------------------
void ofApp::drawOutline(int x, int y, int w, int h)
{
	ofPushStyle();
	ofSetColor(128);
	ofNoFill();
	ofRect(x, y, w, h);
	ofSetColor(255);
	ofPopStyle();
}