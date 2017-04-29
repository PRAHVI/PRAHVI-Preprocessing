//
//  imageToText.cpp
//  prahvi
//
//  Created by Yang Li on 4/29/17.
//  Copyright © 2017 Portable Reading Assistant Headset for the Visually Impaired. All rights reserved.
//
//	Description: module that converts the image received to a string of text
//		the image received is alread preprocessed
//		currently just passes the image to the google tesseract api

#include <tesseract/baseapi.h>
#include "imageToText.hpp"


//	Function: imageToText
//	Description: receive a Mat and pass the Mat to OCR to detect the text
//		The border of the image (Mat) is removed to reduce noise
//		The OCR is initialized for English ONLY.

std::string imageToText(cv::Mat &image)
{
	std::string outText;
	
	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
	
	// Initialize tesseract-ocr with English, without specifying tessdata path
	if (api->Init(NULL, "eng"))
	{
		std::cerr << "ERROR: could not initialize tesseract" << std::endl;
		exit(1);
	}
	
	// crop the image to remove the border
	// this reduces the noise from the background
	// can use fixed pixels or with respect to width and height
	
	int offsetX = 10;//image.size().width*0.01;
	int offsetY = 10;//image.size().height*0.01;
	
	cv::Rect roi;
	roi.x = offsetX;
	roi.y = offsetY;
	roi.width = image.size().width - (offsetX*2);
	roi.height = image.size().height - (offsetY*2);
	
	// crop the original image to the defined ROI
	
	image = image(roi);
	
	// send the image to OCR
	api->SetImage((uchar*)image.data,
				  image.size().width,
				  image.size().height,
				  image.channels(),
				  image.step1());
	
	// get OCR result
	api->Recognize(0);
	outText = api->GetUTF8Text();
	
	// destroy used object and release memory
	api->End();
	
	return outText;
}
