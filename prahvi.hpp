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
#include "tfidf.hpp"

enum ProcessResult {SUCCESS, BLUR, SIMILAR};

class prahvi
{
public:
	prahvi();
	std::string getText();
	std::string getKeyword(int n=1);
	std::string getNewText(int &result);
	
private:
	cv::Mat _previousImage;
	cv::Mat _currentImage;
	std::string _currentText;
	//	TODO - uncomment after add IDF
	//tfidf _tfidf;
};

#endif /* prahvi_hpp */
