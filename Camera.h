#pragma once


// Std. Includes
#include <vector>
#include <iostream>

// GL Includes
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const GLfloat YAW        = -90.0f;
const GLfloat PITCH      =  0.0f;
const GLfloat SPEED      =  100.0f;
const GLfloat SENSITIVTY =  0.25f;
const GLfloat ZOOM       =  29.0f;
const GLfloat WIDTH      =  100.0f;
const GLfloat HEIGHT     =  100.0f;
const GLfloat DISTANCE   =  560.0f;
const GLfloat ZNEAR      =  560.0f;
const GLfloat ZFAR       =  1000.0f;

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:

    // Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), GLfloat screen_width = WIDTH, GLfloat screen_height = HEIGHT, GLfloat screen_distance = DISTANCE, GLfloat far = ZFAR) : yaw(YAW), pitch(PITCH), front(glm::vec3(0.0f, 0.0f, -1.0f)), movement_speed(SPEED), mouse_sensitivity(SENSITIVTY), zoom(ZOOM)
    {
		this->position = position;
		this->screen_width = screen_width;
		this->screen_height = screen_height;
		this->screen_distance = screen_distance;
		this->far = far;
		this->world_up = glm::vec3(0.0f, 1.0f, 0.0f);
        this->updateCameraVectors();
    }
    // Constructor with scalar values
	Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movement_speed(SPEED), mouse_sensitivity(SENSITIVTY), zoom(ZOOM)
    {
		this->position = glm::vec3(posX, posY, posZ);
		this->world_up = glm::vec3(upX, upY, upZ);
        this->yaw = yaw;
        this->pitch = pitch;
        this->updateCameraVectors();
    }

	glm::vec3 getPosition()
	{
		return position;
	}

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 getViewMatrix()
    {
		// Build transformation matrix to position slices in middle of clipping volume and scale to fill screen
		float scaleRatio = (screen_distance + ( far - screen_distance ) / 2.f ) / screen_distance;
		glm::mat4 translate_mat = glm::translate(glm::mat4(1.f), glm::vec3(0.0f, 0.0f, -1000.0f)); // Identity matrix
		glm::mat4 scale_mat = glm::scale(glm::mat4(1.f), glm::vec3(scaleRatio)); // Identity matrix
		glm::mat4 xform_mat = translate_mat * scale_mat;

        return glm::lookAt(this->position, this->position + this->front, this->up) * xform_mat;
    }

	glm::mat4 getProjectionMatrix()
	{
		//return glm::perspective(glm::radians(zoom), (GLfloat)WIDTH / (GLfloat)HEIGHT, ZNEAR, ZFAR);
		//return glm::frustum(-WIDTHMM * 0.5f, WIDTHMM * 0.5f, -HEIGHTMM * 0.5f, HEIGHTMM * 0.5f, ZNEAR, ZFAR);
		return glm::frustum(-screen_width * 0.5f, screen_width * 0.5f,
							-screen_height * 0.5f, screen_height * 0.5f,
							 screen_distance, far);
	}

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void processKeyboard(Camera_Movement direction, GLfloat deltaTime)
    {
        GLfloat velocity = this->movement_speed * deltaTime;
        if (direction == FORWARD)
			this->position += this->front * velocity;
        if (direction == BACKWARD)
			this->position -= this->front * velocity;
        if (direction == LEFT)
			this->position -= this->right * velocity;
        if (direction == RIGHT)
			this->position += this->right * velocity;
    }

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void processMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true)
    {
		xoffset *= this->mouse_sensitivity;
		yoffset *= this->mouse_sensitivity;

		this->yaw += xoffset;
		this->pitch += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
			if (this->pitch > 89.0f)
				this->pitch = 89.0f;
			if (this->pitch < -89.0f)
				this->pitch = -89.0f;
        }

        // Update Front, Right and Up Vectors using the updated Eular angles
        this->updateCameraVectors();
    }

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void processMouseScroll(GLfloat yoffset)
    {
		if (this->zoom >= 1.0f && this->zoom <= 60.0f)
			this->zoom -= yoffset;
		if (this->zoom <= 1.0f)
			this->zoom = 1.0f;
		if (this->zoom >= 60.0f)
			this->zoom = 60.0f;
		//std::cout<<"Offset: "<<yoffset<<" Zoom: "<<this->Zoom<<std::endl;
    }

private:
	// Camera Attributes
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 world_up;
	// Eular Angles
	GLfloat yaw;
	GLfloat pitch;
	// Camera options
	GLfloat movement_speed;
	GLfloat mouse_sensitivity;
	GLfloat zoom;
	GLfloat screen_width, screen_height, screen_distance, far;

    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // Calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
        front.y = sin(glm::radians(this->pitch));
        front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
        this->front = glm::normalize(front);
        // Also re-calculate the Right and Up vector
        this->right = glm::normalize(glm::cross(this->front, this->world_up));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        this->up    = glm::normalize(glm::cross(this->right, this->front));
    }
};