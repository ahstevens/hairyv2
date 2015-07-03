/**
 * SweepSurface.cpp - a class implementation representing a swept surface
 *                    object in OpenGL
 *
 * Author: dhs
 * Dec 6, 2014
 */

#include "SweepSurface.h"

struct Vertex
{
	float x, y, z;
	float nx, ny, nz;
};

//------------- constructor -----------------------
SweepSurface::SweepSurface()
{
	// default shape is an uninteresting square
	polygon.push_back( vec2( -0.5, -0.5 ) );
	polygon.push_back( vec2( 0.5, -0.5 ) );
	polygon.push_back( vec2( 0.5, 0.5 ) );
	polygon.push_back( vec2( -0.5, 0.5 ) );

	// default path is straight along z axis
	path.push_back( vec3( 0.0, 0.0, 0.0 ) );
	path.push_back( vec3( 0.0, 0.0, 0.2 ) );
	path.push_back( vec3( 0.0, 0.0, 0.4 ) );
	path.push_back( vec3( 0.0, 0.0, 0.6 ) );
	path.push_back( vec3( 0.0, 0.0, 0.8 ) );
	path.push_back( vec3( 0.0, 0.0, 1.0 ) );
	path.push_back( vec3( 0.0, 0.0, 1.2 ) );
	path.push_back( vec3( 0.0, 0.0, 1.4 ) );
	path.push_back( vec3( 0.0, 0.0, 1.6 ) );
	path.push_back( vec3( 0.0, 0.0, 1.8 ) );
	path.push_back( vec3( 0.0, 0.0, 2.0 ) );

	// default scale is linear downscale to 0
	scales.push_back( vec2( 1.0, 1.0 ) );
	scales.push_back( vec2( 0.9, 0.9 ) );
	scales.push_back( vec2( 0.8, 0.8 ) );
	scales.push_back( vec2( 0.7, 0.7 ) );
	scales.push_back( vec2( 0.6, 0.6 ) );
	scales.push_back( vec2( 0.5, 0.5 ) );
	scales.push_back( vec2( 0.4, 0.4 ) );
	scales.push_back( vec2( 0.3, 0.3 ) );
	scales.push_back( vec2( 0.2, 0.2 ) );
	scales.push_back( vec2( 0.1, 0.1 ) );
	scales.push_back( vec2( 0.0, 0.0 ) );

	// twist 180 degrees in the CCW direction
	rotations.push_back( 0.0f );
	rotations.push_back( -18.0f );
	rotations.push_back( -36.0f );
	rotations.push_back( -54.0f );
	rotations.push_back( -72.0f );
	rotations.push_back( -90.0f );
	rotations.push_back( -108.0f );
	rotations.push_back( -126.0f );
	rotations.push_back( -144.0f );
	rotations.push_back( -162.0f );
	rotations.push_back( -180.0f );
	rotations.push_back( -180.0f );

	// make the points representing the ribs of the swept surface
	makeRibs();
	computePhongNormals();
	pack();

	draw_skin = true;
	draw_wireframe = false;
	draw_path = false;
	draw_normals = false;
	use_gradient = true;
	geomChange = false;

	pathLenMultiplier = 1.0;
}

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
	makeRibs();
	computePhongNormals();
	pack();

	draw_skin = true;
	draw_wireframe = false;
	draw_path = false;
	draw_normals = false;
	use_gradient = true;
	geomChange = false;

	pathLenMultiplier = 1.0;
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

// setter class for sweep surface 2D polygon rotations (in degrees)
void SweepSurface::setPathLengthMultiplier( float m )
{
	pathLenMultiplier = m;

	geomChange = true;
}

/*
 * makeRibs() builds a scaled and rotated uvw coordinate frame for each point
 * along the path, then uses that frame coordinate matrix to transform each
 * point of the defined 2D polygon to get a representation of the polygon at
 * that point on the path.
 */
void SweepSurface::makeRibs()
{
	// make sure points vector is empty before pushing new points
	vertex_buffer.clear();

	mat4 coordFrameTrans;

	// for all points along the defined path
	for( unsigned int i = 0; i < path.size(); i++ )
	{
		vec3 w;

		if( i == 0 ) {
			if( path.size() > 1 ) {
				w = normalize( vec3( path[ i ].x - path[ i + 1 ].x,
					 path[ i ].y - path[ i + 1 ].y,
					 path[ i ].z - path[ i + 1 ].z ) );
			}
			else {
				w = vec3( 0.0, 0.0, 1.0 ); // normal z axis
			}

			// place origin of polygon at first rib, so we can make endcap
			//for( std::vector<vec4>::iterator it = polygon.begin(); it != polygon.end(); it++ )
				vertex_buffer.push_back( vec3( 0.0, 0.0, 0.0 ) );
		}
		else {
			// current path point minus prev path point gives vector pointing
			// towards prev polygon's center.
			w = normalize( vec3( path[ i - 1 ].x - path[ i ].x,
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
		coordFrameTrans = mat4( vec4( u, 0.0 ),
								vec4( v, 0.0 ),
								vec4( w, 0.0 ),
								vec4(path[i], 1.0 ) ); // *pathLenMultiplier );

		
		// push back transformed polygon points as a new "rib" of the surface
		std::vector<vec2>::iterator it;
		for( it = polygon.begin(); it != polygon.end(); it++ )
			vertex_buffer.push_back( vec3( coordFrameTrans * vec4( *it, 0.0, 1.0 ) ) );

		if(i == 0)			
			for( it = polygon.begin(); it != polygon.end(); it++ )
				vertex_buffer.push_back( vec3( coordFrameTrans * vec4( *it, 0.0, 1.0 ) ) );
	}
	std::vector<vec2>::iterator it;
	for (it = polygon.begin(); it != polygon.end(); it++)
		vertex_buffer.push_back(vec3(coordFrameTrans * vec4(*it, 0.0, 1.0)));
	// place origin of polygon at last rib, so we can make endcap
	//for( std::vector<vec4>::iterator it = polygon.begin(); it != polygon.end(); it++ )
		vertex_buffer.push_back( vec3( coordFrameTrans * ( vec4( 0.0, 0.0, 0.0, 1.0 ) ) ) );
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
	int pntBuffSize = vertex_buffer.size();
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

	// push endcap normals
	for( int i = 0; i < polySize + 1; ++i)
		normal_buffer.push_back( normal );

	// calculate swept surface normals
	for( int i = 1 + polySize; i < pntBuffSize - polySize - 1; i++ ) {
		normal.x = normal.y = normal.z = 0.0;
		vertCur = vertex_buffer[ i ];

		// calculate normals for faces on previous path side if not at begin
		if( i >= polySize + 1) {
			vecAdj = vertex_buffer[ i - polySize ] - vertCur;
			// check if i is first point in polygon to get proper prev point
			if( i % polySize == 1 ) {
				vecPrev = vertex_buffer[ i + polySize - 1 ] - vertCur;
			}
			else {
				vecPrev = vertex_buffer[ i - 1 ] - vertCur;
			}
			// check if i is last point in polygon to get proper next point
			if( i % polySize == 0 ) {
				vecNext = vertex_buffer[ i - polySize + 1 ] - vertCur;
				vecAdjNext = vertex_buffer[ i - 2 * polySize + 1 ] - vertCur;
			}
			else {
				vecNext = vertex_buffer[ i + 1 ] - vertCur;
				vecAdjNext = vertex_buffer[ i - polySize + 1 ] - vertCur;
			}

			normal += getWeightedNormal(vecAdj, vecPrev);     // triangle 1
			normal += getWeightedNormal(vecAdjNext, vecAdj);  // triangle 2
			normal += getWeightedNormal(vecNext, vecAdjNext); // triangle 3
		}

		// calculate normals for faces on the next path side if not at end
		if( i < pntBuffSize - polySize - 1 ) {
			vecAdj = vertex_buffer[ i + polySize ] - vertCur;

			// check if i is last point in polygon to get proper next point
			if( i % polySize == 0 )
				vecNext = vertex_buffer[ i - polySize + 1 ] - vertCur;
			else
				vecNext = vertex_buffer[ i + 1 ] - vertCur;

			// check if i is first point in polygon to get proper previous point
			if( i % polySize == 1 ) {
				vecPrev = vertex_buffer[ i + polySize - 1 ] - vertCur;
				vecAdjPrev = vertex_buffer[ i + 2 * polySize - 1 ] - vertCur;
			}
			else {
				vecPrev = vertex_buffer[ i - 1 ] - vertCur;
				vecAdjPrev = vertex_buffer[ i + polySize - 1 ] - vertCur;
			}

			normal += getWeightedNormal(vecAdj, vecNext);     // triangle 4
			normal += getWeightedNormal(vecAdjPrev, vecAdj);  // triangle 5
			normal += getWeightedNormal(vecPrev, vecAdjPrev); // triangle 6
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

void SweepSurface::pack()
{
	int nVerts = vertex_buffer.size();
	Vertex* verts = new Vertex[nVerts];

	std::cout << "Vertex Buffer Size: " << vertex_buffer.size() << std::endl;
	std::cout << "Normal Buffer Size: " << normal_buffer.size() << std::endl;

	for (int i = 0; i < nVerts; ++i)
	{
		verts[i].x = vertex_buffer[i].x;
		verts[i].y = vertex_buffer[i].y;
		verts[i].z = vertex_buffer[i].z;
		verts[i].nx = normal_buffer[i].x;
		verts[i].ny = normal_buffer[i].y;
		verts[i].nz = normal_buffer[i].z;
		std::cout << "Position: " << verts[i].x << ", " << verts[i].y << ", " << verts[i].z << std::endl;
		std::cout << "Normal:   " << verts[i].nx << ", " << verts[i].ny << ", " << verts[i].nz << std::endl << std::endl;
	}

	int pathSize = path.size();
	int polySize = polygon.size();

	indices.clear();

	for (int i = 1; i < polySize; ++i)
	{
		indices.push_back(0);
		indices.push_back(i);
		indices.push_back(i+1);
	}

	indices.push_back(0);
	indices.push_back(polySize);
	indices.push_back(1);

	for( int i = 0; i < pathSize; i++ ) {
		int j;
		for( j = ( polySize * i ) + 1; j < ( polySize ) * ( i + 2 ); j++ ) {
			//triangle 1
			indices.push_back( j );
			indices.push_back( j + polySize );
			indices.push_back( j + polySize + 1 );
			//triangle 2
			indices.push_back( j );
			indices.push_back( j + polySize + 1 );
			indices.push_back( j + 1 );
		}
		
		indices.push_back( j );
		indices.push_back( j + polySize );
		indices.push_back( ( polySize * i ) + 1 + polySize );
				
		indices.push_back( j );
		indices.push_back( ( polySize * i ) + 1 + polySize );
		indices.push_back( ( polySize * i ) + 1 );
	}

	GLuint end = (pathSize + 2) * polySize + 1;
	std::cout << "End: " << end << std::endl;
	std::cout << "Size: " << vertex_buffer.size() << std::endl;
	for (int i = end - 1; i > end - polySize; --i)
	{
		indices.push_back(end);
		indices.push_back(i);
		indices.push_back(i - 1);
	}

	indices.push_back(end);
	indices.push_back(end - polySize);
	indices.push_back(end - 1);

	std::vector<GLuint>::iterator it;
	for (it = indices.begin(); it != indices.end(); it++)
	{
		std::cout << *it << " ";
	}
	std::cout << std::endl;
	//for (int i = 1 + ( pathSize + 1 ) * polySize; i < polySize; ++i)
	//{
	//	indices.push_back(0);
	//	indices.push_back(i);
	//	indices.push_back(i + 1);
	//}

	//indices.push_back(0);
	//indices.push_back(polySize);
	//indices.push_back(1);
		
	glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, nVerts * sizeof(Vertex), verts, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW );

		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		// Normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	delete[] verts;
}

//------------- redraw ---------------------------
void SweepSurface::redraw()
{
	
	if( geomChange ) {
		makeRibs();
		computePhongNormals();
		pack();
		geomChange = false;
	}
	
	// Draw the container (using container's vertex attributes)
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
