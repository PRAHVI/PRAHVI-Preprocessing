//
//  prahvi.hpp
//  prahvi
//
//  Created by Yang Li on 4/29/17.
//  Copyright © 2017 Portable Reading Assistant Headset for the Visually Impaired. All rights reserved.
//
//  Description: header file for prahvi class

#ifndef prahvi_hpp
#define prahvi_hpp

#include <opencv2/opencv.hpp>

enum ProcessResult {SUCCESS, BLUR, SIMILAR, EMPTY};

class prahvi
{
public:
	prahvi();
	std::string getText();
	ProcessResult getNewText(cv::Mat &img);
	
private:
	cv::Mat _previousImage;
	cv::Mat _currentImage;
	std::string _currentText;
};

#endif /* prahvi_hpp */
