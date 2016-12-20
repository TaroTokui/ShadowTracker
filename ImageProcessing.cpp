#include "ImageProcessing.h"

using namespace cv;

//--------------------------------------------------------------
bool CImageProcessing::setup(int w, int h)
{
	depthWidth = w;
	depthHeight = h;

	// init object
	grayscaleImage.allocate(depthHeight, depthWidth);
	depthMat = new Mat(depthHeight, depthWidth, CV_8UC1);
	depthMat16 = new Mat(depthHeight, depthWidth, CV_16SC1);
	bgMat16 = new Mat(depthHeight, depthWidth, CV_16SC1);
	maskMat = new Mat(depthHeight, depthWidth, CV_8UC1);
	dstMat16 = new Mat(depthHeight, depthWidth, CV_16SC1);
	frontMat = new Mat(depthHeight, depthWidth, CV_8UC1);
	destMat = new Mat(depthHeight, depthWidth, CV_8UC1);
	tmpMat = new Mat(depthHeight, depthWidth, CV_8UC1);
	staticMat16 = new Mat(depthHeight, depthWidth, CV_16SC1);
	staticMat = new Mat(depthHeight, depthWidth, CV_8UC1);
	touchMat = new Mat(depthHeight, depthWidth, CV_8UC1);
	//staticPixels = (int*)malloc(depthWidth*depthHeight);

	// Create a structuring element
	int erosion_size = 1;
	erosion_element = getStructuringElement(cv::MORPH_RECT,
		cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
		cv::Point(erosion_size, erosion_size));

	// init gui params
	params.setName("sensing params");
	params.add(sensing_range.set("sensing range", 1000, 1, 7000));
	//params.add(touch_range.set("touch range", 100, 1, 1000));
	params.add(cut_off_threshold.set("nearest distance", 4000, 0, 7000));
	params.add(bg_diff.set("bg offset", 10, 0, 500));
	//params.add(minAreaRadius.set("min blob radius", 10, 0, 50));
	//params.add(maxAreaRadius.set("max blob radius", 150, 1, 500));
	//params.add(heatmapIncrease.set("heatmap increase", 10, 1, 200));
	//params.add(heatmapDecrease.set("heatmap decrease", 0.95, 0.1, 1.0));

	// �����̈��ǐ�
	contourFinder.setTargetColor(ofColor(255));
	contourFinder.setThreshold(128);
	
	startDebugMode();

	return true;
}

//--------------------------------------------------------------
void CImageProcessing::update(cv::Mat* newFrame)
{
	depthMat16 = newFrame;

	if (bDebugMode)
	{
		newFrame->convertTo(*depthMat, CV_8U, -255.0f / 4500.0f, 255.0f);
		depthImage.setFromPixels(depthMat->data, depthWidth, depthHeight);
	}
}

//--------------------------------------------------------------
void CImageProcessing::draw_test()
{
	grayscaleImage.draw(0, 0);
}

//--------------------------------------------------------------
void CImageProcessing::calcBackground()
{
	// ���݂̉�f�l���w�i���߂�(�l��������)�ꍇ�͒l���X�V
	// threshold���߂����͖̂���
	unsigned long image_size = depthHeight * depthWidth;
	for (int i = 0; i < image_size; i++)
	{
		int index = i * 2;

		unsigned short low_bits  = depthMat16->data[index + 0];
		unsigned short high_bits = depthMat16->data[index + 1];

		unsigned short distance = low_bits + (high_bits << 8);
		unsigned short bgDistance = bgMat16->data[index] + (bgMat16->data[index + 1] << 8);

		if (distance > cut_off_threshold &&
			distance < bgDistance)
		{
			bgMat16->data[index + 0] = low_bits;			// ����bit
			bgMat16->data[index + 1] = high_bits;	// ���bit
		}
	}
	bgMat16->convertTo(*depthMat, CV_8U, -255.0f / 4500.0f, 255.0f);
	bgImage.setFromPixels(depthMat->data, depthWidth, depthHeight);
}

//--------------------------------------------------------------
void CImageProcessing::resetBackground()
{
	for (int j = 0; j < depthHeight; j++)
	{
		for (int i = 0; i < depthWidth; i++)
		{
			int index = (i + j*depthWidth) * 2;

			bgMat16->data[index] = 255;
			bgMat16->data[index+1] = 255;
		}
	}
}

//--------------------------------------------------------------
void CImageProcessing::findActiveObject()
{
	// �O�i�̈�����o�� & ��l��
	for (int j = 0; j < depthHeight; j++)
	{
		for (int i = 0; i < depthWidth; i++)
		{
			int index = i + j*depthWidth;
			int index2 = index * 2;

			unsigned short fsDistance = depthMat16->data[index2] + (depthMat16->data[index2 + 1] << 8);
			unsigned short bgDistance = bgMat16->data[index2] + (bgMat16->data[index2 + 1] << 8);

			if (fsDistance < bgDistance - bg_diff &&			// �w�i����O�ɂ���
				fsDistance > bgDistance - bg_diff - sensing_range &&	// �F���ʂ���O�ɂ���
				fsDistance > cut_off_threshold &&							// �s���Ȓl
				maskMat->data[index] > 128)										// �}�X�N�͈̔͊O
			{
				frontMat->data[index] = 255;
				//if (fsDistance > bgDistance - bg_diff - touch_range)
				//{
				//	touchMat->data[index] = 255;
				//}
				//else
				//{
				//	touchMat->data[index] = 0;
				//}
			}
			else
			{
				// ��
				frontMat->data[index] = 0;
				//touchMat->data[index] = 0;
			}

		}
	}

	// �m�C�Y����(���k���[�����c��)
	//erode(*frontMat, *tmpMat, erosion_element);
	//dilate(*tmpMat, *destMat, erosion_element);

	// �m�C�Y����(���k��)
	erode(*frontMat, *destMat, erosion_element);

	// �m�C�Y����(�^�b�`�̈�)
	//erode(*touchMat, *tmpMat, erosion_element);
	//dilate(*tmpMat, *touchMat, erosion_element);

	// �̈敪��
	//findObject(destMat, activeObjects, MAX_ACTIVE_BLOBS);

}

//--------------------------------------------------------------
void CImageProcessing::findStaticObject()
{
////	��������A�q�[�g�}�b�v�����
//
//	// TODO: �d������shader�ł��
//
//	for (int j = 0; j < depthHeight; j++)
//	{
//		for (int i = 0; i < depthWidth; i++)
//		{
//			int index = i + j*depthWidth;
//			int index2 = index * 2;
//
//			unsigned short old_value = staticMat16->data[index2] + (staticMat16->data[index2 + 1] << 8);
//			unsigned short new_value;
//
//			if (frontMat->data[index] > 0)
//			{
//				new_value = min(old_value + heatmapIncrease, HEAT_MAP_RESOLUTION);
//			}
//			else
//			{
//				new_value =  max(old_value * heatmapDecrease, 1.0f);
//			}
//
//			// ���o�p�ɓ�l��
//			if (new_value > 5000)
//			{
//				staticMat->data[index] = 255;
//			}
//			else
//			{
//				staticMat->data[index] = 0;
//			}
//
//			// �l�X�V
//			staticMat16->data[index2] = new_value & 0xff;	// ����bit
//			staticMat16->data[index2 + 1] = new_value >> 8; // ���bit
//
//		}
//	}
//
//	// �̈敪��
//	findObject(staticMat, staticObjects, MAX_STATIC_BLOBS);
}

//--------------------------------------------------------------
void CImageProcessing::findTouchObject()
{
	// �m�C�Y����(���k��)
	erode(*touchMat, *destMat, erosion_element);

	// �̈敪��
	findObject(destMat, touchObjects, MAX_ACTIVE_BLOBS);
}

//--------------------------------------------------------------
void CImageProcessing::calcFrontArea()
{
	// �O�i�̈�����o�� & ��l��
	for (int j = 0; j < depthHeight; j++)
	{
		for (int i = 0; i < depthWidth; i++)
		{
			int index = i + j*depthWidth;
			int index2 = index * 2;

			unsigned short fsDistance = depthMat16->data[index2] + (depthMat16->data[index2 + 1] << 8);
			unsigned short bgDistance = bgMat16->data[index2] + (bgMat16->data[index2 + 1] << 8);

			if ( abs(fsDistance - bgDistance) < bg_diff )
			{
				frontMat->data[index] = 0;
			}
			else
			{
				frontMat->data[index] = 255;
			}

			//if (fsDistance < bgDistance - bg_diff &&			// �w�i����O�ɂ���
			//	fsDistance > bgDistance - bg_diff - sensing_range &&	// �F���ʂ���O�ɂ���
			//	fsDistance > cut_off_threshold &&							// �s���Ȓl
			//	maskMat->data[index] > 128)										// �}�X�N�͈̔͊O
			//{
			//	frontMat->data[index] = 255;
			//}
			//else
			//{
			//	frontMat->data[index] = 0;
			//}

		}
	}

	// �m�C�Y����(���k��)
	erode(*frontMat, *destMat, erosion_element);
}

bool CImageProcessing::isTouchArea(int depth_x, int depth_y)
{
	// TODO: �������l���߂��Ă��邩�`�F�b�N
	if (depth_x < 0 || depth_x > depthWidth - 1) depth_x = 0;
	if (depth_y < 0 || depth_y > depthHeight - 1) depth_y = 0;

	int value = touchMat->data[depth_y * depthWidth + depth_x];

	//if (depth_x > 0)
	//{
	//	cout << "value = " << value << endl;
	//}

	return value > 0;
}

//--------------------------------------------------------------
void CImageProcessing::setMaskImage(uchar* pixels)
{
	unsigned long image_size = depthHeight * depthWidth;
	for (int i = 0; i < image_size; i++)
	{
		maskMat->data[i] = pixels[i*4];
	}
}

//--------------------------------------------------------------
void CImageProcessing::show_bg_image(int _x, int _y)
{
	bgImage.draw(_x, _y);
	
}

//--------------------------------------------------------------
void CImageProcessing::show_mask_image()
{
	grayscaleImage.setFromPixels(maskMat->data, depthWidth, depthHeight);
	grayscaleImage.draw(0, 0);
}

//--------------------------------------------------------------
void CImageProcessing::show_front_image(int _x, int _y)
{
	//dstMat16->convertTo(*depthMat, CV_8U, 255.0f / 4500.0f, .0f);
	//grayscaleImage.setFromPixels(depthMat->data, depthWidth, depthHeight);
	grayscaleImage.setFromPixels(frontMat->data, depthWidth, depthHeight);

	grayscaleImage.draw(_x, _y);
}

//--------------------------------------------------------------
void CImageProcessing::show_denoised_front_image()
{
	//dstMat16->convertTo(*depthMat, CV_8U, 255.0f / 4500.0f, .0f);
	//grayscaleImage.setFromPixels(depthMat->data, depthWidth, depthHeight);
	grayscaleImage.setFromPixels(destMat->data, depthWidth, depthHeight);

	grayscaleImage.draw(0, 0);
}

//--------------------------------------------------------------
void CImageProcessing::show_contours_image()
{
	contourFinder.draw();

	ofPushStyle();
	ofNoFill();
	ofSetColor(255, 0, 0);
	for (int i = 0; i < MAX_ACTIVE_BLOBS; i++)
	{
		if (activeObjects[i].z > 0)
		{
			ofDrawCircle(activeObjects[i].x, activeObjects[i].y, activeObjects[i].z);
		}
	}
	ofPopStyle();
}

//--------------------------------------------------------------
void CImageProcessing::show_debug_image(int _x, int _y)
{
	depthImage.draw(_x, _y);

	ofPushStyle();
	ofNoFill();
	ofSetLineWidth(2);
	// �����Ă���I�u�W�F�N�g
	ofSetColor(255, 0, 0);
	for (int i = 0; i < MAX_ACTIVE_BLOBS; i++)
	{
		if (activeObjects[i].z > 0)
		{
			ofDrawCircle(activeObjects[i].x + _x, activeObjects[i].y + _y, activeObjects[i].z);
		}
	}
	// �~�܂��Ă���I�u�W�F�N�g
	ofSetColor(0, 255, 0);
	for (int i = 0; i < MAX_STATIC_BLOBS; i++)
	{
		if (staticObjects[i].z > 0)
		{
			ofDrawCircle(staticObjects[i].x + _x, staticObjects[i].y + _y, staticObjects[i].z);
		}
	}
	ofPopStyle();
}

//--------------------------------------------------------------
void CImageProcessing::show_static_map_image(int _x, int _y)
{
	staticMat16->convertTo(*destMat, CV_8U, 255.0f / HEAT_MAP_RESOLUTION, 0.0f);
	grayscaleImage.setFromPixels(destMat->data, depthWidth, depthHeight);

	grayscaleImage.draw(_x, _y);
}

//--------------------------------------------------------------
void CImageProcessing::findObject(Mat *inputMat, vector<ofVec3f> &outputObjects, int object_size)
{
	// �̈敪��
	if (bDebugMode)
	{
		//contourFinder.setMinAreaRadius(minAreaRadius);
		//contourFinder.setMaxAreaRadius(maxAreaRadius);
	}
	contourFinder.findContours(*inputMat);

	// activeObjects�����Z�b�g
	for (int i = 0; i < object_size; i++)
	{
		outputObjects[i] = ofVec3f(0, 0, 0);
	}

	// ���ʂ�activeObjects�ɓ����
	int blob_size = min(int(contourFinder.size()), object_size);
	float radius;
	ofVec2f center;
	for (int i = 0; i < blob_size; i++)
	{
		center = ofxCv::toOf(contourFinder.getMinEnclosingCircle(i, radius));
		outputObjects[i] = ofVec3f(center.x, center.y, radius);
	}
}