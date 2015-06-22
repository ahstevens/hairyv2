#include "GL/glut.h"
#include <iostream>
#include <stdio.h>
#include <math.h>
#include "setAngle.h"
#include "CircleCylinder.h"
#include "scalarPlane.h"
#include "slider.h"
#include "Isotrack.h"
#include "Matrix.h"

#define NTRIALS 2 //20
#define NCONDS 5

int *trialvec, trialCounter;
int cond, totalTrials;
bool settingParms;  // the param settin pretrials
bool showProbe;
bool running;
bool setSpd;
FILE *fp;

FILE *fpe;

#define PERIOD 120

using namespace std;

// this is a simple example of a 3D interactive program

float rx,ry, rw, rh,mrx;
float angleError;

float tx,ty,tz,tspd; // target values

bool rotate;
bool rocking;

Isotrack iso;
bool hasIso;

float rAngle[PERIOD];
int rCtr;

float cy;
float size;
float density;

float winWid, winHeight;

float azim, elev;

Circ *C;

Slider *sizeControl;
Slider *densityControl;
scalarPlane *speedBG ;
float sizeScale; // a random scale factor
float densityScale;

float outVec[3], speedEst;

float light_position[] = {-10.0,20.0,20.0,1.0};
int tp;
float startMx, dMx,pX; // mouse move
float startMy, dMy,pY; // mouse move

char dataFile[20];
char name[10];
int block;


void setTrial()
{
	
	if(trialCounter == totalTrials)
	{
		fclose(fp);
		exit(1);
	}	
	cond = trialvec[trialCounter];
	sizeScale = 1.0+(rand()%100)/200.0;
	densityScale = 1.0/(1.0+(rand()%100)/200.0);
	//C->setSize(1.1);
	C->setSize(0.1);

	switch(cond)
	{
		case 0:
			tp = TUBE;
			C->setSize(0.9);
			break;
		case 1:
			tp = DISC;
			break;
		case 2:
			tp = CONE;
			break;
		case 3:
			tp = CONES;
			break;
		case 4:
			tp = LINE;
			break;
	}
	++trialCounter;

	if(trialCounter%5 == 0)
	{
		C->newMap();
		C->makeVecCurtain(density);
	}

	//cerr << "COUNT " << trialCounter << "\n";
	//cerr << "Type " << tp << "\n";

}


void initTrialsSettings()
{
	int i,j,k,tmp;
	totalTrials = NTRIALS*NCONDS;

	trialvec = new int[totalTrials+4];
	for (i=0;i<NTRIALS;++i)
		for(j=0;j<NCONDS;++j)
		{
			k = i*NCONDS + j;
			trialvec[k] =  j;
		}
	for(i=0;i<totalTrials;++i)
	{
		j = rand()%totalTrials;
		tmp = trialvec[i];
		trialvec[i] = trialvec[j];
		trialvec[j] = tmp;
	}
	for(i=0;i<totalTrials;++i)
	cerr << "Trial " << trialvec[i] << "\n";
	fp = fopen("settings.txt","w");
	settingParms = true;
	setTrial();
}

void initTrialsExpt()
{
	int i,j,k,tmp;

	int condvec[20];
	totalTrials = NTRIALS*NCONDS;

	trialvec = new int[totalTrials+4];
	
	for(j=0;j<NCONDS;++j)
			condvec[j] =  j;

	for(i=0;i<NCONDS;++i)
	{
		j = rand()%NCONDS;
		tmp = condvec[i];
		condvec[i] = condvec[j];
		condvec[j] = tmp;
	}

	for(i=0;i<NCONDS;++i)
	{
		for(j=0;j<NTRIALS;++j)
		{
			k = i*NTRIALS + j;
			trialvec[k] = condvec[i];
		}
	}
	for(i=0;i<totalTrials;++i)
	cerr << "Trial " << trialvec[i] << "\n";
	fp = fopen(dataFile,"w");
	running = true;
	C->isx = rand()%40 + 5;
	C->isy = rand()%15 + 5;		
	C->getAngle(tx,ty,tz,tspd);

	setTrial();
}


void redraw( void )
{
	float px, py;
	float angle, ax,ay,az,len;
	float inVec[3];

	inVec[0]=1.0; inVec[1] = 0.0;
	inVec[2] = 0.0;

	rand();
	VLMatrix M3;
	
	if(hasIso)
	{	
	  Isotrack::State s = iso.GetState(1);
        //std::cout << "position: " << s.position[0] << "," << s.position[1] << "," << s.position[2] << "\tquaternion: " << s.orientation[0] << "," << s.orientation[1] << "," << s.orientation[2] << "," << s.orientation[3] << std::endl;
		angle = acos(s.orientation[0])*2.0*180.0/3.14159;
		ax = s.orientation[1];
		ay = s.orientation[2];
		az = s.orientation[3];
		len = sqrt(ax*ax + ay*ay + az*az);
		ax /=len; ay /= len; az /= len;
	}

	M3.LoadGlRotation(angle,ax,az,ay);
	M3.VecMultMat(inVec,outVec);
	//cerr << "Vex " << outVec[0] <<  " " << outVec[1] << " " << outVec[2] << "\n";

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// here is the floor

	px = rx/5.0;
	py = 90.0+ry/5.0;

	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glPushMatrix();
		if(rocking) glRotatef(rAngle[rCtr]+mrx,0.0,1.0,0.0);

glRotatef(15.0,1.0,0.0,0.0);
		glPushMatrix();
		glScalef(1.0,1.0,1.0);

			glTranslatef(10.0,0.0,0.0);
			glRotatef(angle,ax,ay,az);
			glRotatef(-90.0,0.0,1.0,0.0);
			//glScalef(5.0,0.5,0.5);
			if(showProbe) C->CylinderArrow(1.0,8.0);
		glPopMatrix();

		rCtr = (rCtr+1)%PERIOD;

		glTranslatef(-14.0,-6.0,0.0);
		//glRotatef(15.0,0.0,1.0,0.0);

		C->DrawCurtain(tp);
			
		speedBG->draw(C->mag,C->xTop,C->yTop,C->rows, C->cols,C->gap);
		glPopMatrix();
		
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
		glLoadIdentity();
		glOrtho(0.0,winWid,0.0,winHeight,-50.0,50.0);
		glColor3f(1.0,1.0,0.0);
		if(!running) sizeControl->draw();
		if (!running) densityControl->draw();
		//glRectf(0.0,0.0,100.0,200.0);
		speedBG->Key(10.0,50.0,setSpd) ; 
		
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glutSwapBuffers();

}

void mousebutton(int button, int state, int x, int y)
{
	float vx;
	ry = winHeight - y;
	
	rx = x; 

	//cerr << "MXY " << rx << " " << ry << "     ";
	if(ry < 85 && ry > 50)
	{
	
		vx = (rx-10.0)/300.0;
		speedEst = vx;
		setSpd = true;
		//cerr << ry << " " << vx << "\n";
	}

	//cerr << "Vec " << outVec[0] <<  " " << outVec[1] << " " << outVec[2] << "\n\n";
	
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{	
			dMx = 0.0;
			dMy = 0.0;
			startMx = rx;
			startMy = ry;
			sizeControl->update(rx,ry,ACTIVE);
			densityControl->update(rx,ry,ACTIVE);
		
	}
	if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
	{	
		mrx = rx;
	}

	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
			pX = pX + dMx*0.2; dMx = 0.0;
			pY = pY + dMy*0.2; dMy = 0.0;

			if(sizeControl->isActive())
			{
	
				size = sizeScale*sizeControl->getValue()*2.0;	
				C->setSize(size);
				//cerr << "SIZE DENSITY " << size << " " << density <<"\n";
				sizeControl->update(rx,ry,INACTIVE);
			}
				
		
			if(densityControl->isActive())
			{
				density = densityScale*densityControl->getValue();
				//cerr << "SIZE DENSITY " << size << " " << density <<"\n";
				C->makeVecCurtain(density);
			}
			densityControl->update(rx,ry,INACTIVE);

	}
	//light_position[0] = rx/20.0;
	//glLightfv(GL_LIGHT0,GL_POSITION, light_position);
}

void motion(int x, int y)
// called when a mouse is in motion with a button down
{
	rx = x; ry = winHeight - y;
	sizeControl->update(rx,ry,MOVE_IF_ACTIVE);
	densityControl->update(rx,ry,MOVE_IF_ACTIVE);
	//glLightfv(GL_LIGHT0,GL_POSITION, light_position);
}

static void
Menu(int value)
{
  /* Menu items have key values assigned to them.  Just pass
     this value to the key routine. */


	//cerr << "MENU " << value << "\n";
	switch(value)
	{
		case 1:
			tp = TUBE;
			break;
		case 2:
			tp = DISC;
			break;
		case 3:
			tp = CONE;
			break;
		case 4:
			tp = CONES;
			break;
		case 5:
			tp = LINE;
			break;
		case 6:
			rocking = !rocking;
			break;
		case 7: settingParms = true;
			C->newMap();
			C->makeVecCurtain(density);
			C->cursor = false;
			initTrialsSettings();
			showProbe = false;
			break;
		case 8: running = true;
			C->newMap();
			C->makeVecCurtain(density);
			initTrialsExpt();
			showProbe = false;
			break;
		case 99:
			exit(1);
	}
 
}

void keyboard(unsigned char key, int x, int y) // x and y give the mouse pos
{
	int ix,iy;
	float  dot;
	//cerr << "Key " << key << " int " << int(key) << "\n";
	//cerr << char(7);
	
	if (key == 'f')
	{
		C->newMap();
		C->makeVecCurtain(density);
	}

	if (key == ' ') 
	{	
		dot = outVec[0]*tx + outVec[2]*ty - outVec[1]*tz;
			
		angleError = acos(dot)*180.0/3.14159;
		cerr << "ERROR " <<acos(dot)*180.0/3.14159 << "\n\n";
		if(angleError > 25)cerr << char(7);
		if(angleError > 40) cerr << char(7);

		if(settingParms)
		{
			fprintf(fp,"%d \t%f \t%f \n", cond, size, density);
			cond = trialvec[trialCounter];	
			//cerr << "Next " << cond << " " << trialCounter << "\n";
			densityControl->setRandomValue();
			sizeControl->setRandomValue();
			size = sizeScale*sizeControl->getValue()*2.0;	
			C->setSize(size);
			density = densityScale*densityControl->getValue();
				//cerr << "SIZE DENSITY " << size << " " << density <<"\n";
			C->makeVecCurtain(density);
			setTrial();
		}
		if (running && setSpd)
		{	

			if(rocking) fprintf(fp,"%s \tR \t%d ",name,block);
			else fprintf(fp,"%s \tS \t%d ",name,block);

			fprintf(fp,"\t%3.3f \t%3.3f \t%3.3f \t%3.3f \t%3.3f \t%3.3f ",tx,ty,tz,outVec[0], outVec[2], outVec[1]);
			fprintf(fp,"\t%d \t%3.3f \t%3.3f \t%3.3f\n",cond, angleError, speedEst, tspd);
			cond = trialvec[trialCounter];	
			//cerr << "Next " << cond << " " << trialCounter << "\n";
			setTrial();
			setSpd = false;	
			C->isx = rand()%40 + 5;
			C->isy = rand()%15 + 5;		
			C->getAngle(tx,ty,tz,tspd);
			
		}
		else
		if(!running)
		{
			C->isx = rand()%40 + 5;
			C->isy = rand()%15 + 5;		
			C->getAngle(tx,ty,tz,tspd);
		}
	}

	//if (key == 'r') rotate = !rotate;
	if (key == 's') C->speedSize = !C->speedSize;

	if(key == 'm') C->getADCP();

	if (key == 'p') showProbe = !showProbe;
/*
	if (key == 'n')
	{
		cerr << "Vec " << outVec[0] <<  " " << outVec[2] << " " << -outVec[1] << "\n";
	 dot = outVec[0]*tx + outVec[2]*ty - outVec[1]*tz;
	cerr << "DOT " << dot << " " <<acos(dot)*180.0/3.14159 << "\n\n";

		//cerr << "Setting " << azim << " " << -elev << "\n";	
		//cerr << "Actual  " << az   << "  " << pt   << "\n";

		//cerr << "Azim Err " << fabs(azim-az) << "\n";
		//cerr << "Elev Err " << fabs(elev+pt) << "\n";
		C->isx = rand()%40 + 5;
		C->isy = rand()%15 + 5;		
		C->getAngle(tx,ty,tz,tspd);

	}
*/
}

void reshape(int w, int h)
{
	winWid = w, winHeight = h;
	glViewport(0.0,0.0,winWid,winHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-10.0,10.0,-7.0,7.0, 50.0, 2000.0);	
	glTranslatef(0.0,0.0,-65.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int main(int argc, char *argv[])
{	int i;



cerr << "Enter subject Name:  ";
	cin >> name;
	cerr << "Enter block # :  ";
	cin >> block;
	sprintf(dataFile,"%s_%d.txt",name,block);
	cerr << dataFile << "\n";

	tp = TUBE;
	rotate = false; // mouse rotate
	rocking = false;
	//hasIso = true;
	hasIso = false;
	running = false;
	setSpd = false;

	showProbe = true;
	trialCounter = 0;
	totalTrials = 1000;

	sizeScale = densityScale = 1.0;

  if(hasIso)
  {
	if(iso.Initialize("COM1"))
	{
		cerr << "Isotrack Started \n";
		hasIso = true;
	}
	else	
		cerr << "Isotrack failed\n";

  }

	density = 1.0;

	sizeControl = new Slider(300.0,"Size",3,0.5,10.0,10.0);
	densityControl = new Slider(300.0,"Density",3,0.5,10.0,25.0);

//	vAngle = new setAngle(10.0,90.0);
//	hAngle = new setAngle(10.0,180.0);

	for (i=0;i<PERIOD;++i) rAngle[i] = 20.0*sin(2.0*i*3.14159/PERIOD);
	rCtr = 0;
	mrx = 0; // mouse rotate

	rx = 100; ry = 200; rw = 30; rh = 15;
	cy = 0.0;
	azim = elev = 0.0;
	pX = pY=0.0;


	winWid = 1200; winHeight = 750;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Mouse example");
	glutPositionWindow(200,100);
	glutReshapeWindow(winWid,winHeight);

	C = new Circ(4.0,12);
	C->speedSize = false;
	speedBG = new scalarPlane(MAXROWS,MAXCOLS);

	glClearColor(0.4,0.4,0.4,1.0);

	glLightfv(GL_LIGHT0,GL_POSITION, light_position);
	float specReflection[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specReflection);
	glMateriali(GL_FRONT, GL_SHININESS, 96);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-16.0,16.0,-10.0,10.0, 20.0, 2000.0);
	glTranslatef(0.0,0.0,-15.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);

	glutDisplayFunc(redraw);
	glutReshapeFunc(reshape);
	glutIdleFunc(redraw);
	glutMouseFunc( mousebutton);
	glutKeyboardFunc(keyboard);
	glutMotionFunc( motion);

	glutCreateMenu(Menu);
  glutAddMenuEntry("3D arrow", 1);
  glutAddMenuEntry("Disc", 2);
  glutAddMenuEntry("Cone", 3);
   glutAddMenuEntry("Cones", 4);
  glutAddMenuEntry("Lines", 5);
    glutAddMenuEntry("Rocking", 6);
	glutAddMenuEntry("Settings", 7);
	glutAddMenuEntry("Start Expt", 8);
  glutAddMenuEntry("Quit", 99);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutMainLoop();

	return 0;
}