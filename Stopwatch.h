/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// Stopwatch.h
/** 
 *	Class for tracking time.  This class has the following responsibilities:
 *	*	Provide a way to start, read, and stop a time measurement.
 *
 *	Created by mdp 8/21/2000
 */
///////////////////////////////////////////////////////////////////////////////
class Stopwatch;

#ifndef STOPWATCH_H
#define STOPWATCH_H

#ifdef _WIN32
#include <sys/types.h>
#include <sys/timeb.h>
#define TIME_STRUCT struct timeb
#else
#include <sys/time.h>
#include <unistd.h>
#define TIME_STRUCT struct timeval
#endif

class Stopwatch
{
	public:
		enum State {NotStarted, Started, Stopped};
		Stopwatch();

		void start();
		double read();
		double sinceLastRead();
		double stop();
		void go();

	protected:
		TIME_STRUCT startTime, lastReadTime;
		State state;
};

#endif
