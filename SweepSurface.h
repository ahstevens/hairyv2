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
    SweepSurface( std::vector<vec2> polygon,
    		      std::vector<vec3> path,
				  std::vector<vec2> scales,
				  std::vector<float> rotations );
    virtual ~SweepSurface();
   


    void updatePolygon( std::vector<vec2> polygon );
    void updatePath( std::vector<vec3> path );
    void updateScales( std::vector<vec2> scales );
    void updateScales( float scales );
    void updateRotations( std::vector<float> rotations );
    void updateRotations( float rotations );

    void setPathLengthMultiplier( float m );

	void enableSkin();
	void disableSkin();
    void enableWireframe();
    void disableWireframe();
    void enableNormals();
    void disableNormals();
    void enablePath();
    void disablePath();
    void enablePathGradient();
    void disablePathGradient();

    void drawWireframe();
    void drawSkin();
    void drawNormals();
    void drawPath();

    virtual void redraw();
   
protected:
    void makeRibs();
    void computePhongNormals();
	void pack();

    std::vector<vec2> polygon;
    std::vector<vec2> circle;
    std::vector<vec3> path;
    std::vector<vec2> scales;
    std::vector<float> rotations;

    std::vector<vec3> vertex_buffer;
    std::vector<vec3> normal_buffer;

    bool draw_skin, draw_wireframe, draw_normals, draw_path, use_gradient,
        geomChange;

    float pathLenMultiplier;
};

#endif /*SWEEPSURFACE_H_*/
