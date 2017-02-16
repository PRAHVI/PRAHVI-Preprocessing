//
//  blurDetection.cpp
//  camera_blur
//
//  Created by Yang Li on 2/12/17.
//  Copyright © 2017 Yang Li. All rights reserved.
//

#include "opencv2/opencv.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include "blurDetection.h"
#include "modes.h"

extern int mode;

#define BLUR_THRESHOLD 50

using namespace cv;
using namespace std;

double variance_of_laplacian(Mat *gray)
{
	Mat laplacian_result;
	Scalar mean;
	Scalar stddev;
	
	Laplacian(*gray, laplacian_result, CV_64F);
	meanStdDev(laplacian_result, mean, stddev);
	return pow((double) stddev[0],2);
}

bool blur_detection(Mat *image)
{
	Mat gray;
	cvtColor(*image, gray, COLOR_BGR2GRAY);
	string result = "Not Blur";
	double variance;
	bool output = true;
	
	variance = variance_of_laplacian(&gray);
	
	if(variance < BLUR_THRESHOLD)
	{
		result = "Blur";
		output = false;
	}
	
	if(mode != RUNNING)
		putText(*image, result+ ": " + to_string(variance), Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0,0,255));
		
	return output;
}
