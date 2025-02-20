﻿// KinectApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <Kinect.h>
#include <Kinect.VisualGestureBuilder.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease){
	if (pInterfaceToRelease != NULL){
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}

int _tmain(int argc, _TCHAR* argv[]){
	cv::setUseOptimized(true);

	// Sensor
	IKinectSensor* pSensor;
	HRESULT hResult = S_OK;
	hResult = GetDefaultKinectSensor(&pSensor);
	if (FAILED(hResult)){
		std::cerr << "Error : GetDefaultKinectSensor" << std::endl;
		return -1;
	}

	hResult = pSensor->Open();
	if (FAILED(hResult)){
		std::cerr << "Error : IKinectSensor::Open()" << std::endl;
		return -1;
	}

	// Source
	IColorFrameSource* pColorSource;
	hResult = pSensor->get_ColorFrameSource(&pColorSource);
	if (FAILED(hResult)){
		std::cerr << "Error : IKinectSensor::get_ColorFrameSource()" << std::endl;
		return -1;
	}

	// Reader
	IColorFrameReader* pColorReader;
	hResult = pColorSource->OpenReader(&pColorReader);
	if (FAILED(hResult)){
		std::cerr << "Error : IColorFrameSource::OpenReader()" << std::endl;
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
	double scaleX = 0.4;
	double scaleY = 0.4;

	cv::Mat bufferMat(height, width, CV_8UC4);
	cv::Mat colorMat(height * scaleY, width * scaleX, CV_8UC4);
	cv::namedWindow("Color");

	cv::VideoCapture capture(CV_CAP_OPENNI);

	while (1) {
		// Frame
		IColorFrame* pColorFrame = nullptr;
		hResult = pColorReader->AcquireLatestFrame(&pColorFrame);
		if (SUCCEEDED(hResult)){
			hResult = pColorFrame->CopyConvertedFrameDataToArray(bufferSize, reinterpret_cast<BYTE*>(bufferMat.data), ColorImageFormat::ColorImageFormat_Bgra);
			if (SUCCEEDED(hResult)){
				cv::resize(bufferMat, colorMat, cv::Size(), scaleX, scaleY);
			}

			cvtColor(colorMat, colorMat, CV_BGR2GRAY);
			GaussianBlur(colorMat, colorMat, cv::Size(7, 7), 1.5, 1.5);
			Canny(colorMat, colorMat, 0, 30, 3);
		}
		SafeRelease(pColorFrame);

		cv::imshow("Color", colorMat);
		if (cv::waitKey(30) == VK_ESCAPE){
			break;
		}
	}

	SafeRelease(pColorSource);
	SafeRelease(pColorReader);
	SafeRelease(pDescription);
	if (pSensor){
		pSensor->Close();
	}
	SafeRelease(pSensor);
	cv::destroyAllWindows();

	return 0;
}
