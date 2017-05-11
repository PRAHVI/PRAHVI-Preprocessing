//
//  scanner.cpp
//  prahvi
//
//  Created by Yang Li on 4/29/17.
//  Copyright © 2017 Portable Reading Assistant Headset for the Visually Impaired. All rights reserved.
//
//	Description: module that extract the text area from the image
//		such that the result will be like a scanned document

#include <algorithm>
#include <vector>
#include "scanner.hpp"

//	Function: comaprePointSum
//	Description: compare 2 points based on the sum of the coordinate
//		return true if the first point is smaller than the second point
bool comparePointSum(cv::Point a, cv::Point b)
{
	return a.x + a.y < b.x + b.y;
}
//	Function: comaprePointDifference
//	Description: compare 2 points based on the difference of the coordinate
//		return true if the first point is smaller than the second point
bool comparePointDifference(cv::Point a, cv::Point b)
{
	return a.y - a.x < b.y - b.x;
}

//	Function: compareArea
//	Description: compare 2 points based on the contor area
//		return true if the first point is larger than the second point
bool compareArea(std::vector<cv::Point> a, std::vector<cv::Point> b)
{
	return contourArea(a) > contourArea(b);
}

//	Function: getDistance
//	Description: return the distance between two points
int getDistance(cv::Point a, cv::Point b)
{
	return sqrt(pow((double)b.x - (double)a.x, 2) + pow((double)b.y - (double)a.y, 2));
}

//	Function: sortContours
//	Description: sort the contours based on the contour area
//		in descending order
void sortContours(std::vector<std::vector<cv::Point>> &contours)
{
	sort(contours.begin(), contours.end(), compareArea);
}

//	Function: getTextArea
//	Description: extract the text area from the image
//		Based on find the largest contour with 4 sides in the image
//		this function also transform the result found and rectify it
cv::Mat getTextArea(cv::Mat &image)
{
	
	// convert to grayscale and blur
	cv::Mat imageGray;
	cvtColor(image, imageGray, CV_BGR2GRAY);
	
	cv::Mat blurred;
	GaussianBlur(imageGray, blurred, cv::Size(5, 5), 0);
	
	// apply Canny Edge Detection to find the edges
	cv::Mat edged;
	Canny(blurred, edged, 0, 50);
	
	//	find the contours in the edged image
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	
	findContours(edged, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
	
	//	sort the contours in descending order
	sortContours(contours);
	
	//	initialize the screen contour
	std::vector<cv::Point> screenContour;
	std::vector<cv::Point> approx;
	
	//	set screen contour to the largest contour with 4 sides
	for(int i = 0; i < contours.size(); i++)
	{
		double peri = arcLength(contours[i], true);
		
		approxPolyDP(cv::Mat(contours[i]), approx, 0.02*peri,true);
		
		if(approx.size() == 4)
		{
			screenContour = approx;
			break;
		}
	}
	
	std::vector<std::vector<cv::Point>> screen;
	screen.push_back(screenContour);
	
	//	initialize transformation
	cv::Mat lambda(2, 4, CV_32FC1);
	lambda = cv::Mat::zeros(image.rows, image.cols, image.type());
	
	//	input and output coordinates
	cv::Point2f inputQuad[4];
	cv::Point2f outputQuad[4];
	
	//	find the max dimension of the crop
	cv::Point topLeft, topRight, bottomRight, bottomLeft;
	
	//	the top left point has the smallest sum
	topLeft = *min_element(screenContour.begin(), screenContour.end(), comparePointSum);
	
	//	the bottom right point has the largest sum
	bottomRight = *max_element(screenContour.begin(), screenContour.end(), comparePointSum);
	
	//	the top right point has the smallest difference
	topRight = *min_element(screenContour.begin(), screenContour.end(), comparePointDifference);
	
	//	the bottom left point has the largest difference
	bottomLeft = *max_element(screenContour.begin(), screenContour.end(), comparePointDifference);

	//	set input coordinates
	inputQuad[0] = topLeft;
	inputQuad[1] = topRight;
	inputQuad[2] = bottomRight;
	inputQuad[3] = bottomLeft;
	
	//	the dimension of the output is based on the input
	//	1:1 ratio
	int width = std::max(getDistance(topLeft, topRight), getDistance(bottomLeft, bottomRight));
	int height = std::max(getDistance(topLeft, bottomLeft), getDistance(topRight, bottomRight));
	
	//	the output coordinates is based on the output dimention
	outputQuad[0] = cv::Point2f(0,0);
	outputQuad[1] = cv::Point2f(width-1, 0);
	outputQuad[2] = cv::Point2f(width-1, height-1);
	outputQuad[3] = cv::Point2f(0, height-1);
	
	//	set up transformation
	lambda = getPerspectiveTransform(inputQuad, outputQuad);
	
	cv::Mat output;
	
	//	apply transformation
	warpPerspective(image, output, lambda, cv::Size(width,height));
	
	return output;
}
