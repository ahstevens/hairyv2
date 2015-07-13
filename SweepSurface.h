/**
 * SweepSurface.h - a class implementation representing a swept surface object
 *           		in OpenGL
 *
 * Author: dhs
 * Dec 6, 2014
 */

#ifndef SWEEPSURFACE_H_
#define SWEEPSURFACE_H_

#include "Object.h"
#include <vector>

class SweepSurface: public Object
{
public:
    SweepSurface( std::vector<glm::vec2> polygon,
    		      std::vector<glm::vec3> path,
				  std::vector<glm::vec2> scales,
				  std::vector<float> rotations );
    virtual ~SweepSurface();
	void tube(int segments);
   
    void updatePolygon( std::vector<glm::vec2> polygon );
    void updatePath( std::vector<glm::vec3> path );
    void updateScales( std::vector<glm::vec2> scales );
    void updateScales( float scales );
    void updateRotations( std::vector<float> rotations );
    void updateRotations( float rotations );
	
    virtual void redraw( Shader shader );
   
protected:
    void computeGeometry();
    void computePhongNormals();
	void computeTextureCoords();
	void computeIndices();
	void pack();
	void update();

    std::vector<glm::vec2> polygon;
    std::vector<glm::vec3> path;
    std::vector<glm::vec2> scales;
    std::vector<float> rotations;

    std::vector<glm::vec3> vertex_buffer;
    std::vector<glm::vec3> normal_buffer;
	std::vector<glm::vec2> texture_buffer;

	std::vector<GLuint> index_buffer;

    bool geomChange;
};

#endif /*SWEEPSURFACE_H_*/
