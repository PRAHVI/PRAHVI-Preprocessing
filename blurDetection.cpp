//
//  blurDetection.cpp
//  prahvi
//
//  Created by Yang Li on 4/29/17.
//  Copyright © 2017 Portable Reading Assistant Headset for the Visually Impaired. All rights reserved.
//
//	Description: module to check whether the image (Mat object) received is blur

#include <opencv2/imgproc/imgproc.hpp>
#include "blurDetection.hpp"

//	threshold value to determine if the image is blur
#define BLUR_THRESHOLD 50

//	Function: varianceOfLaplacian
//	Description: generate the variance of Laplacian for the matrix received
double varianceOfLaplacian(cv::Mat &imageGray)
{
	cv::Mat laplacian_result;
	cv::Scalar mean;
	cv::Scalar stddev;
	
	Laplacian(imageGray, laplacian_result, CV_64F);
	meanStdDev(laplacian_result, mean, stddev);
	
	return pow((double) stddev[0],2);
}

//	Function: isBlur
//	Description: determine whether image received is blur or not
//		If the variance of Laplacian of the grayscalled image is less than the threshold
//		Then the image is blurred
bool isBlur(cv::Mat &image)
{
	cv::Mat imageGray;
	double variance;
	
	cvtColor(image, imageGray, cv::COLOR_BGR2GRAY);
	variance = varianceOfLaplacian(imageGray);
	
	if(variance < BLUR_THRESHOLD)
	{
		return true;
	}
	return false;
}
