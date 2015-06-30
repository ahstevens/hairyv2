/**
 * SweepSurface.cpp - a class implementation representing a swept surface
 *                    object in OpenGL
 *
 * Author: dhs
 * Dec 6, 2014
 */

#include "SweepSurface.h"

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
	path.push_back( vec3( 0.0, 0.0, -0.2 ) );
	path.push_back( vec3( 0.0, 0.0, -0.4 ) );
	path.push_back( vec3( 0.0, 0.0, -0.6 ) );
	path.push_back( vec3( 0.0, 0.0, -0.8 ) );
	path.push_back( vec3( 0.0, 0.0, -1.0 ) );
	path.push_back( vec3( 0.0, 0.0, -1.2 ) );
	path.push_back( vec3( 0.0, 0.0, -1.4 ) );
	path.push_back( vec3( 0.0, 0.0, -1.6 ) );
	path.push_back( vec3( 0.0, 0.0, -1.8 ) );
	path.push_back( vec3( 0.0, 0.0, -2.0 ) );

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
		coordFrameTrans = transpose( mat4( vec4( u, 0.0 ),
										   vec4( v, 0.0 ),
										   vec4( w, 0.0 ),
										   vec4( path[ i ], 1.0 ) * pathLenMultiplier ) );

		// push back transformed polygon points as a new "rib" of the surface
		std::vector<vec2>::iterator it;
		for( it = polygon.begin(); it != polygon.end(); it++ )
			vertex_buffer.push_back( vec3( coordFrameTrans * vec4( *it, 0.0, 1.0 ) ) );
	}

	// place origin of polygon at last rib, so we can make endcap
	//for( std::vector<vec4>::iterator it = polygon.begin(); it != polygon.end(); it++ )
		vertex_buffer.push_back( vec3( coordFrameTrans * ( vec4( 0.0, 0.0, 0.0, 1.0 ) ) ) );
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

	if( path.size() > 1 )
		normal = normalize( vec3( path[ 0 ].x - path[ 1 ].x,
			 path[ 0 ].y - path[ 1 ].y,
			 path[ 0 ].z - path[ 1 ].z ) );
	else
		normal = vec3( 0.0, 0.0, 1.0 );

	normal_buffer.push_back( normal );

	for( int i = 1; i < pntBuffSize - 1; i++ ) {
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

			// weighted normal for triangle 1
			if( vecAdj != vecPrev &&
				length( vecAdj ) != 0 &&
				length( vecPrev ) != 0 )
			{
				dotProd = dot( vecAdj, vecPrev );
				cosTheta = dotProd / \
						   ( length( vecAdj ) * length ( vecPrev ) );
				if( cosTheta >= -1 && cosTheta <= 1 )
				{
					theta = acos( cosTheta );
					normal += theta * cross( vecAdj, vecPrev );
				}
			}

			// weighted normal for triangle 2
			if( vecAdjNext != vecAdj &&
				length( vecAdjNext ) != 0 &&
				length( vecAdj ) != 0 )
			{
				dotProd = dot( vecAdjNext, vecAdj );
				cosTheta = dotProd / \
						   ( length( vecAdjNext ) * length ( vecAdj ) );
				if( cosTheta >= -1 && cosTheta <= 1 )
				{
					theta = acos( cosTheta );
					normal += theta * cross( vecAdjNext, vecAdj );
				}
			}

			// weighted normal for triangle 3
			if( vecNext != vecAdjNext &&
				length( vecNext ) != 0 &&
				length( vecAdjNext ) != 0 )
			{
				dotProd = dot( vecNext, vecAdjNext );
				cosTheta = dotProd / \
						   ( length( vecNext ) * length ( vecAdjNext ) );
				if( cosTheta >= -1 && cosTheta <= 1 )
				{
					theta = acos( cosTheta );
					normal += theta * cross( vecNext, vecAdjNext );
				}
			}
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

			// weighted normal for triangle 4
			if( vecAdj != vecNext &&
				length( vecAdj ) != 0 &&
				length( vecNext ) != 0 )
			{
				dotProd = dot( vecAdj, vecNext );
				cosTheta = dotProd / \
						   ( length( vecAdj ) * length ( vecNext ) );
				if( cosTheta >= -1 && cosTheta <= 1 )
				{
					theta = acos( cosTheta );
					normal += theta * cross( vecAdj, vecNext );
				}
			}

			// weighted normal for triangle 5
			if( vecAdjPrev != vecAdj &&
				length( vecAdjPrev ) != 0 &&
				length( vecAdj ) != 0 )
			{
				dotProd = dot( vecAdjPrev, vecAdj );
				cosTheta = dotProd / \
						   ( length( vecAdjPrev ) * length ( vecAdj ) );
				if( cosTheta >= -1 && cosTheta <= 1 )
				{
					theta = acos( cosTheta );
					normal += theta * cross( vecAdjPrev, vecAdj );
				}
			}

			// weighted normal for triangle 6
			if( vecPrev != vecAdjPrev &&
				length( vecPrev ) != 0 &&
				length( vecAdjPrev ) != 0 )
			{
				dotProd = dot( vecPrev, vecAdjPrev );
				cosTheta = dotProd / \
						   ( length( vecPrev ) * length ( vecAdjPrev ) );
				if( cosTheta >= -1 && cosTheta <= 1 )
				{
					theta = acos( cosTheta );
					normal += theta * cross( vecPrev, vecAdjPrev );
				}
			}
		}
		// add normalized normal to the vector
		normal_buffer.push_back( normalize( normal ) );
	}

	if( path.size() > 1 )
		normal = normalize( vec3( path[ path.size() - 1 ].x - path[ path.size() - 2 ].x,
			 path[ path.size() - 1 ].y - path[ path.size() - 2 ].y,
			 path[ path.size() - 1 ].z - path[ path.size() - 2 ].z ) );
	else
		normal = vec3( 0.0, 0.0, -1.0 );

	normal_buffer.push_back( normal );
}

void SweepSurface::pack()
{
	int polySize = polygon.size();
	float *data = new float[ 3 * 2 * vertex_buffer.size() ];

		for( int i = 0; i < polySize + 1; ++i) {			
			glVertex3f( vertex_buffer[ i ].x,
						vertex_buffer[ i ].y, 
						vertex_buffer[ i ].z );
			glNormal3f( normalFront.x, normalFront.y, normalFront.z );
		}
		u = 1.0f;
		glTexCoord2f( u, v );
		glNormal3f( normalFront.x, normalFront.y, normalFront.z );
		glVertex3f( vertex_buffer[ 1 ].x,
					vertex_buffer[ 1 ].y, 
					vertex_buffer[ 1 ].z );
}

/*
* enable skin mode
*/
void SweepSurface::enableSkin() { draw_skin = true; }

/*
* disable skin mode
*/
void SweepSurface::disableSkin() { draw_skin = false; }

/*
 * enable wireframe mode
 */
void SweepSurface::enableWireframe() { draw_wireframe = true; }

/*
 * disable wireframe mode
 */
void SweepSurface::disableWireframe() { draw_wireframe = false; }

/*
 * enable drawing the Phong normals at the vertices
 */
void SweepSurface::enableNormals() { draw_normals = true; }

/*
 * disable drawing the Phong normals at the vertices
 */
void SweepSurface::disableNormals() { draw_normals = false; }

/*
 * enable drawing the path along which the surface sweeps
 */
void SweepSurface::enablePath() { draw_path = true; }

/*
 * disable drawing the path along which the surface sweeps
 */
void SweepSurface::disablePath() { draw_path = false; }

/*
 * enable drawing the path along which the surface sweeps
 */
void SweepSurface::enablePathGradient() { use_gradient = true; }

/*
 * disable drawing the path along which the surface sweeps
 */
void SweepSurface::disablePathGradient() { use_gradient = false; }

/*
 * Drawing method for wireframe mode. Disables textures and material properties
 * in order to draw a plain white wireframe that is uninfluenced by lighting
 */
void SweepSurface::drawWireframe()
{
	int polySize = polygon.size();
	int pathSize = path.size();

	// disable lighting, textures, material properties
	glDisable( GL_LIGHTING );
	//if( useTexture ) tex->disable();
	//if( useMaterial ) mat->disable();

	glColor3f( 1.0, 1.0, 1.0 ); // white wireframe

	// draw polygon ribs using line loop
	for( int i = 0; i < pathSize; i++ ) {
		glBegin( GL_LINE_LOOP );
			for( int j = ( polySize * i ) + 1; j < polySize * ( i + 1 ) + 1; j++) {
				glVertex3f( vertex_buffer[ j ].x,
						    vertex_buffer[ j ].y,
							vertex_buffer[ j ].z );
			}
		glEnd();
	}

	// draw wireframe for triangle skin mesh
	for( int i = 0; i < pathSize - 1; i++ ) {
		glBegin( GL_LINE_STRIP );
			for( int j = ( polySize * i ) + 1; j < polySize * ( i + 1 ) + 1; j++ ) {
				glVertex3f( vertex_buffer[ j ].x,
						    vertex_buffer[ j ].y,
							vertex_buffer[ j ].z );
				glVertex3f( vertex_buffer[ j + polySize ].x,
							vertex_buffer[ j + polySize ].y,
							vertex_buffer[ j + polySize ].z );
			}
			// connect back to beginning points
			glVertex3f( vertex_buffer[ ( polySize * i ) + 1 ].x,
						vertex_buffer[ ( polySize * i ) + 1 ].y,
						vertex_buffer[ ( polySize * i ) + 1 ].z );
			glVertex3f( vertex_buffer[ polySize * ( i + 1) + 1 ].x,
						vertex_buffer[ polySize * ( i + 1) + 1 ].y,
						vertex_buffer[ polySize * ( i + 1) + 1 ].z );
		glEnd();
	}

	glEnable( GL_LIGHTING );
}

/*
 * Drawing method for the "skin" of the object. Uses triangle strips to skin
 * between pairs of polygons along the path. Texture mapping approach is that
 * polygon points are mapped to the texture space's r axis, and path length is
 * mapped to the texture space's s axis.
 */
void SweepSurface::drawSkin()
{
	int polySize = polygon.size();
	int pathSize = path.size();

	// initialize texture coordinates
	float u = 0.0, v = 0.0;

	vec3 curVec, normalFront, normalBack;
	normalFront = normal_buffer.front();
	normalBack = normal_buffer.back();

	// draw front endcap
	glBegin( GL_TRIANGLE_FAN );
		for( int i = 0; i < polySize + 1; ++i) {
			u = i / (float) ( polySize + 1 );
			glTexCoord2f( u, v );
			glNormal3f( normalFront.x, normalFront.y, normalFront.z );
			glVertex3f( vertex_buffer[ i ].x,
						vertex_buffer[ i ].y, 
						vertex_buffer[ i ].z );
		}
		u = 1.0f;
		glTexCoord2f( u, v );
		glNormal3f( normalFront.x, normalFront.y, normalFront.z );
		glVertex3f( vertex_buffer[ 1 ].x,
					vertex_buffer[ 1 ].y, 
					vertex_buffer[ 1 ].z );
	glEnd();	

	// draw skin mesh using triangle strips
	for( int i = 0; i < pathSize - 1; i++ ) {
		glBegin( GL_TRIANGLE_STRIP );
			for( int j = ( polySize * i ) + 1; j < polySize * ( i + 1 ) + 1; j++ ) {
				u = (float) ( ( j - 1 ) % polySize ) / polySize;
				v = (float) i / ( pathSize - 1 );
				glTexCoord2f( u, v );
				glNormal3f( normal_buffer[ j ].x,
						    normal_buffer[ j ].y,
							normal_buffer[ j ].z );
				glVertex3f( vertex_buffer[ j ].x,
						    vertex_buffer[ j ].y,
							vertex_buffer[ j ].z );
				v = (float) ( i + 1 ) / ( pathSize - 1 );
				glTexCoord2f( u, v );
				glNormal3f( normal_buffer[ j + polySize ].x,
							normal_buffer[ j + polySize ].y,
							normal_buffer[ j + polySize ].z );
				glVertex3f( vertex_buffer[ j + polySize ].x,
							vertex_buffer[ j + polySize ].y,
							vertex_buffer[ j + polySize ].z );
			}
			// connect back to beginning polygon points to complete skin section
			u = 1.0;
			v = (float) i / ( pathSize - 1 );
			glTexCoord2f( u, v );
			glNormal3f( normal_buffer[ ( polySize * i ) + 1 ].x,
						normal_buffer[ ( polySize * i ) + 1 ].y,
						normal_buffer[ ( polySize * i ) + 1 ].z );
			glVertex3f( vertex_buffer[ ( polySize * i ) + 1 ].x,
						vertex_buffer[ ( polySize * i ) + 1 ].y,
						vertex_buffer[ ( polySize * i ) + 1 ].z );
			v = (float) ( i + 1 ) / ( pathSize - 1 );
			glTexCoord2f( u, v );
			glNormal3f( normal_buffer[ polySize * ( i + 1) + 1 ].x,
						normal_buffer[ polySize * ( i + 1) + 1 ].y,
						normal_buffer[ polySize * ( i + 1) ].z );
			glVertex3f( vertex_buffer[ polySize * ( i + 1) + 1 ].x,
						vertex_buffer[ polySize * ( i + 1) + 1 ].y,
						vertex_buffer[ polySize * ( i + 1) + 1 ].z );
		glEnd();
	}

	v = 0.99f;
	// draw back endcap
	glBegin( GL_TRIANGLE_FAN );
		for( int i = 0; i < polySize + 1; ++i) {
			u = i / (float) ( polySize + 1 );
			glTexCoord2f( u, v );
			glNormal3f( normalBack.x, normalBack.y, normalBack.z );
			glVertex3f( vertex_buffer[ vertex_buffer.size() - 1 - i ].x,
						vertex_buffer[ vertex_buffer.size() - 1 - i ].y, 
						vertex_buffer[ vertex_buffer.size() - 1 - i ].z );
		}
		u = 1.0;
		glTexCoord2f( u, v );
		glNormal3f( normalBack.x, normalBack.y, normalBack.z );
		glVertex3f( vertex_buffer[ vertex_buffer.size() - 2 ].x,
					vertex_buffer[ vertex_buffer.size() - 2 ].y, 
					vertex_buffer[ vertex_buffer.size() - 2 ].z );
	glEnd();
}

/*
 * Drawing method for vertex normals. Adds the normal to each vertex and draws
 * a line between the two to help show correct normal calculations.
 */
void SweepSurface::drawNormals()
{
	int polySize = polygon.size();
	int pathSize = path.size();

	// disable lighting, textures, material properties
	glDisable( GL_LIGHTING );
	//if( useTexture ) tex->disable();
	//if( useMaterial ) mat->disable();

	glColor3f( 1.0, 1.0, 0.0 ); // yellow normals

	for( int i = 0; i < pathSize - 1; i++ ) {
		glBegin( GL_LINES );
		for( int j = ( polySize * i ) + 1; j < polySize * ( i + 1 ) + 1; j++ ) {
			glVertex3f( vertex_buffer[ j ].x,
						vertex_buffer[ j ].y,
						vertex_buffer[ j ].z );
			glVertex3f( (vertex_buffer[ j ] + normal_buffer[ j ]).x,
						(vertex_buffer[ j ] + normal_buffer[ j ]).y,
						(vertex_buffer[ j ] + normal_buffer[ j ]).z );
			glVertex3f( vertex_buffer[ j + polySize ].x,
						vertex_buffer[ j + polySize ].y,
						vertex_buffer[ j + polySize ].z );
			glVertex3f( (vertex_buffer[ j + polySize ]\
							+ normal_buffer[ j + polySize ]).x,
						(vertex_buffer[ j + polySize ]\
							+ normal_buffer[ j + polySize ]).y,
						(vertex_buffer[ j + polySize ]\
							+ normal_buffer[ j + polySize ]).z );
		}
		// connect back to beginning vertex_buffer
		glVertex3f( vertex_buffer[ ( polySize * i ) + 1 ].x,
					vertex_buffer[ ( polySize * i ) + 1 ].y,
					vertex_buffer[ ( polySize * i ) + 1 ].z );
		glVertex3f( (vertex_buffer[ ( polySize * i ) + 1 ]\
						+ normal_buffer[ ( polySize * i ) + 1 ]).x,
					(vertex_buffer[ ( polySize * i ) + 1 ]\
						+ normal_buffer[ ( polySize * i ) + 1 ]).y,
					(vertex_buffer[ ( polySize * i ) + 1 ]\
						+ normal_buffer[ ( polySize * i ) + 1 ]).z );
		glVertex3f( vertex_buffer[ polySize * ( i + 1) + 1 ].x,
					vertex_buffer[ polySize * ( i + 1) + 1 ].y,
					vertex_buffer[ polySize * ( i + 1) + 1 ].z );
		glVertex3f( (vertex_buffer[ polySize * ( i + 1) + 1 ]\
						+ normal_buffer[ polySize * ( i + 1) + 1 ]).x,
					(vertex_buffer[ polySize * ( i + 1) + 1 ]\
						+ normal_buffer[ polySize * ( i + 1) + 1 ]).y,
					(vertex_buffer[ polySize * ( i + 1) + 1 ]\
						+ normal_buffer[ polySize * ( i + 1) + 1 ]).z );
		glEnd();
	}
	glEnable( GL_LIGHTING );
}

/*
 * Drawing method for the swept path.
 */
void SweepSurface::drawPath()
{
	// disable lighting, textures, material properties
	glDisable( GL_LIGHTING );
	//if( useTexture ) tex->disable();
	//if( useMaterial ) mat->disable();

	GLfloat* oldLW = new GLfloat [1];
	glGetFloatv(GL_LINE_WIDTH, oldLW);

    glLineWidth( scales.front().x * 2.0 );

	float fraction;
	glBegin( GL_LINE_STRIP );
		for( unsigned int i = 0; i < path.size(); i++ ) {
			fraction = i / ( path.size() - 1 ) * 4.0f / 5.0f + 1.0f / 5.0f;
			glColor4f( 0.9, 0.9, 0.9, use_gradient ? fraction : 1.0 );
			glVertex3f( path[ i ].x * pathLenMultiplier, 
						path[ i ].y * pathLenMultiplier,
						path[ i ].z * pathLenMultiplier );
		}
	glEnd();

	glLineWidth(oldLW[0]);

	delete[] oldLW;

	// reenable lighting
	glEnable( GL_LIGHTING );
}


//------------- redraw ---------------------------
void SweepSurface::redraw()
{
	
	if( geomChange ) {
		makeRibs();
		computePhongNormals();
		geomChange = false;
	}
	
  //  glPushMatrix();
  //  	glColor3f( colors[ 0 ]->r, colors[ 0 ]->g, colors[ 0 ]->b );
  //      glTranslatef( xLoc, yLoc, zLoc );
  //      glRotatef( angle, dxRot, dyRot, dzRot );
  //      glScalef( xSize, ySize, zSize );

  //      if( useMaterial ){
  //      	mat->enable();
  //      	mat->get();
  //      }

  //      if( useTexture ) tex->enable();

		//if (draw_wireframe) drawWireframe();
		//
		//if (draw_skin) drawSkin();

		//if( draw_normals ) drawNormals();

		//if( draw_path ) drawPath();

  //      if( useTexture ) tex->disable();

  //      if( useMaterial ) mat->disable();

  //  glPopMatrix();
}
