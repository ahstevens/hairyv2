/**
 * SweepSurface.cpp - a class implementation representing a swept surface
 *                    object in OpenGL
 */
#include "SweepSurface.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <glm/gtc/type_ptr.hpp>

using namespace glm;

struct Vertex
{
	vec3 position;
	vec3 normal;
	vec2 texture;
};

//------------- constructor -----------------------
SweepSurface::SweepSurface( std::vector<vec2> polygon,
						    std::vector<vec3> path,
						    std::vector<vec2> scales,
						    std::vector<float> rotations )
{
	this->polygon = polygon;
	this->path = path;
	this->scales = scales;
	this->rotations = rotations;

	// make the vertices representing the ribs of the swept surface
	makeGeometry();
	computePhongNormals();
	computeTextureCoords();
	pack();

	geomChange = false;
	
	tex.stripes(8, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00);
	tex.setMinFilter(GL_NEAREST);
	tex.setMagFilter(GL_NEAREST);
}

//------------- destructor -----------------------
SweepSurface::~SweepSurface()
{
}

void SweepSurface::tube(int segments)
{
    float angleIncrement = 2.0f * M_PI / (float) segments;
    
    std::vector<vec2> circle;

    for( int i = segments - 1; i >= 0; --i )
        circle.push_back(vec2(float(sin(i * angleIncrement)) * 0.5f, 
                              float(cos(i * angleIncrement)) * 0.5f));

    this->polygon = circle;

	geomChange = true;
}

// setter class for sweep surface 2D polygon
void SweepSurface::updatePolygon( std::vector<vec2> polygon )
{
	this->polygon = polygon;

	geomChange = true;
}

// setter class for sweep surface path
void SweepSurface::updatePath( std::vector<vec3> path )
{
	this->path = path;

	geomChange = true;
}

// setter class for sweep surface 2D polygon scales
void SweepSurface::updateScales( std::vector<vec2> scales )
{
	this->scales = scales;

	geomChange = true;
}

// setter class for sweep surface 2D polygon rotations (in degrees)
void SweepSurface::updateScales( float scales )
{
	std::vector<vec2>::iterator it;
	for( it = this->scales.begin(); it != this->scales.end(); it++ )
		*it = vec2( scales, scales );

	geomChange = true;
}

// setter class for sweep surface 2D polygon rotations (in degrees)
void SweepSurface::updateRotations( std::vector<float> rotations )
{
	this->rotations = rotations;

	geomChange = true;
}

// setter class for sweep surface 2D polygon rotations (in degrees)
void SweepSurface::updateRotations( float rotations )
{
	std::vector<float>::iterator it;
	for( it = this->rotations.begin(); it != this->rotations.end(); it++ )
		*it = rotations;

	geomChange = true;
}

/*
 * makeGeometry() builds a scaled and rotated uvw coordinate frame for each 
 * point along the path, then uses that frame coordinate matrix to transform 
 * each point of the defined 2D polygon to get a representation of the polygon
 * at that point on the path.
 */
void SweepSurface::makeGeometry()
{
	int pathSize = path.size();

	// make sure points vector is empty before pushing new points
	vertex_buffer.clear();

	mat4 coordFrameTrans;

	// for all points along the defined path
	for( unsigned int i = 0; i < pathSize; i++ )
	{
		vec3 w;

		if( i == 0 ) {
			if( pathSize > 1 ) {
				w = normalize( vec3( path[ i ].x - path[ i + 1 ].x,
					 path[ i ].y - path[ i + 1 ].y,
					 path[ i ].z - path[ i + 1 ].z ) );
			}
			else {
				w = vec3( 0.0, 0.0, 1.0 ); // normal z axis
			}			
		}
		else if( i == pathSize - 1 ){
			// current path point minus prev path point gives vector pointing
			// towards prev polygon's center.
			w = normalize( vec3( path[ i - 1 ].x - path[ i ].x,
								 path[ i - 1 ].y - path[ i ].y,
								 path[ i - 1 ].z - path[ i ].z ) );
		}
		else {
			w = normalize( vec3( path[ i ].x - path[ i + 1 ].x,
								 path[ i ].y - path[ i + 1 ].y,
								 path[ i ].z - path[ i + 1 ].z) +
						   vec3( path[ i - 1 ].x - path[ i ].x,
								 path[ i - 1 ].y - path[ i ].y,
								 path[ i - 1 ].z - path[ i ].z ) );
		}

		// calculate up vector for angle offset
		vec3 up = vec3( -sin( radians( rotations[ i ] ) ),
						 cos( radians( rotations[ i ] ) ),
						 0.0 );

		// u is orthogonal to up vector and w vector
		vec3 u = normalize( cross( up, w ) );

		// v vector orthogonal to w vector and u vector
		vec3 v = normalize( cross( w, u ) );

		// scale the polygon
		u = scales[ i ].x * u;
		v = scales[ i ].y * v;

		// build coordinate frame transformation matrix at path point
		coordFrameTrans = mat4( vec4( u, 0.0f ),
								vec4( v, 0.0f ),
								vec4( w, 0.0f ),
								vec4(path[i], 1.0f ) ); // *pathLenMultiplier );
				
		std::vector<vec2>::iterator it;

		// push back front endcap vertices
		if(i == 0) {
			// place origin of polygon
			vertex_buffer.push_back( vec3( coordFrameTrans * vec4( 0.0f, 0.0f, 0.0f, 1.0f ) ) );
			// place polygon endcap points
			for( it = polygon.begin(); it != polygon.end(); it++ )
				vertex_buffer.push_back( vec3( coordFrameTrans * vec4( *it, 0.0f, 1.0f ) ) );
		}

		// push back transformed polygon points as a new "rib" of the surface
		for( it = polygon.begin(); it != polygon.end(); it++ )
			vertex_buffer.push_back( vec3( coordFrameTrans * vec4( *it, 0.0f, 1.0f ) ) );		
		vertex_buffer.push_back( vec3( coordFrameTrans * vec4( polygon.front(), 0.0f, 1.0f ) ) );
	}

	std::vector<vec2>::iterator it;
	for (it = polygon.begin(); it != polygon.end(); it++)
		vertex_buffer.push_back(vec3(coordFrameTrans * vec4(*it, 0.0f, 1.0f)));
	// place origin of polygon at last rib, so we can make endcap
	vertex_buffer.push_back( vec3( coordFrameTrans * vec4( 0.0f, 0.0f, 0.0f, 1.0f ) ) );
}

// utility function for calculating a weighted normal based on angle size
vec3 getWeightedNormal(vec3 a, vec3 b)
{
	if (a != b && length(a) != 0 &&	length(b) != 0)
	{
		float cosTheta = dot(a, b) / (length(a) * length(b));
		if (cosTheta >= -1 && cosTheta <= 1)
			return acos(cosTheta) * cross(a, b);
	}

	return vec3(0.0);
}

/*
 * computePhongNormals() calculates per-vertex normals, weighting each triangle
 * normal's contribution to the vertex normal by its angle at the vertex
 */
void SweepSurface::computePhongNormals()
{
	normal_buffer.clear();

	int polySize = polygon.size();
	int ribSize = polySize + 1;
	int endCapSize = polySize + 1;
	int nVerts = vertex_buffer.size();
	float dotProd, cosTheta, theta;
	vec3 normal;
	// vectors from current vertex to next vertex, previous vertex,
	// adjacent vertex in the next or previous polygon, the adjacent
	// previous vertex, and the adjacent next vertex
	vec3 vertCur, vecNext, vecPrev, vecAdj, vecAdjNext, vecAdjPrev;

	// calculate the normal of the front endcap
	if( path.size() > 1 )
		normal = normalize( vec3( path[ 0 ].x - path[ 1 ].x,
			 path[ 0 ].y - path[ 1 ].y,
			 path[ 0 ].z - path[ 1 ].z ) );
	else
		normal = vec3( 0.0, 0.0, 1.0 );

	int i;
	// push endcap normals
	for( i = 0; i < endCapSize; ++i)
		normal_buffer.push_back( normal );

	// calculate swept surface normals
	for( i; i < nVerts - endCapSize; i++ ) {
		normal.x = normal.y = normal.z = 0.0;
		vertCur = vertex_buffer[ i ];

		if( i % ribSize == polySize ) {
			normal_buffer.push_back( normal_buffer[i - polySize] );
			continue;
		}		
		// calculate normals for faces on previous path side if not at begin
		if( i >= endCapSize + ribSize ) {
			vecAdj = vertex_buffer[ i - ribSize ] - vertCur;
			// check if i is first point in polygon to get proper prev point
			if( i % ribSize == 0 ) {
				vecPrev = vertex_buffer[ i + polySize - 1 ] - vertCur;				
				vecAdjPrev = vertex_buffer[ i - ribSize + polySize - 1 ] - vertCur;
			}
			else {
				vecPrev = vertex_buffer[ i - 1 ] - vertCur;
				vecAdjPrev = vertex_buffer[ i - ribSize - 1 ] - vertCur;
			}
			// check if i is last point in polygon to get proper next point
			if( i % ribSize == polySize - 1 ) {
				vecNext = vertex_buffer[ i - polySize + 1 ] - vertCur;
				vecAdjNext = vertex_buffer[ i - ribSize - polySize + 1 ] - vertCur;
			}
			else {
				vecNext = vertex_buffer[ i + 1 ] - vertCur;
				vecAdjNext = vertex_buffer[ i - ribSize + 1 ] - vertCur;
			}
			
			normal += getWeightedNormal(vecNext, vecAdjNext);     // triangle 1
			normal += getWeightedNormal(vecAdjNext, vecAdj);      // triangle 2
			normal += getWeightedNormal(vecAdj, vecAdjPrev);      // triangle 3
			normal += getWeightedNormal(vecAdjPrev, vecPrev);     // triangle 4
		}

		// calculate normals for faces on the next path side if not at end
		if( i < nVerts - endCapSize - ribSize ) {
			vecAdj = vertex_buffer[ i + ribSize ] - vertCur;
			
			// check if i is first point in polygon to get proper previous point
			if( i % ribSize == 0 ) {
				vecPrev = vertex_buffer[ i + polySize - 1 ] - vertCur;
				vecAdjPrev = vertex_buffer[ i + ribSize + polySize - 1 ] - vertCur;
			}
			else {
				vecPrev = vertex_buffer[ i - 1 ] - vertCur;
				vecAdjPrev = vertex_buffer[ i + ribSize - 1 ] - vertCur;
			}

			// check if i is last point in polygon to get proper next point
			if( i % ribSize == polySize - 1 ) {
				vecNext = vertex_buffer[ i - polySize + 1 ] - vertCur;
				vecAdjNext = vertex_buffer[ i + ribSize - polySize + 1 ] - vertCur;
			}
			else {
				vecNext = vertex_buffer[ i + 1 ] - vertCur;
				vecAdjNext = vertex_buffer[ i + ribSize + 1 ] - vertCur;
			}
			
			normal += getWeightedNormal(vecPrev, vecAdjPrev);     // triangle 5
			normal += getWeightedNormal(vecAdjPrev, vecAdj);      // triangle 6
			normal += getWeightedNormal(vecAdj, vecAdjNext);      // triangle 7
			normal += getWeightedNormal(vecAdjNext, vecNext);     // triangle 8
		}
		// add normalized normal to the vector
		normal_buffer.push_back( normalize( normal ) );
	}

	// calculate back endcap normal
	if( path.size() > 1 )
		normal = normalize( vec3( path[ path.size() - 1 ].x - path[ path.size() - 2 ].x,
			 path[ path.size() - 1 ].y - path[ path.size() - 2 ].y,
			 path[ path.size() - 1 ].z - path[ path.size() - 2 ].z ) );
	else
		normal = vec3( 0.0, 0.0, -1.0 );

	// push back endcap normals
	for (int i = 0; i < polySize + 1; ++i)
		normal_buffer.push_back(normal);
}

void SweepSurface::computeTextureCoords()
{
	texture_buffer.clear();

	int pathSize = path.size();
	int polySize = polygon.size();
	int ribSize = polySize + 1;
	int pntBuffSize = vertex_buffer.size();

	float s, t;

	// front endcap
	texture_buffer.push_back(vec2(0.5f));
	for (int i = 0; i < polySize; ++i) {
		texture_buffer.push_back(vec2(0.5f) + polygon[i]);
	}

	// surface segments
	for (int i = 0; i < pathSize; i++) {
		for (int j = 0; j < ribSize; j++) {
			s = (float) j / polySize;
			t = i;                                  // texture per segment
			//t = (float) i / pathSize;               // texture per object
			texture_buffer.push_back(vec2(s, t));	
		}
	}

	// back endcap
	for (int i = 0; i < polySize; ++i) {
		texture_buffer.push_back(vec2(0.5f) + polygon[i]);
	}

	texture_buffer.push_back(vec2(0.5f));
}

void SweepSurface::pack()
{
	int nVerts = vertex_buffer.size();
	Vertex* verts = new Vertex[nVerts];

	std::cout << "Vertex Buffer Size: " << vertex_buffer.size() << std::endl;
	std::cout << "Normal Buffer Size: " << normal_buffer.size() << std::endl;
	std::cout << "Texture Buffer Size: " << texture_buffer.size() << std::endl;

	for (int i = 0; i < nVerts; ++i)
	{
		verts[i].position = vertex_buffer[i];
		verts[i].normal = normal_buffer[i];
		verts[i].texture = texture_buffer[i];
		//std::cout << "Position: " << verts[i].x << ", " << verts[i].y << ", " << verts[i].z << std::endl;
		//std::cout << "Normal:   " << verts[i].nx << ", " << verts[i].ny << ", " << verts[i].nz << std::endl;
		//std::cout << "Texture:   " << verts[i].s << ", " << verts[i].t <<  std::endl << std::endl;
	}

	int pathSize = path.size();
	int polySize = polygon.size();
	int ribSize = polySize + 1;

	indices.clear();

	// triangles for front endcap
	for (int i = 1; i < polySize; ++i)
	{
		indices.push_back(0);
		indices.push_back(i);
		indices.push_back(i + 1);
	}

	indices.push_back(0);
	indices.push_back(polySize);
	indices.push_back(1);

	// create strips of triangles connecting ribs together along path
	for( int i = 1; i < pathSize; i++ ) {
		int j;
		for( j = ribSize * i; j < ribSize * ( i + 1 ) - 1; j++ ) {
			//triangle 1
			indices.push_back( j );
			indices.push_back( j + ribSize );
			indices.push_back( j + 1 );
			//triangle 2
			indices.push_back( j + ribSize + 1 );
			indices.push_back( j + 1 );
			indices.push_back( j + ribSize );
		}
	}

	// triangles for back endcap
	GLuint end = vertex_buffer.size() - 1;
	for (int i = end - polySize; i < end - 1; ++i)
	{
		indices.push_back(end);
		indices.push_back(i + 1);
		indices.push_back(i);
	}

	indices.push_back(end);
	indices.push_back(end - polySize);
	indices.push_back(end - 1);
	
	// set up VAO
	glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, nVerts * sizeof(Vertex), verts, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW );

		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
		glEnableVertexAttribArray(0);
		// Normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(offsetof(Vertex, normal)));
		glEnableVertexAttribArray(1);
		// Texture coordinate attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(offsetof(Vertex, texture)));
		glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	delete[] verts;
}

//------------- redraw ---------------------------
void SweepSurface::redraw( Shader shader )
{
	
	if( geomChange ) {
		makeGeometry();
		computePhongNormals();
		computeTextureCoords();
		pack();
		geomChange = false;
	}
	
	glActiveTexture(GL_TEXTURE0);
	tex.enable();
		
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 
					   1,
					   GL_FALSE,
					   glm::value_ptr(getModelMatrix())
					   );

	// Draw the container (using container's vertex attributes)
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
