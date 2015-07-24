//#include <vld.h> // Visual Leak Detector

#include <iostream>
#include <cmath>

#include "Study.h"

// The MAIN function, from here we start the application and run the game loop
int main()
{
    Study* study = Study::getInstance();
	study->init();
   
    return 0;
}

