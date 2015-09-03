#include "biMap.h"

#define _USE_MATH_DEFINES
#include <math.h> // M_PI
#include <stdlib.h> // rand()

#define KEYSTEPS 10

BiMap::BiMap(float xsz, float ysz)
{
	int i;

	xspace = xsz; yspace = ysz;

	fieldX = new float*[FSIZE];
	fieldY = new float*[FSIZE];
	fieldZ = new float*[FSIZE];
	for (i=0;i<FSIZE;++i)
	{
		fieldX[i] = new float[FSIZE];
		fieldY[i] = new float[FSIZE];
		fieldZ[i] = new float[FSIZE];
	}

	newMap();
}

BiMap::~BiMap()
{
	if (fieldX) {
		for (int i = 0; i < FSIZE; ++i)
			delete[] fieldX[i];

		delete[] fieldX;
	}

	if (fieldY) {
		for (int i = 0; i < FSIZE; ++i)
			delete[] fieldY[i];

		delete[] fieldY;
	}

	if (fieldZ) {
		for (int i = 0; i < FSIZE; ++i)
			delete[] fieldZ[i];

		delete[] fieldZ;
	}
}

void BiMap::newMap()
{
	int i,j;
	for(i=0;i<FSIZE;++i)
		for(j=0;j<FSIZE;++j)
		{
			fieldX[i][j] = 0.0;
			fieldY[i][j] = 0.0;
			fieldZ[i][j] = 0.0;
		}
	mkDataMap(fieldX);
	mkDataMap(fieldY);
	mkDataMap(fieldZ);
}

void BiMap::mkDataMap(float **t)
{	
	int k,kx,ky;
	float sz,angle;
	float lx,ly;

	lx = 0.5*float(FSIZE);
	ly = 0.5*float(FSIZE);

   for (k = 0; k < 40; ++k)
   {
		kx =  int(lx + MRand()*lx);
		ky =  int(ly + MRand()*ly);
		sz = 100.0f + MRand()*30.0f;  // 80+/-20;

		angle = (MRand()+0.5f)*180.0f;
		//gabor(t, kx, ky, angle, sz, 0.1f*sz, 0.4f);
		gabor(t, kx, ky, angle, sz, 0.02f*sz, 0.4f);
   }  

   for (k = 0; k < 160; ++k)
   {
		kx =  int(lx + MRand()*lx);
		ky =  int(ly + MRand()*ly);
		sz = 50.0f + MRand()*25.0f;  // 45+/-10;

		angle = (MRand()+0.5f)*180.0f;
		gabor(t, kx, ky, angle, sz, 0.06f*sz, 0.4f);
   }
  


	int i,j;
	float min,max;
	max = -100.0; min = 100.0;
	for(i=FSIZE/4;i<FSIZE*3/4;i=i+5)
		for(j=FSIZE/4;j<FSIZE*3/4;j=j+5)
		{
			t[i][j] = t[i][j];
			if(t[i][j] > max) max = t[i][j];
			if(t[i][j] < min) min = t[i][j];
		}

//cerr << "M M " << min <<" "<< max << "\n";

	float range = max - min;
	float adj; 
	adj = min + range/2.0f;
  
	for(i=0;i<FSIZE;++i)
		for(j=0;j<FSIZE;++j)
		{
			t[i][j] = t[i][j]-adj;
		}	
}

void BiMap::gabor(float **t, int cx, int cy, float r, float size,float contrast,float rat)
{
    //const int TPATCH = 50;
    float freq, prod;
    float cosr,sinr;
    float halfx,halfy;
    float	x,y,gx,gy;
    int ix, iy, ccx, ccy;
	int TPATCH;

	TPATCH = static_cast<int>(size) * 2;

    ccx = cx - TPATCH/2;
    ccy = cy - TPATCH/2;
    r = r*(float)M_PI/180.0f;

    sinr = float(sin(r));
    cosr = float(-cos(r));

    halfx = float(TPATCH)/2.0f;
    halfy = float(TPATCH)/2.0f;

    freq = 2.0f*(float)M_PI/size;
    rat = 1.0f/(rat*2.0f*(float)M_PI);
    rat = -rat*rat;

    for (ix=0;ix<TPATCH;++ix)
	for(iy=0;iy<TPATCH;++iy)
	{
		x = (ix-halfx)*freq;
		y = (iy-halfy)*freq;
		/* rotate x and y */
		gx = cosr*x + sinr*y;
		gy = cosr*y  - sinr*x;
		prod = (cosr*x + sinr*y) ;

		t[ix + ccx][iy + ccy] += float(contrast*cos(prod)*exp((gx*gx +gy*gy)*rat));
	}
}

float BiMap::MRand()
{
    return float (rand()%1000)/1000.0f - 0.5f;
}

void BiMap::getVecValues(float x, float y, float &m1, float &m2, float &m3)
{
	// only the middle part of the data field is exposed
	int r,c;

	r = FSIZE*(0.5*x/xspace + 0.25);
	c = FSIZE*(0.5*y/xspace + 0.25);

	m1 = fieldX[r][c];
	m2 = fieldY[r][c];	
	m3 = fieldZ[r][c];		
}

void BiMap::getVecValues(float x, float y, float &m1, float &m2, float &m3, float &len)
{
	// only the middle part of the data field is exposed
	int r, c;

	r = FSIZE*(0.5*x / xspace + 0.25);
	c = FSIZE*(0.5*y / xspace + 0.25);

	m1 = fieldX[r][c];
	m2 = fieldY[r][c];
	m3 = fieldZ[r][c];

	len = sqrtf( m1 * m1 + m2 * m2 + m3 * m3 );
}

void BiMap::normalize()
{
	int i, j;

	float temp, max = 0;

	// get max mag
	for(i = 0; i < FSIZE; ++i)
		for(j = 0; j < FSIZE; ++j)
		{
			temp = sqrt( fieldX[i][j] * fieldX[i][j] + fieldY[i][j] * fieldY[i][j] + fieldZ[i][j] * fieldZ[i][j] );
			if( temp > max ) max = temp;
		}

	for(i = 0; i < FSIZE; ++i)
		for(j = 0; j < FSIZE; ++j)
		{
			fieldX[i][j] /= max;
			fieldY[i][j] /= max;
			fieldZ[i][j] /= max;
		}
}

float BiMap::getXSize()
{
	return xspace;
}

float BiMap::getYSize()
{
	return yspace;
}
