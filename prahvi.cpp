//
//  prahvi.cpp
//  prahvi
//
//  Created by Yang Li on 4/29/17.
//  Copyright © 2017 Portable Reading Assistant Headset for the Visually Impaired. All rights reserved.
//
//  Description: prahvi class
//		the prahvi class does the preprocessing and text detection
//		other program can create and call this class to get corresponding results

#include "prahvi.hpp"
#include "getImage.hpp"
#include "blurDetection.hpp"
#include "similarityDetection.hpp"
#include "imageToText.hpp"
#include "scanner.hpp"
#include "boundingBoxDetection.hpp"

//	Function: prahvi::prahvi
//	Description: constructor for prahvi
prahvi::prahvi()
{
	_previousImage = cv::Mat::zeros(1, 1, CV_64F);
	_currentText = "";
	_currentImage = cv::Mat::zeros(1, 1, CV_64F);
}

//	Function: prahvi::getText
//	Description: get the text of the current image
std::string prahvi::getText()
{
	return _currentText;
}

//	Function: prahvi::getNewText
//	Description: get a new image and process it
//		the fucntion will get a new image
//		if the new image is blur, it will terminate
//		otherwise, it will extract the text area
//		and compare to the previous text area
std::string prahvi::getNewText(int &result)
{
	cv::Mat newImage = getImage();
	
	//	check if the new image is blurred
	if(isBlur(newImage))
	{
		result = BLUR;
		return "";
	}
	
	_previousImage = _currentImage;
	_currentImage = getTextArea(newImage);
	
	//	check if the new image is similar to the previous image
	//	TODO - uncomment after add IDF
	/*
	if(_previousImage == cv::Mat::zeros(1, 1, CV_64F) || isSimilar(_previousImage, _currentImage))
	{
		result = SIMILAR;
		return "";
	}
	 */
	
	//	convert the image to text
	_currentText = imageToText(_currentImage);
	
	//	reset TF-IDF and generate the score for the new document
	//	TODO - uncomment after add IDF
	//_tfidf.resetTerms();
	//_tfidf.addTerms(_currentText);
	
	result = EMPTY;
	
	for(int i = 0; i < _currentText.length(); i++)
	{
		if(!isspace(_currentText[i]))
		{
			result = SUCCESS;
		}
	}
	
	return _currentText;
}

std::string prahvi::getKeyword(int n)
{
	//	TODO - uncomment after add IDF
	return "";//_tfidf.getTerm(n);
}
