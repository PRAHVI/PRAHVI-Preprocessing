//
//  main.cpp
//  camera_blur
//
//  Created by Yang Li on 1/12/17.
//  Copyright © 2017 Yang Li. All rights reserved.
//


#include <iostream>
#include "opencv2/opencv.hpp"
#include "blurDetection.h"
#include "featureMatching.h"
#include "modes.h"

int mode;

using namespace cv;
using namespace std;

int main()
{
	mode = DEVELOPING;
	// Open the first camera attached to your computer
	VideoCapture cap(0);
	
	if(!cap.isOpened())
	{
		cout << "Unable to open the camera\n";
		exit(-1);
	}

	Mat image;
	double FPS = 1.0;
	
	// Read camera frames (at approx 1 FPS) and show the result
	while(true)
	{
		cap >> image;
		if(image.empty())
		{
			cout << "Can't read frames from your camera\n";
			break;
		}

		blur_detection(&image);
		
		imshow("Camera feed", image);

		// Stop the camera if users presses the "ESC" key
		if(waitKey(1000.0/FPS) == 27)
		{
			break;
		}
	}
	return 0;
}
