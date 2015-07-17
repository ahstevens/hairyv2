#ifndef BIMAP_H_
#define BIMAP_H_

#define FSIZE 500

class BiMap
{
public:
	BiMap(float xsz, float ysz); // defines the space to be sampled
	~BiMap();
	
	void newMap();
	void mkDataMap(float **t);
	void gabor(float **t, int cx, int cy, float r, float size, float contrast, float rat);
	void getVecValues(float x, float y, float &m1, float &m2, float &m3);
	void normalize();
	float getXSize();
	float getYSize();


private:
	float MRand();
	
	float **fieldX;
	float **fieldY;
	float **fieldZ;

	int ksize;
	int rows, cols;	

	float xspace, yspace;

	bool texture;

	float tx, ty;

	int xs, ys;
};

#endif /*BIMAP_H_*/
