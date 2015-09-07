/**
 * SweepSurface.cpp - a class implementation representing a swept surface
 *                    object in OpenGL
 */
#include "SweepSurface.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <glm/gtc/type_ptr.hpp>

using namespace glm;

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

	geomChange = true;
	
	use_texture = true;
	tex.stripes(16, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF);
	tex.setMinFilter(GL_NEAREST);
	tex.setMagFilter(GL_NEAREST);
}

SweepSurface::SweepSurface( std::vector<vec2> polygon,
						    std::vector<vec3> path ) : scales( path.size(), vec2( 1.0f, 1.0f ) ), rotations( path.size(), 0.0f )
{
	this->polygon = polygon;
	this->path = path;

	geomChange = true;
}

//------------- destructor -----------------------
SweepSurface::~SweepSurface()
{
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
void SweepSurface::computeGeometry()
{
	int pathSize = path.size();

	// make sure points vector is empty before pushing new points
	vertex_buffer.clear();

	mat4 coordFrameTrans;

	// for all points along the defined path
	for( int i = 0; i < pathSize; i++ )
	{
		vec3 w;

		if( i == 0 ) {
			if( pathSize > 1 ) {
				w = normalize( path[ i ] - path[ i + 1 ] );
			}
			else {
				w = vec3( 0.0, 0.0, -1.0 ); // normal z axis
			}			
		}
		else if( i == pathSize - 1 ){
			// current path point minus prev path point gives vector pointing
			// towards prev polygon's center.
			w = normalize( path[ i - 1 ] - path[ i ] );
		}
		else {
			w = normalize( path[ i ] - path[ i + 1 ] );
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
		return cross(a, b);
	
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
			t = (float) i;                                  // texture per segment
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

void SweepSurface::computeIndices()
{
	GLsizei pathSize = path.size();
	GLsizei polySize = polygon.size();
	GLsizei ribSize = polySize + 1;

	index_buffer.clear();

	// triangles for front endcap
	for (GLsizei i = 1; i < polySize; ++i)
	{
		index_buffer.push_back(0);
		index_buffer.push_back(i);
		index_buffer.push_back(i + 1);
	}

	index_buffer.push_back(0);
	index_buffer.push_back(polySize);
	index_buffer.push_back(1);

	// create strips of triangles connecting ribs together along path
	for (GLsizei i = 1; i < pathSize; i++) {
		int j;
		for (j = ribSize * i; j < ribSize * (i + 1) - 1; j++) {
			//triangle 1
			index_buffer.push_back(j);
			index_buffer.push_back(j + ribSize);
			index_buffer.push_back(j + 1);
			//triangle 2
			index_buffer.push_back(j + ribSize + 1);
			index_buffer.push_back(j + 1);
			index_buffer.push_back(j + ribSize);
		}
	}

	// triangles for back endcap
	GLsizei end = vertex_buffer.size() - 1;
	for (GLsizei i = end - polySize; i < end - 1; ++i)
	{
		index_buffer.push_back(end);
		index_buffer.push_back(i + 1);
		index_buffer.push_back(i);
	}

	index_buffer.push_back(end);
	index_buffer.push_back(end - polySize);
	index_buffer.push_back(end - 1);
}

void SweepSurface::pack()
{
	int nVerts = vertex_buffer.size();
	Vertex* verts = new Vertex[nVerts];

	for (int i = 0; i < nVerts; ++i)
	{
		verts[i].position = vertex_buffer[i];
		verts[i].normal = normal_buffer[i];
		verts[i].texture = texture_buffer[i];
	}
		
	// set up VAO
	glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, nVerts * sizeof(Vertex), verts, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * index_buffer.size(), &index_buffer[0], GL_STATIC_DRAW);

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

std::vector<Object::Vertex> SweepSurface::getVertices()
{
	update( false );

	int nVerts = vertex_buffer.size();
	std::vector<Vertex> verts(nVerts);

	for (int i = 0; i < nVerts; ++i)
	{
		verts[i].position = vertex_buffer[i];
		verts[i].normal = normal_buffer[i];
		verts[i].texture = texture_buffer[i];
	}

	return verts;
}

std::vector<GLuint> SweepSurface::getIndices()
{
	update( false );

	return index_buffer;
}

void SweepSurface::update( bool pack = true )
{
	if( geomChange ) {
		computeGeometry();
		computePhongNormals();
		computeTextureCoords();
		computeIndices();
		if( pack ) this->pack();
		geomChange = false;
	}
}

//------------- redraw ---------------------------
void SweepSurface::redraw()
{
	update();

	glUniform3f(glGetUniformLocation(shader->Program, "material.ambient"), 
		mat.getAmbientColor().r, mat.getAmbientColor().g, mat.getAmbientColor().b);
	glUniform3f(glGetUniformLocation(shader->Program, "material.diffuse"), 
		mat.getDiffuseColor().r, mat.getDiffuseColor().g, mat.getDiffuseColor().b);
	glUniform3f(glGetUniformLocation(shader->Program, "material.specular"), 
		mat.getSpecularColor().r, mat.getSpecularColor().g, mat.getSpecularColor().b);
	glUniform1f(glGetUniformLocation(shader->Program, "material.shininess"), 
		mat.getShininess());
	glUniform1i(glGetUniformLocation(shader->Program, "use_texture"),
		use_texture);
		
	//glActiveTexture(GL_TEXTURE0);
	if (use_texture) tex.enable();
		
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "model"), 
					   1,
					   GL_FALSE,
					   glm::value_ptr(getModelMatrix())
					   );

	// Draw the container (using container's vertex attributes)
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, index_buffer.size(), GL_UNSIGNED_INT, (GLvoid*) 0);
	glBindVertexArray(0);

	if (use_texture) tex.disable();
}
