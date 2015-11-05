/*
 * Texture.h 
 */

#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>

#include <glm/glm.hpp>

class Texture
{
    public:
        Texture();
        ~Texture();

        GLuint  getId();
        GLsizei getWidth();
        GLsizei getHeight();

        void setMinFilter( GLint filter );
        void setMagFilter( GLint filter );
        void setWrapS( GLint wrapType );
        void setWrapT( GLint wrapType );

        void enable();
        void disable();

		void checker(int w, int h);
		void stripes(int nStripes, GLubyte s1r,
			GLubyte s1g, GLubyte s1b, GLubyte s2r, GLubyte s2g, GLubyte s2b);
		void stripes1D(int nStripes, glm::vec3 stripe_color1, glm::vec3 stripe_color2);
		void grid( glm::vec3 line_color );

    protected:
        GLuint  textureID;
        GLsizei width, height;
};

#endif
