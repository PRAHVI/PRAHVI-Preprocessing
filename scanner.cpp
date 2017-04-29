//
//  scanner.cpp
//  prahvi
//
//  Created by Yang Li on 4/9/17.
//  Copyright © 2017 Yang Li. All rights reserved.
//

#include <iostream>
#include <algorithm>
#include <vector>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;


// compare 2 points based on the sum of the coordinate
bool comparePointSum(Point a, Point b)
{
	return a.x+a.y < b.x+b.y;
}

// compare 2 points based on the difference of the coordinate
bool comparePointDifference(Point a, Point b)
{
	//return abs(a.x-a.y) < abs(b.x-b.y);
	return a.y-a.x < b.y-b.x;
}

bool compareArea(vector<Point> a, vector<Point> b)
{
	return contourArea(a)>contourArea(b);
}

int getDistance(Point a, Point b)
{
	return sqrt(pow(b.x-a.x,2)+pow(b.y-a.y,2));
}

void sort_contours(vector<vector<Point>> &contours)
{
	sort(contours.begin(), contours.end(), compareArea);
}

int mainTest()
{
	//string name = "/Users/Youngestyle/Downloads/document-scanner-master/test_s1.jpg";
	string name = "/Users/Youngestyle/Desktop/IMG_9121.JPG";
	Mat image=imread(name);
	
	
	// resize for display purpose
	// TODO - need to be removed
	//resize(image, image, Size(1500, 880));
	
	// creating copy of original image
	Mat orig = image;
	
	// convert to grayscale and blur to smooth
	Mat gray;
	cvtColor(image, gray, CV_BGR2GRAY);
	
	Mat blurred;
	GaussianBlur(gray, blurred, Size(5, 5), 0);
	
	// apply Canny Edge Detection
	Mat edged;
	Canny(blurred, edged, 0, 50);
	Mat orig_edged = edged;
	
	//imshow("edged",edged);
	//waitKey(0);
	//destroyAllWindows();
	
	// find the contours in the edged image, keeping only the
	// largest ones, and initialize the screen contour
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(edged, contours, hierarchy, RETR_LIST, CHAIN_APPROX_NONE);
	//contours = sorted(contours, key=cv2.contourArea, reverse=True);
	sort_contours(contours);
	
	//vector<vector<Point> > approx;
	vector<Point> screen_contours;
	vector<Point> approx;
	for(int i = 0; i < contours.size(); i++)
	{
		double peri = arcLength(contours[i], true);
		
		approxPolyDP(Mat(contours[i]), approx, 0.02*peri,true);
		if(approx.size() == 4)
		{
			screen_contours = approx;
			break;
		}
	}
	vector<vector<Point>> screen;
	screen.push_back(screen_contours);
	//drawContours(image, screen, -1, Scalar(128,255,255));
	//drawContours(image, contours, -1, Scalar(255,0,0));
	//imwrite( name+"contours.png", image );
	
	/*
	 for( int idx = 0; idx >= 0; idx = hierarchy[idx][0] )
	{
		Scalar color( rand()&255, rand()&255, rand()&255 );
		drawContours( image, contours, idx, color, CV_FILLED, 8, hierarchy );
	}
	 */
	//imshow("contours", image);
	//imwrite( name+"contours.png", image );
	//waitKey(0);
	//destroyAllWindows();
	
	Mat lambda( 2, 4, CV_32FC1 );
	lambda = Mat::zeros( image.rows, image.cols, image.type() );
	Mat dst;
	Point2f inputQuad[4];
	Point2f outputQuad[4];
	
	// find the max dimension of the crop
	Point tl, tr, br, bl;
	
	tl = *min_element(screen_contours.begin(), screen_contours.end(), comparePointSum);
	br = *max_element(screen_contours.begin(), screen_contours.end(), comparePointSum);
	
	tr = *min_element(screen_contours.begin(), screen_contours.end(), comparePointDifference);
	bl = *max_element(screen_contours.begin(), screen_contours.end(), comparePointDifference);
	
	
	//tl = Point2f(277,176);
	//tr = Point2f(1261,176);
	//br = Point2f(1404,724);
	//bl = Point2f(213,777);
	/*
	circle(image, tl, 50, Scalar(0,255,0),10);
	circle(image, tr, 50, Scalar(255,0,0),10);
	circle(image, br, 50, Scalar(0,255,0),10);
	circle(image, bl, 50, Scalar(0,255,0),10);
	imwrite( name+"corners.png", image );
	//imshow("contours", image);
	*/
	inputQuad[0] = tl;
	inputQuad[1] = tr;
	inputQuad[2] = br;
	inputQuad[3] = bl;
	
	int width = max(getDistance(tl, tr), getDistance(bl, br));
	int height = max(getDistance(tl, bl), getDistance(tr, br));
	
	
	outputQuad[0] = Point2f(0,0);
	outputQuad[1] = Point2f(width-1, 0);
	outputQuad[2] = Point2f(width-1, height-1);
	outputQuad[3] = Point2f(0, height-1);
	
	lambda = getPerspectiveTransform(inputQuad,outputQuad);
	warpPerspective(image, dst, lambda,Size(width,height));
	
	//ret,th1 = cv2.threshold(dst,127,255,cv2.THRESH_BINARY)
	//double th1 = threshold(dst, dst, 127, 255, THRESH_BINARY);
	
	//double th2 = adaptiveThreshold(dst, dst, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY,11,2);
	//th3 = cv2.adaptiveThreshold(dst,255,cv2.ADAPTIVE_THRESH_GAUSSIAN_C,\
								cv2.THRESH_BINARY,11,2)
	//ret2,th4 = cv2.threshold(dst,0,255,cv2.THRESH_BINARY+cv2.THRESH_OTSU)
	imwrite( name+".png", dst );
	//imshow("crop", dst);
	//waitKey(0);
	//destroyAllWindows();
	
	return 0;
	}
