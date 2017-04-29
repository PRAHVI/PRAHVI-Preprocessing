//
//  imageToText.cpp
//  prahvi
//
//  Created by Yang Li on 4/24/17.
//  Copyright © 2017 Yang Li. All rights reserved.
//

#include <stdio.h>
#include <tesseract/baseapi.h>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;


/*
 * Function: getText
 * Description: receive a Mat and pass the Mat to OCR to detect the text
 * The border of the image (Mat) is removed to reduce noise
 * The OCR is initialized for English ONLY.
 */
int main()//Mat &image)
{
	string outText;
	
	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
	
	
	
	// Initialize tesseract-ocr with English, without specifying tessdata path
	if (api->Init(NULL, "eng"))
	{
		fprintf(stderr, "Could not initialize tesseract.\n");
		exit(1);
	}
	if(!api->SetVariable("tessedit_char_blacklist", "123456"))
	{
		cout << "ERROR" << endl;
	}
	//api->SimpleInit([dataPath cStringUsingEncoding:NSUTF8StringEncoding],
	//				 "eng", false);
	
	Mat image=imread("/Users/Youngestyle/Desktop/Senier Design - Flow Test/IMG_9121.JPG.png");
	
	// crop the image to remove the border
	// this reduces the noise from the background
	
	int offset_x = 10;//image.size().width*0.01;
	int offset_y = 10;//image.size().height*0.01;
	
	Rect roi;
	roi.x = offset_x;
	roi.y = offset_y;
	roi.width = image.size().width - (offset_x*2);
	roi.height = image.size().height - (offset_y*2);
	
	/* Crop the original image to the defined ROI */
	
	image = image(roi);
	
	// send the image to OCR
	api->SetImage((uchar*)image.data, image.size().width, image.size().height, image.channels(), image.step1());
	
	// Get OCR result
	api->Recognize(0);
	outText = api->GetUTF8Text();
	
	// output result
	cout << "OCR output:\n" << outText;
	
	// Destroy used object and release memory
	api->End();
	
	//return outText;
}
