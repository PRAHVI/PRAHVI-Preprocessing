//
//  boundingBoxDetection.cpp
//  prahvi
//
//  Created by Yang Li on 4/29/17.
//  Copyright © 2017 Portable Reading Assistant Headset for the Visually Impaired. All rights reserved.
//
//	Description: module for boundingBoxDetection
//		generates the bounding boxes for texts

#include "boundingBoxDetection.hpp"

//	Function: detectLetters
//	Description: identify the bounding box for texts
//		Based on the idea of license plate detection
std::vector<cv::Rect> detectLetters(cv::Mat &image)
{
	std::vector<cv::Rect> boundingBoxes;
	cv::Mat imageGray, imageSobel, imageThreshold, element;
	
	//	convert image to gray scale
	cvtColor(image, imageGray, CV_BGR2GRAY);
	
	//	apply sobel filter
	cv::Sobel(imageGray, imageSobel, CV_8U, 1, 0, 3, 1, 0, cv::BORDER_DEFAULT);
	
	//	threshold the sobel result
	cv::threshold(imageSobel, imageThreshold, 0, 255, CV_THRESH_OTSU+CV_THRESH_BINARY);
	
	//	get element for morphologyEx operation
	element = getStructuringElement(cv::MORPH_RECT, cv::Size(17, 3) );
	
	//	apply morphological operation
	cv::morphologyEx(imageThreshold, imageThreshold, CV_MOP_CLOSE, element);
	
	//	get contours
	std::vector<std::vector< cv::Point>> contours;
	
	cv::findContours(imageThreshold, contours, 0, 1);
	
	std::vector<std::vector<cv::Point>> contoursPoly(contours.size());
	
	//	get bounding boxes
	for(int i = 0; i < contours.size(); i++)
	{
		if (contours[i].size()>100)
		{
			cv::approxPolyDP(cv::Mat(contours[i]), contoursPoly[i], 3, true);
			cv::Rect approxRectangle(boundingRect(cv::Mat(contoursPoly[i])));
			
			if (approxRectangle.width>approxRectangle.height)
			{
				boundingBoxes.push_back(approxRectangle);
			}
		}
	}
	
	return boundingBoxes;
}

std::vector<cv::Rect> getBoundingBoxes(cv::Mat &image)
{
	//Detect
	std::vector<cv::Rect> boundingBoxes;
	boundingBoxes=detectLetters(image);
	
	//Display
	for(int i=0; i< boundingBoxes.size(); i++)
	{
		cv::rectangle(image,boundingBoxes[i],cv::Scalar(0,255,0),3,8,0);
	}
	//cv::imwrite( name +"extract.png", img1);

	return boundingBoxes;
}
