//
//  main.cpp
//  prahvi
//
//  Created by Yang Li on 4/29/17.
//  Copyright © 2017 Portable Reading Assistant Headset for the Visually Impaired. All rights reserved.
//

#include <iostream>
#include "prahvi.hpp"

int main(int argc, const char * argv[]) {
	// insert code here...
	//std::cout << "Hello, World!\n";
	
	int result;
	std::string text;
	
	prahvi myPrahvi;
	text = myPrahvi.getNewText(result);
	if(result == SUCCESS)
	{
		std::cout << text << std::endl;
	}
	
	return 0;
}
