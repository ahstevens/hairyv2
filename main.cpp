#include <iostream>
#include <cmath>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other includes
#include "Shader.h"
#include "Camera.h"
#include "Light.h"

#include "Texture.h"
#include "SweepSurface.h"


// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();

// NOTE: the 9.7" 2048x1536 retina display area measures 196x157mm

// Window dimensions
const GLuint WIDTHMM = 196, HEIGHTMM = 157;

// Camera
Camera  camera(glm::vec3(0.0f, 0.0f, 10.0f));
GLfloat lastX  =  WIDTH  / 2.0;
GLfloat lastY  =  HEIGHT / 2.0;
bool    keys[1024];

// Light attributes
Light light;

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

bool draw_normals = false;

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
	GLFWmonitor* retinaDisplay = glfwGetPrimaryMonitor();
	for(int i = 0; i < count; ++i)
	{
		glfwGetMonitorPhysicalSize(monitors[i], &widthMM, &heightMM);
		std::cout << "Monitor " << i << ": " << widthMM << " x " << heightMM << std::endl;
		if(widthMM == 722 && heightMM == 542)
		{
			retinaDisplay = monitors[i];
			break;
		}
	}

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Hairy Slices", retinaDisplay, nullptr);
    glfwMakeContextCurrent(window);

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // GLFW Options
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    glewInit();

    // Define the viewport dimensions
    glViewport(0, 0, WIDTH, HEIGHT);

    // OpenGL options
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);


    // Build and compile our shader program
    Shader lightingShader("materials.vs", "materials.frag");
    Shader normalShader("normals.vs", "normals.frag", "normals.gs");

 	// SWEEPSURFACE
	std::vector<glm::vec2> poly;
	poly.push_back( glm::vec2( -0.5, -0.5 ) );
	poly.push_back( glm::vec2( 0.5, -0.5 ) );
	poly.push_back( glm::vec2( 0.5, 0.5 ) );
	poly.push_back( glm::vec2( -0.5, 0.5 ) );
	
	float length = 10.0f,
		  step = 0.5f;


	std::vector<glm::vec3> path;
	std::vector<glm::vec2> scales;
	std::vector<float> rots;
	for(float i = 0.0f; i < length; i += step) {
		path.push_back( glm::vec3( cos(i), sin(i), i ) );
		scales.push_back( glm::vec2( 1.0f, 1.0f ) );
		rots.push_back(0.0);
	}
	
	SweepSurface* s = new SweepSurface(poly, path, scales, rots);
	s->tube(32);
	
    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Calculate deltatime of current frame
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();
        do_movement();

        // Clear the colorbuffer
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // Use cooresponding shader when setting uniforms/drawing objects
        lightingShader.Use();

		glm::vec4 lightPos = light.getPosition();
		glUniform4f(glGetUniformLocation(lightingShader.Program, "light.position"), lightPos.x, lightPos.y, lightPos.z, lightPos.w);
		glm::vec3 cameraPos = camera.getPosition();
		glUniform3f(glGetUniformLocation(lightingShader.Program, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);

        // Set lights properties
		//glm::vec3 lightColor;
		//lightColor.x = 1.0f; //sin(glfwGetTime() * 2.0f);
		//lightColor.y = 1.0f; //sin(glfwGetTime() * 0.7f);
		//lightColor.z = 1.0f; //sin(glfwGetTime() * 1.3f);

		light.setColor( 1.0f, 1.0f, 1.0f );
		glm::vec3 ambientColor = light.getAmbientColor();
		glm::vec3 diffuseColor = light.getDiffuseColor();
        glm::vec3 specularColor = light.getSpecularColor(); 
        glUniform3f(glGetUniformLocation(lightingShader.Program, "light.ambient"),  ambientColor.r, ambientColor.g, ambientColor.b);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "light.diffuse"),  diffuseColor.r, diffuseColor.g, diffuseColor.b);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "light.specular"), specularColor.r, specularColor.g, specularColor.b);

        // Create camera transformations
        glm::mat4 view = camera.getViewMatrix();
        
		glm::mat4 projection = camera.getProjectionMatrix();
        // Get the uniform locations
        GLint viewLoc  = glGetUniformLocation(lightingShader.Program,  "view");
        GLint projLoc  = glGetUniformLocation(lightingShader.Program,  "projection");
        // Pass the matrices to the shader
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		
		s->redraw(lightingShader);

		if(draw_normals)
		{
			normalShader.Use();
			glUniformMatrix4fv(glGetUniformLocation(normalShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
			glUniformMatrix4fv(glGetUniformLocation(normalShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			// And draw model again, this time only drawing normal vectors using the geometry shaders (on top of previous model)
			s->redraw(normalShader);
		}

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }

    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS) {
            keys[key] = true;
			if (keys[GLFW_KEY_N])
				draw_normals = abs(draw_normals - 1);
		}
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void do_movement()
{
    // Camera controls
    if (keys[GLFW_KEY_W])
        camera.processKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.processKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.processKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.processKeyboard(RIGHT, deltaTime);
	if (keys[GLFW_KEY_K])
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (keys[GLFW_KEY_L])
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left

    lastX = xpos;
    lastY = ypos;

    camera.processMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.processMouseScroll(yoffset);
}