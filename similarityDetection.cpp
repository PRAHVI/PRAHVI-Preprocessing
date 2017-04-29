//
//  similarityDetection.cpp
//  prahvi
//
//  Created by Yang Li on 4/29/17.
//  Copyright © 2017 Portable Reading Assistant Headset for the Visually Impaired. All rights reserved.
//
//	Description: module to detect whether the two image received are similar
//		Currently, the method used is AKAZE tracking
//		Can be improved using matching



#include <opencv2/features2d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <vector>
#include "similarityDetection.hpp"

//	threshold value to determine whether the two images are similar
#define FEATURE_THRESHOLD 1000


const float inlier_threshold = 2.5f; // Distance threshold to identify inliers
const float nn_match_ratio = 0.8f;   // Nearest neighbor matching ratio

//	Function: akazeTracking
//	Description: comapre two images and return the number of good match points
//		Uses the A-Kaze tracking
int akazeTracking(cv::Mat &image1, cv::Mat &image2)
{
	// convert the images to grayscale
	cv::Mat image1Gray;
	cv::Mat image2Gray;
	
	cvtColor(image1, image1Gray, cv::COLOR_BGR2GRAY);
	cvtColor(image2, image2Gray, cv::COLOR_BGR2GRAY);
	
	
	// detect keypoints and compute descriptors using A-KAZE
	std::vector<cv::KeyPoint> keyPoints1, keyPoints2;
	cv::Mat descriptors1, descriptors2;
	
	cv::Ptr<cv::AKAZE> akaze = cv::AKAZE::create();
	akaze->detectAndCompute(image1Gray, cv::noArray(), keyPoints1, descriptors1);
	akaze->detectAndCompute(image2Gray, cv::noArray(), keyPoints2, descriptors2);
	
	// use the brute force matcher to find 2-nn matches
	cv::BFMatcher matcher(cv::NORM_HAMMING);
	std::vector<std::vector<cv::DMatch>> nn_matches;
	matcher.knnMatch(descriptors1, descriptors2, nn_matches, 2);
	
	// if one or more of the image does not have any keypoint, return 0
	if(keyPoints1.size() <= 0 || keyPoints2.size() <= 0)
	{
		return 0;
	}
	
	// use 2-nn matches to find correct keypoint matches
	std::vector<cv::KeyPoint> matched1, matched2, inliers1, inliers2;
	std::vector<cv::DMatch> good_matches;
	
	for(size_t i = 0; i < nn_matches.size(); i++)
	{
		cv::DMatch first = nn_matches[i][0];
		
		float distance1 = nn_matches[i][0].distance;
		float distance2 = nn_matches[i][1].distance;
		
		if(distance1 < nn_match_ratio * distance2)
		{
			matched1.push_back(keyPoints1[first.queryIdx]);
			matched2.push_back(keyPoints2[first.trainIdx]);
		}
	}
	
	// check if the matches is within the inlier_threshold
	for(unsigned i = 0; i < matched1.size(); i++) {
		cv::Mat col = cv::Mat::ones(3, 1, CV_64F);
		col.at<double>(0) = matched1[i].pt.x;
		col.at<double>(1) = matched1[i].pt.y;
		
		col /= col.at<double>(2);
		double distance = sqrt(
						   pow(col.at<double>(0) - matched2[i].pt.x, 2)
						   + pow(col.at<double>(1) - matched2[i].pt.y, 2)
						   );
		
		if(distance < inlier_threshold) {
			int new_i = static_cast<int>(inliers1.size());
			inliers1.push_back(matched1[i]);
			inliers2.push_back(matched2[i]);
			good_matches.push_back(cv::DMatch(new_i, new_i, 0));
		}
	}
	return good_matches.size();
}

bool isSimilar(cv::Mat &image1, cv::Mat &image2)
{
	if(akazeTracking(image1, image2) > FEATURE_THRESHOLD)
	{
		return true;
	}
	return false;
}
