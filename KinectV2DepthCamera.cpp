#include "KinectV2DepthCamera.h"

using namespace cv;

//--------------------------------------------------------------
bool CKinectV2DepthCamera::setup()
{
	hResult = S_OK;

	// Open Kinect
	hResult = GetDefaultKinectSensor(&this->pSensor);
	if (FAILED(hResult)) {
		std::cerr << "Error : GetDefaultKinectSensor" << std::endl;
		return false;
	}
	hResult = this->pSensor->Open();
	if (FAILED(hResult)) {
		std::cerr << "Error : IKinectSensor::Open()" << std::endl;
		return false;
	}

	// Open Source
	hResult = pSensor->get_DepthFrameSource(&this->pDepthSource);
	if (FAILED(hResult)) {
		std::cerr << "Error : IKinectSensor::get_DepthFrameSource()" << std::endl;
		return false;
	}
	hResult = pSensor->get_InfraredFrameSource(&this->pInfraredSource);
	if (FAILED(hResult)) {
		std::cerr << "Error : IKinectSensor::get_InfraredFrameSource()" << std::endl;
		return false;
	}

	// Open Reader
	hResult = this->pDepthSource->OpenReader(&this->pDepthReader);
	if (FAILED(hResult)) {
		std::cerr << "Error : IDepthFrameSource::OpenReader()" << std::endl;
		return false;
	}
	hResult = this->pInfraredSource->OpenReader(&this->pInfraredReader);
	if (FAILED(hResult)) {
		std::cerr << "Error : IInfraredFrameSource::OpenReader()" << std::endl;
		return false;
	}

	// get descriptions
	hResult = pDepthSource->get_FrameDescription(&this->pDepthDescription);
	if (FAILED(hResult)) {
		std::cerr << "Error : IDepthFrameSource::get_FrameDescription()" << std::endl;
		return false;
	}
	hResult = pInfraredSource->get_FrameDescription(&this->pInfraredDescription);
	if (FAILED(hResult)) {
		std::cerr << "Error : IDepthFrameSource::get_FrameDescription()" << std::endl;
		return false;
	}

	// get coordinate mapper
	hResult = this->pSensor->get_CoordinateMapper(&this->pCoordinateMapper);
	if (FAILED(hResult)) {
		std::cerr << "Error : IKinectSensor::get_CoordinateMapper()" << std::endl;
		return false;
	}
	this->pDepthDescription->get_Width(&depthWidth); // 512
	this->pDepthDescription->get_Height(&depthHeight); // 424
	this->depthBufferSize = depthWidth * depthHeight * sizeof(unsigned short);

	cout << "Kinect V2 has initialized" << endl;

	// init buffers
	pDepthFrame = NULL;
	pInfraredFrame = NULL;
	bufferMat = new Mat(depthHeight, depthWidth, CV_16SC1);
	depthMat = new Mat(depthHeight, depthWidth, CV_8UC1);
	infraredMat = new Mat(depthHeight, depthWidth, CV_8UC1);
	pBuffer = NULL;

	// init CV object
	grayscaleImage.allocate(depthHeight, depthWidth);

	//startThread();
	return true;
}

//--------------------------------------------------------------
void CKinectV2DepthCamera::draw_test()
{
	bufferMat->convertTo(*depthMat, CV_8U, -255.0f / 4500.0f, 255.0f);
	grayscaleImage.setFromPixels(depthMat->data, depthWidth, depthHeight);
	grayscaleImage.draw(0, 0);
}

//--------------------------------------------------------------
void CKinectV2DepthCamera::draw_IR_image(int _x, int _y)
{
	bufferMat->convertTo(*infraredMat, CV_8U, 255.0f / 4500.0f, 0.0f);
	grayscaleImage.setFromPixels(infraredMat->data, depthWidth, depthHeight);
	grayscaleImage.draw(_x, _y);
}

//--------------------------------------------------------------
Mat* CKinectV2DepthCamera::getDepthData16()
{
	return bufferMat;
}

//--------------------------------------------------------------
bool CKinectV2DepthCamera::newFrameIsAvailable()
{
	// この処理いる？
	if (bufferMat->data != NULL)
	{
		return true;
	}
	else {
		return false;
	}
}

//--------------------------------------------------------------
void CKinectV2DepthCamera::grabNewFrame()
{
	// depthFrameを開放
	if (pDepthFrame != NULL)
	{
		pDepthFrame->Release();
	}

	// 新しいフレームを取得
	hResult = E_PENDING;
	while (FAILED(hResult))
	{
		hResult = pDepthReader->AcquireLatestFrame(&pDepthFrame);
		ofSleepMillis(1);
	}

//	hResult = pDepthFrame->AccessUnderlyingBuffer(&depthBufferSize, &pBuffer);
	hResult = pDepthFrame->AccessUnderlyingBuffer(&depthBufferSize, reinterpret_cast<UINT16**>(&bufferMat->data));
	if (FAILED(hResult))
	{
		return;
	}
}

//--------------------------------------------------------------
void CKinectV2DepthCamera::grabNewFrameIR()
{
	// depthFrameを開放
	if (pInfraredFrame != NULL)
	{
		pInfraredFrame->Release();
	}

	// 新しいフレームを取得
	hResult = E_PENDING;
	while (FAILED(hResult))
	{
		hResult = pInfraredReader->AcquireLatestFrame(&pInfraredFrame);
		ofSleepMillis(1);
	}

	hResult = pInfraredFrame->AccessUnderlyingBuffer(&depthBufferSize, reinterpret_cast<UINT16**>(&bufferMat->data));
	if (FAILED(hResult))
	{
		return;
	}
}

//--------------------------------------------------------------
void CKinectV2DepthCamera::releaseFrame()
{
	// depthFrameを開放
	if (pDepthFrame != NULL)
	{
		pDepthFrame->Release();
	}
}