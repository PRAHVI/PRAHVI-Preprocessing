//
//  getImage.cpp
//  prahvi
//
//  Created by Yang Li on 4/29/17.
//  Copyright © 2017 Portable Reading Assistant Headset for the Visually Impaired. All rights reserved.
//
//  Description: module for get the image for PRAHVI
//

#include "getImage.hpp"

//  Function: getImage()
//  Description: function that returns an opencv Mat object - an image for PRAHVI to process
//		Initially setup to read from a file, need to change with ios
//		TODO
cv::Mat getImage()
{
	cv::Mat image = cv::imread("/Users/Youngestyle/Desktop/Senier Design - Flow Test/IMG_9121.png");
	return image;
}
