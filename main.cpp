//#include <vld.h> // Visual Leak Detector

#include <iostream>
#include <cmath>

#include "Study.h"

// NOTE: the 9.7" 2048x1536 retina display area measures 196x157mm

// Window dimensions
const GLuint WIDTHMM = 196, HEIGHTMM = 157;


// The MAIN function, from here we start the application and run the game loop
int main()
{
    Study* study = Study::getInstance();
	study->init();
   
    return 0;
}

