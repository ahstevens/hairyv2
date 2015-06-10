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
	polygon.push_back( glm::vec4( -0.5, -0.5, 0.0, 1.0 ) );
	polygon.push_back( glm::vec4( 0.5, -0.5, 0.0, 1.0 ) );
	polygon.push_back( glm::vec4( 0.5, 0.5, 0.0, 1.0 ) );
	polygon.push_back( glm::vec4( -0.5, 0.5, 0.0, 1.0 ) );

	// default path is straight along z axis
	path.push_back( glm::vec4( 0.0, 0.0, 0.0, 1.0 ) );
	path.push_back( glm::vec4( 0.0, 0.0, -0.2, 1.0 ) );
	path.push_back( glm::vec4( 0.0, 0.0, -0.4, 1.0 ) );
	path.push_back( glm::vec4( 0.0, 0.0, -0.6, 1.0 ) );
	path.push_back( glm::vec4( 0.0, 0.0, -0.8, 1.0 ) );
	path.push_back( glm::vec4( 0.0, 0.0, -1.0, 1.0 ) );
	path.push_back( glm::vec4( 0.0, 0.0, -1.2, 1.0 ) );
	path.push_back( glm::vec4( 0.0, 0.0, -1.4, 1.0 ) );
	path.push_back( glm::vec4( 0.0, 0.0, -1.6, 1.0 ) );
	path.push_back( glm::vec4( 0.0, 0.0, -1.8, 1.0 ) );
	path.push_back( glm::vec4( 0.0, 0.0, -2.0, 1.0 ) );

	// default scale is linear downscale to 0
	scales.push_back( glm::vec2( 1.0, 1.0 ) );
	scales.push_back( glm::vec2( 0.9, 0.9 ) );
	scales.push_back( glm::vec2( 0.8, 0.8 ) );
	scales.push_back( glm::vec2( 0.7, 0.7 ) );
	scales.push_back( glm::vec2( 0.6, 0.6 ) );
	scales.push_back( glm::vec2( 0.5, 0.5 ) );
	scales.push_back( glm::vec2( 0.4, 0.4 ) );
	scales.push_back( glm::vec2( 0.3, 0.3 ) );
	scales.push_back( glm::vec2( 0.2, 0.2 ) );
	scales.push_back( glm::vec2( 0.1, 0.1 ) );
	scales.push_back( glm::vec2( 0.0, 0.0 ) );

	// twist 180 degrees in the CCW direction
	rotations.push_back( 0.0 );
	rotations.push_back( -18 );
	rotations.push_back( -36 );
	rotations.push_back( -54 );
	rotations.push_back( -72 );
	rotations.push_back( -90 );
	rotations.push_back( -108 );
	rotations.push_back( -126 );
	rotations.push_back( -144 );
	rotations.push_back( -162 );
	rotations.push_back( -180 );
	rotations.push_back( -180 );

	// make the points representing the ribs of the swept surface
	makeRibs();
	computePhongNormals();

	wireframe = 0;
	draw_path = 0;
	draw_normals = 0;
}

SweepSurface::SweepSurface( std::vector<glm::vec2> poly,
						    std::vector<glm::vec3> path,
						    std::vector<glm::vec2> scales,
						    std::vector<float> rots )
{
	// convert 2D polygon points to vec4
	std::vector<glm::vec2>::iterator it;
	for( it = poly.begin(); it != poly.end(); it++ )
		polygon.push_back( glm::vec4( *it, 0.0, 1.0 ) );

	// convert 3D path points to vec4
	std::vector<glm::vec3>::iterator it2;
	for( it2 = path.begin(); it2 != path.end(); it2++ )
			this->path.push_back( glm::vec4( *it2, 1.0 ) );

	this->scales = scales;
	rotations = rots;

	// make the points representing the ribs of the swept surface
	makeRibs();
	computePhongNormals();

	wireframe = 0;
	draw_path = 0;
	draw_normals = 0;
}

//------------- destructor -----------------------
SweepSurface::~SweepSurface()
{
}

// setter class for sweep surface 2D polygon
void SweepSurface::updatePolygon( std::vector<glm::vec2> poly )
{
	// start fresh
	polygon.clear();

	// convert 2D polygon points to vec4
	std::vector<glm::vec2>::iterator it;
	for( it = poly.begin(); it != poly.end(); it++ )
		polygon.push_back( glm::vec4( *it, 0.0, 1.0 ) );

	makeRibs();
	computePhongNormals();
}

// setter class for sweep surface path
void SweepSurface::updatePath( std::vector<glm::vec3> p )
{
	// start fresh
	path.clear();

	// convert 3D path points to vec4
	std::vector<glm::vec3>::iterator it2;
	for( it2 = p.begin(); it2 != p.end(); it2++ )
		path.push_back( glm::vec4( *it2, 1.0 ) );

	makeRibs();
	computePhongNormals();
}

// setter class for sweep surface 2D polygon scales
void SweepSurface::updateScales( std::vector<glm::vec2> s )
{
	scales = s;

	makeRibs();
	computePhongNormals();
}

// setter class for sweep surface 2D polygon rotations (in degrees)
void SweepSurface::updateRotations( std::vector<float> rots )
{
	rotations = rots;

	makeRibs();
	computePhongNormals();
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
	point_buffer.clear();

	// for all points along the defined path
	for( int i = 0; i < path.size(); i++ )
	{
		glm::vec3 w;

		if( i == 0 )
			w = glm::vec3( 0.0, 0.0, 1.0 ); // normal z axis to start off
		else {
			// current path point minus prev path point gives vector pointing
			// towards prev polygon's center.
			w = glm::normalize( glm::vec3( path[ i - 1 ].x - path[ i ].x,
								 path[ i - 1 ].y - path[ i ].y,
								 path[ i - 1 ].z - path[ i ].z ) );
		}

		// calculate up vector for angle offset
		glm::vec3 up = glm::vec3( -sin( glm::radians( rotations[ i ] ) ),
						 cos( glm::radians( rotations[ i ] ) ),
						 0.0 );

		// u is orthogonal to up vector and w vector
		glm::vec3 u = normalize( cross( up, w ) );

		// v vector orthogonal to w vector and u vector
		glm::vec3 v = normalize( cross( w, u ) );

		// scale the polygon
		u = scales[ i ].x * u;
		v = scales[ i ].y * v;

		// build coordinate frame transformation matrix at path point
		glm::mat4 coordFrameTrans = glm::transpose( glm::mat4( glm::vec4( u, 0.0 ),
												glm::vec4( v, 0.0 ),
												glm::vec4( w, 0.0 ),
												path[ i ] ) );

		// push back transformed polygon points as a new "rib" of the surface
		std::vector<glm::vec4>::iterator it;
		for( it = polygon.begin(); it != polygon.end(); it++ )
		{
			point_buffer.push_back( coordFrameTrans * ( *it ) );
		}
	}
}

/*
 * computePhongNormals() calculates per-vertex normals, weighting each triangle
 * normal's contribution to the vertex normal by its angle at the vertex
 */
void SweepSurface::computePhongNormals()
{
	normal_buffer.clear();

	int polySize = polygon.size();
	int pntBuffSize = point_buffer.size();
	float dotProd, cosTheta, theta;
	glm::vec3 normal;
	// vectors from current vertex to next vertex, previous vertex,
	// adjacent vertex in the next or previous polygon, the adjacent
	// previous vertex, and the adjacent next vertex
	glm::vec4 vertCur, vecNext, vecPrev, vecAdj, vecAdjNext, vecAdjPrev;

	for( int i = 0; i < pntBuffSize; i++ ) {
		normal.x = normal.y = normal.z = 0.0;
		vertCur = point_buffer[ i ];

		// calculate normals for faces on previous path side if not at begin
		if( i >= polySize ) {
			vecAdj = point_buffer[ i - polySize ] - vertCur;
			// check if i is first point in polygon to get proper prev point
			if( ( i + 1 ) % polySize == 1 ) {
				vecPrev = point_buffer[ i + polySize - 1 ] - vertCur;
			}
			else {
				vecPrev = point_buffer[ i - 1 ] - vertCur;
			}
			// check if i is last point in polygon to get proper next point
			if( (i + 1) % polySize == 0 ) {
				vecNext = point_buffer[ i - polySize + 1 ] - vertCur;
				vecAdjNext = point_buffer[ i - 2 * polySize + 1 ] - vertCur;
			}
			else {
				vecNext = point_buffer[ i + 1 ] - vertCur;
				vecAdjNext = point_buffer[ i - polySize + 1 ] - vertCur;
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
					normal += theta * glm::cross( glm::vec3( vecAdj ), glm::vec3( vecPrev ) );
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
					normal += theta * glm::cross( glm::vec3( vecAdjNext ), glm::vec3( vecAdj ) );
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
					normal += theta * glm::cross( glm::vec3( vecNext ), glm::vec3( vecAdjNext ) );
				}
			}
		}

		// calculate normals for faces on the next path side if not at end
		if( i < pntBuffSize - polySize ) {
			vecAdj = point_buffer[ i + polySize ] - vertCur;

			// check if i is last point in polygon to get proper next point
			if( (i + 1) % polySize == 0 )
				vecNext = point_buffer[ i - polySize + 1 ] - vertCur;
			else
				vecNext = point_buffer[ i + 1 ] - vertCur;

			// check if i is first point in polygon to get proper previous point
			if( ( i + 1 ) % polySize == 1 ) {
				vecPrev = point_buffer[ i + polySize - 1 ] - vertCur;
				vecAdjPrev = point_buffer[ i + 2 * polySize - 1 ] - vertCur;
			}
			else {
				vecPrev = point_buffer[ i - 1 ] - vertCur;
				vecAdjPrev = point_buffer[ i + polySize - 1 ] - vertCur;
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
					normal += theta * glm::cross( glm::vec3( vecAdj ), glm::vec3( vecNext ) );
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
					normal += theta * glm::cross( glm::vec3( vecAdjPrev ), glm::vec3( vecAdj ) );
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
					normal += theta * glm::cross( glm::vec3( vecPrev ), glm::vec3( vecAdjPrev ) );
				}
			}
		}
		// add normalized normal to the vector
		normal_buffer.push_back( glm::vec4( glm::normalize( normal ), 0.0f ) );
	}
}

/*
 * enable wireframe mode
 */
void SweepSurface::enableWireframe() { wireframe = 1; }

/*
 * disable wireframe mode
 */
void SweepSurface::disableWireframe() { wireframe = 0; }

/*
 * enable drawing the Phong normals at the vertices
 */
void SweepSurface::enableNormals() { draw_normals = 1; }

/*
 * disable drawing the Phong normals at the vertices
 */
void SweepSurface::disableNormals() { draw_normals = 0; }

/*
 * enable drawing the path along which the surface sweeps
 */
void SweepSurface::enablePath() { draw_path = 1; }

/*
 * disable drawing the path along which the surface sweeps
 */
void SweepSurface::disablePath() { draw_path = 0; }

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
	if( useTexture ) tex->disable();
	if( useMaterial ) mat->disable();

	glColor3f( 1.0, 1.0, 1.0 ); // white wireframe

	// draw polygon ribs using line loop
	for( int i = 0; i < pathSize; i++ ) {
		glBegin( GL_LINE_LOOP );
			for( int j = polySize * i; j < polySize * ( i + 1 ); j++)
				glVertex3f( point_buffer[ j ].x,
						    point_buffer[ j ].y,
							point_buffer[ j ].z );
		glEnd();
	}

	// draw wireframe for triangle skin mesh
	for( int i = 0; i < pathSize - 1; i++ ) {
		glBegin( GL_LINE_STRIP );
			for( int j = polySize * i; j < polySize * ( i + 1 ); j++ ) {
				glVertex3f( point_buffer[ j ].x,
						    point_buffer[ j ].y,
							point_buffer[ j ].z );
				glVertex3f( point_buffer[ j + polySize ].x,
							point_buffer[ j + polySize ].y,
							point_buffer[ j + polySize ].z );
			}
			// connect back to beginning points
			glVertex3f( point_buffer[ polySize * i ].x,
						point_buffer[ polySize * i ].y,
						point_buffer[ polySize * i ].z );
			glVertex3f( point_buffer[ polySize * ( i + 1) ].x,
						point_buffer[ polySize * ( i + 1) ].y,
						point_buffer[ polySize * ( i + 1) ].z );
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
	float u, v = 0.0;

	// draw skin mesh using triangle strips
	for( int i = 0; i < pathSize - 1; i++ ) {
		glBegin( GL_TRIANGLE_STRIP );
			for( int j = polySize * i; j < polySize * ( i + 1 ); j++ ) {
				u = (float) ( j % polySize ) / polySize;
				v = (float) i / ( pathSize - 1 );
				glTexCoord2f( u, v );
				glNormal3f( normal_buffer[ j ].x,
						    normal_buffer[ j ].y,
							normal_buffer[ j ].z );
				glVertex3f( point_buffer[ j ].x,
						    point_buffer[ j ].y,
							point_buffer[ j ].z );
				v = (float) ( i + 1 ) / ( pathSize - 1 );
				glTexCoord2f( u, v );
				glNormal3f( normal_buffer[ j + polySize ].x,
							normal_buffer[ j + polySize ].y,
							normal_buffer[ j + polySize ].z );
				glVertex3f( point_buffer[ j + polySize ].x,
							point_buffer[ j + polySize ].y,
							point_buffer[ j + polySize ].z );
			}
			// connect back to beginning polygon points to complete skin section
			u = 1.0;
			v = (float) i / ( pathSize - 1 );
			glTexCoord2f( u, v );
			glNormal3f( normal_buffer[ polySize * i ].x,
						normal_buffer[ polySize * i ].y,
						normal_buffer[ polySize * i ].z );
			glVertex3f( point_buffer[ polySize * i ].x,
						point_buffer[ polySize * i ].y,
						point_buffer[ polySize * i ].z );
			v = (float) ( i + 1 ) / ( pathSize - 1 );
			glTexCoord2f( u, v );
			glNormal3f( normal_buffer[ polySize * ( i + 1) ].x,
						normal_buffer[ polySize * ( i + 1) ].y,
						normal_buffer[ polySize * ( i + 1) ].z );
			glVertex3f( point_buffer[ polySize * ( i + 1) ].x,
						point_buffer[ polySize * ( i + 1) ].y,
						point_buffer[ polySize * ( i + 1) ].z );
		glEnd();
	}
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
	if( useTexture ) tex->disable();
	if( useMaterial ) mat->disable();

	glColor3f( 1.0, 1.0, 0.0 ); // yellow normals

	for( int i = 0; i < pathSize - 1; i++ ) {
		glBegin( GL_LINES );
		for( int j = polySize * i; j < polySize * ( i + 1 ); j++ ) {
			glVertex3f( point_buffer[ j ].x,
						point_buffer[ j ].y,
						point_buffer[ j ].z );
			glVertex3f( (point_buffer[ j ] + normal_buffer[ j ]).x,
						(point_buffer[ j ] + normal_buffer[ j ]).y,
						(point_buffer[ j ] + normal_buffer[ j ]).z );
			glVertex3f( point_buffer[ j + polySize ].x,
						point_buffer[ j + polySize ].y,
						point_buffer[ j + polySize ].z );
			glVertex3f( (point_buffer[ j + polySize ]\
							+ normal_buffer[ j + polySize ]).x,
						(point_buffer[ j + polySize ]\
							+ normal_buffer[ j + polySize ]).y,
						(point_buffer[ j + polySize ]\
							+ normal_buffer[ j + polySize ]).z );
		}
		// connect back to beginning point_buffer
		glVertex3f( point_buffer[ polySize * i ].x,
					point_buffer[ polySize * i ].y,
					point_buffer[ polySize * i ].z );
		glVertex3f( (point_buffer[ polySize * i ]\
						+ normal_buffer[ polySize * i ]).x,
					(point_buffer[ polySize * i ]\
						+ normal_buffer[ polySize * i ]).y,
					(point_buffer[ polySize * i ]\
						+ normal_buffer[ polySize * i ]).z );
		glVertex3f( point_buffer[ polySize * ( i + 1) ].x,
					point_buffer[ polySize * ( i + 1) ].y,
					point_buffer[ polySize * ( i + 1) ].z );
		glVertex3f( (point_buffer[ polySize * ( i + 1) ]\
						+ normal_buffer[ polySize * ( i + 1) ]).x,
					(point_buffer[ polySize * ( i + 1) ]\
						+ normal_buffer[ polySize * ( i + 1) ]).y,
					(point_buffer[ polySize * ( i + 1) ]\
						+ normal_buffer[ polySize * ( i + 1) ]).z );
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
	if( useTexture ) tex->disable();
	if( useMaterial ) mat->disable();

	glColor3f( 1.0, 0.0, 1.0 ); // path is purple

	glBegin( GL_LINE_STRIP );
		std::vector<vec4>::iterator it2;
		for( it2 = path.begin(); it2 != path.end(); it2++ )
			glVertex3f( it2->x, it2->y, it2->z );
	glEnd();

	// reenable lighting
	glEnable( GL_LIGHTING );
}


//------------- redraw ---------------------------
void SweepSurface::redraw()
{
    glPushMatrix();
    	glColor3f( colors[ 0 ]->r, colors[ 0 ]->g, colors[ 0 ]->b );
        glTranslatef( xLoc, yLoc, zLoc );
        glRotatef( angle, dxRot, dyRot, dzRot );
        glScalef( xSize, ySize, zSize );

        if( useMaterial ){
        	mat->enable();
        	mat->get();
        }

        if( useTexture ) tex->enable();

		if( wireframe ) {
			drawWireframe();
		} else {
			drawSkin();
		}

		if( draw_normals ) drawNormals();

		if( draw_path ) drawPath();

        if( useTexture ) tex->disable();

        if( useMaterial ) mat->disable();

    glPopMatrix();
}
