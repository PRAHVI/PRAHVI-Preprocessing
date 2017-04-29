//
//  detectSkewAngle.cpp
//  prahvi
//
//  Created by Yang Li on 4/6/17.
//  Copyright © 2017 Yang Li. All rights reserved.
//

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


using namespace std;

void compute_skew(const char* filename)
{
	// Load in grayscale.
	cv::Mat original = cv::imread(filename);
	cv::Mat src;
	
	
	cvtColor(original, src, cv::COLOR_BGR2GRAY);
	
	cv::Size size = src.size();

	cv::bitwise_not(src, src);

	std::vector<cv::Vec4i> lines;

	cv::HoughLinesP(src, lines, 1, CV_PI/180, 100, size.width / 2.f, 20);

	cv::Mat disp_lines(size, CV_8UC1, cv::Scalar(0, 0, 0));
	double angle = 0.;
	unsigned nb_lines = lines.size();
	for (unsigned i = 0; i < nb_lines; ++i)
	{
		cv::line(disp_lines, cv::Point(lines[i][0], lines[i][1]),
				 cv::Point(lines[i][2], lines[i][3]), cv::Scalar(255, 0 ,0));
		angle += atan2((double)lines[i][3] - lines[i][1],
					   (double)lines[i][2] - lines[i][0]);
	}
	angle /= nb_lines; // mean angle, in radians.
 
	std::cout << "File " << filename << ": " << angle * 180 / CV_PI << std::endl;
 
	cv::imshow(filename, disp_lines);
	cv::waitKey(0);
	cv::destroyWindow(filename);
}

const char* files[] = { "/Users/Youngestyle/Downloads/ImagefromWood/IMG_1040.jpg"};

int mainTEST()
{
	unsigned nb_files = sizeof(files) / sizeof(const char*);
	for (unsigned i = 0; i < nb_files; ++i)
		compute_skew(files[i]);
	return 0;
}
