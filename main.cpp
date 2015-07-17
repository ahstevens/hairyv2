//#include <vld.h> // Visual Leak Detector

#include <iostream>
#include <cmath>

#include "Study.h"


// Function prototypes
//void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
//void mouse_callback(GLFWwindow* window, double xpos, double ypos);
//void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
//void do_movement();

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

