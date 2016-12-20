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
	warpFbo.allocate(depthWidth, depthHeight);
	shadowFbo.allocate(SHADOW_WIDTH, SHADOW_HEIGHT);

	// init mapper
	bezManager.setup(10); //WarpResolution
	bezManager.addFbo(&warpFbo);
	bezManager.toggleGuideVisible();

	bezManager.loadSettings();

	currentMode = MODE_DEPTH_SETTING;
}

//--------------------------------------------------------------
void ofApp::update(){

	update_kinect();
	calc_front_image();
	update_warp_fbo();
	update_shadow_fbo();

	//Spout
	spout.sendTexture(shadowFbo.getTexture(), "fromOF");

	switch (currentMode)
	{
	case MODE_DEPTH_SETTING:
		break;

	case MODE_WARP_SETTING:
		break;

	case MODE_IMAGE_PROCESSING:
		break;

	default:
		break;
	}

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

	// mode
	string text = "mode: ";
	switch (currentMode)
	{
	case MODE_DEPTH_SETTING:
		text += "depth setting";

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
		window_y = offset_y;

		imageProcessing.show_front_image(window_x, window_y);

		drawInfo(window_x, window_y, "front image");
		drawOutline(window_x, window_y, depthWidth, depthHeight);

		// shadow image
		window_x = depthWidth + offset_x * 2;
		window_y = depthHeight + offset_y * 2;

		shadowFbo.draw(window_x, window_y, SHADOW_WIDTH, SHADOW_HEIGHT);
		drawInfo(window_x, window_y, "shadow image");
		drawOutline(window_x, window_y, SHADOW_WIDTH, SHADOW_HEIGHT);

		break;

	case MODE_WARP_SETTING:
		text += "warp setting";

		ofPushStyle();
		// draw bezier
		ofSetColor(255, 255, 255, 255);
		bezManager.draw();

		window_x = depthWidth + offset_x * 2;
		window_y = offset_y;
		ofSetColor(255, 0, 0);
		drawOutline((ofGetWindowWidth() - SHADOW_WIDTH) / 2,
					(ofGetWindowHeight() - SHADOW_HEIGHT ) / 2,
					SHADOW_WIDTH, SHADOW_HEIGHT);
		ofPopStyle();
		break;

	case MODE_IMAGE_PROCESSING:
		text += "image processing";
		break;

	default:
		break;
	}

	drawInfo(0, 0, text);
	gui.draw();
}

//--------------------------------------------------------------
void ofApp::exit() {
	gui.saveToFile("settings.xml");
	maskGenerator.savePointData();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	// send key event
	if (currentMode == MODE_WARP_SETTING)
	{
		bezManager.keyPressed(key);
	}

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
	case ' ':
		toggle_mode();
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

	// send drag event
	if (currentMode == MODE_WARP_SETTING)
	{
		bezManager.mouseDragged(x, y, button);
	}
	else {
		maskGenerator.drag_nearest_corner(ofVec2f(x - WINDOW_OFFSET_X, y - WINDOW_OFFSET_Y));
		imageProcessing.setMaskImage(maskGenerator.getMaskPixels());

	}
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

	// send press event
	if (currentMode == MODE_WARP_SETTING)
	{
		bezManager.mousePressed(x, y, button);
	}
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
	//ofSetColor(128);
	ofSetColor(255,0,0);
	ofNoFill();
	ofRect(x, y, w, h);
	ofSetColor(255);
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::toggle_mode()
{
	switch (currentMode)
	{
	case MODE_DEPTH_SETTING:
		currentMode = MODE_WARP_SETTING;
		break;
	case MODE_WARP_SETTING:
		currentMode = MODE_IMAGE_PROCESSING;
		break;
	case MODE_IMAGE_PROCESSING:
		currentMode = MODE_DEPTH_SETTING;
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::update_kinect()
{
	if (bShowIrImage)
	{
		kinectV2.grabNewFrameIR();
	}
	else
	{
		kinectV2.grabNewFrame();
	}
}

//--------------------------------------------------------------
void ofApp::check_bg_mode()
{
	if (bgCalcStartTime + bgAddDuration * 1000 > ofGetElapsedTimeMillis())
	{
		bSetBg.set(true);
	}
	else {
		bSetBg.set(false);
	}
}

//--------------------------------------------------------------
void ofApp::calc_front_image()
{
	// calc front image
	if (kinectV2.newFrameIsAvailable())
	{
		imageProcessing.update(kinectV2.getDepthData16());

		check_bg_mode();

		if (bSetBg)
		{
			imageProcessing.calcBackground();
		}
		else
		{
			imageProcessing.calcFrontArea();
		}
	}
}

//--------------------------------------------------------------
void ofApp::update_warp_fbo()
{
	// update fbo
	warpFbo.begin();
	ofPushStyle();
	ofBackground(0);
	ofSetColor(255);
	ofFill();
	//imageProcessing.show_debug_image(0, 0);
	imageProcessing.show_front_image(0, 0);
	ofPopStyle();
	warpFbo.end();
}

//--------------------------------------------------------------
void ofApp::update_shadow_fbo()
{
	shadowFbo.begin();
	ofBackground(0);
	ofPushMatrix();
	ofTranslate((SHADOW_WIDTH - 1920) / 2, (SHADOW_HEIGHT - 1080) / 2);
	bezManager.draw();
	ofPopMatrix();

	// for debug
	// todo: delete this line
	//ofDrawCircle(SHADOW_WIDTH / 2, SHADOW_HEIGHT / 2, 100);

	shadowFbo.end();
}