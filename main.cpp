//#include <vld.h> // Visual Leak Detector

#include <iostream>
#include <cmath>

#include "Study.h"

const GLint   WIDTH_RETINA_PX   = 2048;    // iPad 3 Retina screen pixel width
const GLint   HEIGHT_RETINA_PX  = 1536;    // iPad 3 Retina screen pixel height
const GLfloat WIDTH_RETINA_MM   = 197.1f;  // iPad 3 Retina screen physical width (mm)
const GLfloat HEIGHT_RETINA_MM  = 147.82f; // iPad 3 Retina screen physical height (mm)
const GLint   WIDTH_DELL_PX     = 1920;    // Dell U2412M screen pixel width
const GLint   HEIGHT_DELL_PX    = 1200;    // Dell U2412M screen pixel height
const GLfloat WIDTH_DELL_MM     = 518.4f;  // Dell U2412M screen physical width (mm)
const GLfloat HEIGHT_DELL_MM    = 324.0f;  // Dell U2412M screen physical height (mm)
const GLfloat DIST_EYE_MM       = 560.0f;  // Eye distance from display (mm)



// The MAIN function, from here we start the application and run the game loop
int main()
{
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Choose the iPad retina display for full screen, if present
	int count, widthMM, heightMM;
	GLFWmonitor** monitors = glfwGetMonitors(&count);
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	GLboolean retinaDisplayPresent = false;
	for (int i = 0; i < count; ++i)
	{
		glfwGetMonitorPhysicalSize(monitors[i], &widthMM, &heightMM);
		std::cout << "Monitor " << i << ": " << widthMM << "mm x " << heightMM << "mm" << std::endl;
		if (widthMM == 722 && heightMM == 542)
		{
			retinaDisplayPresent = true;
			monitor = monitors[i];
			break;
		}
	}

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window;
	if (retinaDisplayPresent)
		window = glfwCreateWindow(WIDTH_RETINA_PX, HEIGHT_RETINA_PX, "Hairy Slices", monitor, nullptr);
	else
		window = glfwCreateWindow(WIDTH_DELL_PX, HEIGHT_DELL_PX, "Hairy Slices", monitor, nullptr);

	glfwMakeContextCurrent(window);

	// GLFW Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, (GLsizei)retinaDisplayPresent ? WIDTH_RETINA_PX : WIDTH_DELL_PX, (GLsizei)retinaDisplayPresent ? HEIGHT_RETINA_PX : HEIGHT_DELL_PX);

    Study* study = Study::getInstance( window );
	study->init(retinaDisplayPresent ? WIDTH_RETINA_MM : WIDTH_DELL_MM, 
				retinaDisplayPresent ? HEIGHT_RETINA_MM : HEIGHT_DELL_MM,
				DIST_EYE_MM);
   
    return 0;
}

