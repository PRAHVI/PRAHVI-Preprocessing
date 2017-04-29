//
//  featureMatching.cpp
//  prahvi
//
//  Created by Yang Li on 2/12/17.
//  Copyright © 2017 Yang Li. All rights reserved.
//

#include <opencv2/features2d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>

#include "featureMatching.h"
#include "modes.h"

using namespace cv;
using namespace std;

// threshold value to determine whether the two images are similar
#define FEATURE_THRESHOLD 1000


//extern int mode;

const float inlier_threshold = 2.5f; // Distance threshold to identify inliers
const float nn_match_ratio = 0.8f;   // Nearest neighbor matching ratio

bool featureMatching(Mat *img1, Mat *img2)
{
	int mode;

	// convert to grayscale
	Mat img1_gray;
	Mat img2_gray;

	cvtColor(*img1, img1_gray, COLOR_BGR2GRAY);
	cvtColor(*img2, img2_gray, COLOR_BGR2GRAY);
	//Mat img1_gray = imread("graf1.png", IMREAD_GRAYSCALE);
	//Mat img2_gray = imread("graf3.png", IMREAD_GRAYSCALE);
	
	// homograph is stored in the xml created with FileStorage
	Mat homography;
	FileStorage fs("H1to3p.xml", FileStorage::READ);
	fs.getFirstTopLevelNode() >> homography;
	
	// detect keypoints and compute descriptors using AKAZE
	vector<KeyPoint> kpts1, kpts2;
	Mat desc1, desc2;
	
	Ptr<AKAZE> akaze = AKAZE::create();
	akaze->detectAndCompute(img1_gray, noArray(), kpts1, desc1);
	akaze->detectAndCompute(img2_gray, noArray(), kpts2, desc2);
	
	// use brute force matcher to find 2-nn matches
	BFMatcher matcher(NORM_HAMMING);
	vector< vector<DMatch> > nn_matches;
	matcher.knnMatch(desc1, desc2, nn_matches, 2);
	
	if(kpts1.size()>0 && kpts2.size()>0)
	{
		// use 2-nn matches to find correct keypoint matches
		vector<KeyPoint> matched1, matched2, inliers1, inliers2;
		vector<DMatch> good_matches;
		for(size_t i = 0; i < nn_matches.size(); i++) {
			DMatch first = nn_matches[i][0];
			float dist1 = nn_matches[i][0].distance;
			float dist2 = nn_matches[i][1].distance;
			//cout << "i=" << i <<endl;
			//cout << "size=" << nn_matches.size() << endl;
			if(dist1 < nn_match_ratio * dist2) {
				matched1.push_back(kpts1[first.queryIdx]);
				matched2.push_back(kpts2[first.trainIdx]);
			}
		}
		
		// check if the matches fit in the homography model
		for(unsigned i = 0; i < matched1.size(); i++) {
			Mat col = Mat::ones(3, 1, CV_64F);
			col.at<double>(0) = matched1[i].pt.x;
			col.at<double>(1) = matched1[i].pt.y;
			
			//col = homography * col;
			col /= col.at<double>(2);
			double dist = sqrt( pow(col.at<double>(0) - matched2[i].pt.x, 2) + pow(col.at<double>(1) - matched2[i].pt.y, 2));
			
			if(dist < inlier_threshold) {
				int new_i = static_cast<int>(inliers1.size());
				inliers1.push_back(matched1[i]);
				inliers2.push_back(matched2[i]);
				good_matches.push_back(DMatch(new_i, new_i, 0));
			}
		}
		
		if(mode !=RUNNING)
		{
			cout << "good match points: " << good_matches.size() << endl;
			
			Mat res;
			//drawMatches(*img1, inliers1, *img2, inliers2, good_matches, res);
			drawMatches(img1_gray, inliers1, img2_gray, inliers2, good_matches, res);
			//imwrite("res.png", res);
			resize(res, res, Size(), 0.2, 0.2);
			imshow("Camera feed", res);
			
			double inlier_ratio = inliers1.size() * 1.0 / matched1.size();
			cout << "A-KAZE Matching Results" << endl;
			cout << "*******************************" << endl;
			cout << "# Keypoints 1:                        \t" << kpts1.size() << endl;
			cout << "# Keypoints 2:                        \t" << kpts2.size() << endl;
			cout << "# Matches:                            \t" << matched1.size() << endl;
			cout << "# Inliers:                            \t" << inliers1.size() << endl;
			cout << "# Inliers Ratio:                      \t" << inlier_ratio << endl;
			cout << endl;
			
			waitKey(0);
		}
		if(good_matches.size() > FEATURE_THRESHOLD)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

}
