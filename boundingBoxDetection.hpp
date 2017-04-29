//
//  boundingBoxDetection.hpp
//  prahvi
//
//  Created by Yang Li on 4/29/17.
//  Copyright © 2017 Portable Reading Assistant Headset for the Visually Impaired. All rights reserved.
//
//	Description: header file for the boundingBoxDetection module

#ifndef boundingBoxDetection_hpp
#define boundingBoxDetection_hpp

#include <opencv2/opencv.hpp>

std::vector<cv::Rect> getBoundingBoxes(cv::Mat &image);

#endif /* boundingBoxDetection_hpp */
