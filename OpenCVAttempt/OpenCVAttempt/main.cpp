// KinectApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <Kinect.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease){
	if (pInterfaceToRelease != NULL){
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}


int main() {
	//Sensor
	IKinectSensor* pSensor;
	HRESULT hResult = S_OK;

	hResult = GetDefaultKinectSensor(&pSensor);
	if (FAILED(hResult)){
		std::cout << "ERROR: GetDefaultKinectSensor()" << std::endl;
		return -1;
	}

	hResult = pSensor->Open();
	if (FAILED(hResult)){
		std::cout << "ERROR: IKinectSensor::Open()" << std::endl;
		return -1;
	}

	//Source
	IColorFrameSource* pColorSource;
	hResult = pSensor->get_ColorFrameSource(&pColorSource);
	if (FAILED(hResult)){
		std::cerr << "ERROR: IKinectSensor::get_ColorFrameSource()" << std::endl;
		return -1;
	}

	//Reader
	IColorFrameReader* pColorReader;
	hResult = pColorSource->OpenReader(&pColorReader);
	if (FAILED(hResult)){
		std::cerr << "ERROR: IColorFrameSource::OpenReader()" << std::endl;
		return -1;
	}

	// Description
	IFrameDescription* pDescription;
	hResult = pColorSource->get_FrameDescription(&pDescription);
	if (FAILED(hResult)){
		std::cerr << "Error : IColorFrameSource::get_FrameDescription()" << std::endl;
		return -1;
	}

	int width = 0;
	int height = 0;
	pDescription->get_Width(&width); // 1920
	pDescription->get_Height(&height); // 1080
	unsigned int bufferSize = width * height * 4 * sizeof(unsigned char);

	cv::Mat bufferMat(height, width, CV_8UC4);
	cv::Mat colorMat(height / 2, width / 2, CV_8UC4);
	cv::namedWindow("Color");

	while (1){
		//Frame
		IColorFrame* pColorFrame = nullptr;
		hResult = pColorReader->AcquireLatestFrame(&pColorFrame);
		if (SUCCEEDED(hResult)){
			//Data
			unsigned int bufferSize = 0;
			unsigned char* pBuffer = nullptr;
			hResult = pColorFrame->AccessRawUnderlyingBuffer(&bufferSize, &pBuffer);
			if (SUCCEEDED(hResult)){
				hResult = pColorFrame->CopyConvertedFrameDataToArray(bufferSize, reinterpret_cast<BYTE*>(bufferMat.data), ColorImageFormat::ColorImageFormat_Bgra);
				if (SUCCEEDED(hResult)){
					cv::resize(bufferMat, colorMat, cv::Size(), 0.5, 0.5);
				}
			}

			SafeRelease(pColorFrame);

			cv::imshow("Color", colorMat);

			if (cv::waitKey(30) == VK_ESCAPE){
				break;
			}
		}

	}

	SafeRelease(pColorSource);
	SafeRelease(pColorReader);
	SafeRelease(pDescription);
	if (pSensor){
		pSensor->Close();
	}
	SafeRelease(pSensor);

	return 0;
}

