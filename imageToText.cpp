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


//	Function: replaceString
//	Description: replace all "toReplace" with "replaceWith" in string "s"
std::string replaceString(std::string &text, const std::string &toReplace, const std::string &replaceWith)
{
	int location = 0;
	int replaceWithLength = replaceWith.length();
	
	while((location = (int) text.find(toReplace, location)) != std::string::npos)
	{
		text.replace(text.find(toReplace), toReplace.length(), replaceWith);
		location += replaceWithLength;
	}
	return text;
}

//	Function: replaceLigatures
//	Description: replace the ligatures with non-ligatures
std::string replaceLigatures(std::string text)
{
	// list of ligatures and non ligatures
	// the list is too long, and it is making the system really slow
	//std::vector<std::string> ligatures = {"Ꜳ", "ꜳ", "Æ", "æ", "Ꜵ",
	//	"ꜵ", "Ꜷ", "ꜷ", "Ꜹ", "ꜹ",
	//	"Ꜻ", "ꜻ", "Ꜽ", "ꜽ", "ﬀ",
	//	"ﬃ", "ﬄ", "ﬁ", "ﬂ", "Œ",
	//	"œ", "Ꝏ", "ꝏ", "ẞ", "ß",
	//	"ﬆ", "ﬅ", "Ꜩ", "ꜩ", "ᵫ",
	//	"Ꝡ", "ꝡ"};
	//std::vector<std::string> nonLigatures = {"AA", "aa", "AE", "ae", "AO",
	//	"ao", "AU", "au", "AV", "av",
	//	"AV", "av", "AY", "ay", "ff",
	//	"ffi", "ffl", "fi", "fl", "OE",
	//	"oe", "OO", "oo", "fs", "fz",
	//	"st", "ft", "TZ", "tz", "ue",
	//	"VY", "vy"};
	
	// thus a shorter list of common ligatures are searched and replaced
	std::vector<std::string> ligatures = {"ﬀ", "ﬃ", "ﬄ", "ﬁ", "ﬂ","ﬆ", "ﬅ"};
	std::vector<std::string> nonLigatures = {"ff", "ffi", "ffl", "fi", "fl", "st", "ft"};
		
	for(int i = 0; i < ligatures.size(); i++)
	{
		text = replaceString(text, ligatures[i], nonLigatures[i]);
	}
	
	return text;
}

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
	
	outText = replaceLigatures(outText);
	
	return outText;
}
