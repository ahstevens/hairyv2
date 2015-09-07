#include "Texture.h"
#include <iostream>

#define RGB_SIZE 3

//------------------ Constructors ------------------------------------

/*
 *  Initialize texture id
 */
Texture::Texture()
{
   textureID = 0;  // avoid compiler warning
   width = height = 0;

   glGenTextures( 1, &textureID );
}

//-------------------- destructors ----------------------------------

/*
 *  Release texture id
 */

Texture::~Texture()
{
}

//---------------------- accessors ------------------------------------
GLuint  Texture::getId()     { return textureID; }
GLsizei Texture::getWidth()  { return width; }
GLsizei Texture::getHeight() { return height; }

//---------------------------------------------------------------------

/*
 *  Use the texture specified and read in the info
 */

//---------------------------------------------------------------------

/*
 *  Set GL_TEXTURE_MIN_FILTER
 */
void Texture::setMinFilter( GLint filter )
{
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glBindTexture(GL_TEXTURE_2D, 0);
}

/*
 *  Set GL_TEXTURE_MAG_FILTER
 */
void Texture::setMagFilter( GLint filter )
{
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glBindTexture(GL_TEXTURE_2D, 0);
}

/*
 *  Set GL_TEXTURE_WRAP_S
 */
void Texture::setWrapS( GLint wrapType )
{
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapType);
	glBindTexture(GL_TEXTURE_2D, 0);
}

/*
 *  Set GL_TEXTURE_WRAP_T
 */
void Texture::setWrapT( GLint wrapType )
{
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapType);
	glBindTexture(GL_TEXTURE_2D, 0);
}

//---------------------------------------------------------------------

/*
 *  Enable the texture to be mapped to vertices
 */
void Texture::enable()
{
   glBindTexture( GL_TEXTURE_2D, textureID );
}

/*
 * Stop mapping this texture to vertices
 */

void Texture::disable()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::checker(int w, int h)
{
	width = w;
	height = h;
	
	int i, j, c, base;

	GLubyte* checks = new GLubyte[width * height * RGB_SIZE];
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			base = (i * width + j) * RGB_SIZE;
			c = (((i & 0x1) == 0) ^ ((j % 0x2) == 0)) * 255;
			checks[base + 0] = (GLubyte)c;
			checks[base + 1] = (GLubyte)c;
			checks[base + 2] = (GLubyte)c;
		}
	}

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
		GL_RGB, GL_UNSIGNED_BYTE, checks);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	delete[] checks;
}

/*
 *  method to create a two-toned striped texture
 */
void Texture::stripes(int nStripes, GLubyte s1r, GLubyte s1g,
   GLubyte s1b, GLubyte s2r, GLubyte s2g, GLubyte s2b )
{
	width = height = nStripes;

	GLubyte* stripes = new GLubyte[width * height * RGB_SIZE];

    int base;
	for (int i = 0; i < height; ++i)
		for (int j = 0; j < width; ++j) {
			base = (i * width + j) * RGB_SIZE;
            if( i % 2 == 0 ) {           // EVEN STRIPES
				stripes[base + 0] = s1r; // r
				stripes[base + 1] = s1g; // g
				stripes[base + 2] = s1b; // b
            }
            else {                       // ODD STRIPES
				stripes[base + 0] = s2r; // r
				stripes[base + 1] = s2g; // g
				stripes[base + 2] = s2b; // b
            }
        }

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
		GL_RGB, GL_UNSIGNED_BYTE, stripes);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	delete[] stripes;
}

void Texture::stripes2D(int nStripes, GLubyte s1r, GLubyte s1g,
	GLubyte s1b, GLubyte s2r, GLubyte s2g, GLubyte s2b)
{
	width = nStripes;
	height = 1;

	GLubyte* stripes = new GLubyte[width * height * RGB_SIZE];

	int base;
	for (int i = 0; i < width; ++i)
	{
		base = i * RGB_SIZE;
		if (i % 2 == 0) {           // EVEN STRIPES
			stripes[base + 0] = s1r; // r
			stripes[base + 1] = s1g; // g
			stripes[base + 2] = s1b; // b
		}
		else {                       // ODD STRIPES
			stripes[base + 0] = s2r; // r
			stripes[base + 1] = s2g; // g
			stripes[base + 2] = s2b; // b
		}
	}

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
		GL_RGB, GL_UNSIGNED_BYTE, stripes);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	delete[] stripes;
}
