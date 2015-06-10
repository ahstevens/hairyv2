/**
 * SweepSurface.h - a class implementation representing a swept surface object
 *           		in OpenGL
 *
 * Author: dhs
 * Dec 6, 2014
 */

#ifndef SWEEPSURFACE_H_
#define SWEEPSURFACE_H_

#include "Object3D.h"

class SweepSurface: public Object3D
{
public:
    SweepSurface();
    SweepSurface( std::vector<glm::vec2> poly,
    		      std::vector<glm::vec3> path,
				  std::vector<glm::vec2> scales,
				  std::vector<float> rots );
    virtual ~SweepSurface();
   


    void updatePolygon( std::vector<glm::vec2> poly );
    void updatePath( std::vector<glm::vec3> p );
    void updateScales( std::vector<glm::vec2> s );
    void updateRotations( std::vector<float> rots );

    void enableWireframe();
    void disableWireframe();
    void enableNormals();
    void disableNormals();
    void enablePath();
    void disablePath();

    void drawWireframe();
    void drawSkin();
    void drawNormals();
    void drawPath();

    virtual void redraw();
   
protected:
    void makeRibs();
    void computePhongNormals();

    std::vector<glm::vec4> polygon;
    std::vector<glm::vec4> path;
    std::vector<glm::vec2> scales;
    std::vector<float> rotations;

    std::vector<glm::vec4> point_buffer;
    std::vector<glm::vec4> normal_buffer;

    int wireframe, draw_normals, draw_path;
};

#endif /*SWEEPSURFACE_H_*/
