/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// Stopwatch.cpp
// 
//	Class for tracking time.
//
// Created by mdp 8/21/2000
//
///////////////////////////////////////////////////////////////////////////////

#include "stopwatch.h"

#ifdef _WIN32
#define DIVISIONS_PER_SECOND 1000
#else
#define ftime(arg) gettimeofday(arg, NULL);
#define time tv_sec
#define millitm tv_usec
#define DIVISIONS_PER_SECOND 1000000
#endif

#define DIFFERENCE_BETWEEN_TIMES(time1, time2) \
	((double)(time2.time - time1.time) + \
	(time2.millitm - time1.millitm) / (double)DIVISIONS_PER_SECOND)

#define SET_TIME(dest_time, src_time) \
	dest_time.time = src_time.time; \
	dest_time.millitm = src_time.millitm;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *  Constructor.
 */
Stopwatch::Stopwatch()
{
	state = NotStarted;
}

/*----------------------------Stopwatch Interface----------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *	This procedure starts the stopwatch, setting the lastReadTime to
 *	the same value as startTime.
 */
void Stopwatch::start()
{
	ftime(&startTime);
	SET_TIME(lastReadTime, startTime);
	state = Started;
}

/*---------------------------------------------------------------------------*/
/**
 *	This procedure reads the stopwatch, returning the number of seconds since
 *	the stopwatch was started or the time elapsed when it was stopped.
 *	Returns -1.0 if the	stopwatch was never started.
 */
double Stopwatch::read()
{
	if (state == NotStarted)
		return -1.0;

	if (state == Started)
		ftime(&lastReadTime);

	return DIFFERENCE_BETWEEN_TIMES(startTime, lastReadTime);
}

/*---------------------------------------------------------------------------*/
/**
 *	This procedure returns the time passed since the last time read() or
 *	sinceLastRead() was called if the stopwatch is going.  If it is stopped,
 *	sinceLastRead returns the same thing as read.  Returns -1.0 if the stopwatch
 *	was never started.
 */
double Stopwatch::sinceLastRead()
{
	TIME_STRUCT currentTime;
	double retval;

	if (state == NotStarted)
		return -1.0;

	if (state == Stopped)
		return read();

	ftime(&currentTime);

    retval = DIFFERENCE_BETWEEN_TIMES(lastReadTime, currentTime);
	SET_TIME(lastReadTime, currentTime);

	return retval;
}

/*---------------------------------------------------------------------------*/
/**
 *	This procedure stops the stopwatch, recording the time of stopping for
 *	subsequent read and sinceLastRead operations.
 */
double Stopwatch::stop()
{
	double retval = read();

	state = Stopped;

	return retval;
}

/*---------------------------------------------------------------------------*/
/**
 *	This procedure allows the stopwatch to continue after it has been stopped.
 *	I would have used the word continue if it weren't a reserved word.
 *	(Since it is go, however, then it will act the same as start if the
 *	stopwatch has never been started)
 */
void Stopwatch::go()
{
	double elapsedTime;
	unsigned short elapsedSeconds;
	long elapsedPieces;

	if (state == NotStarted)
	{
		start();
		return;
	}
	else if (state == Started)
		return;

	//we do it the following way so we don't have to fiddle with the
	//cases where the milliseconds are negative.
	state = Started;
	elapsedTime = sinceLastRead(); //fool it into taking the time since we stopped it.
	elapsedSeconds = (int)elapsedTime;
	elapsedPieces = (long) (DIVISIONS_PER_SECOND * (elapsedTime - elapsedSeconds));
	if (elapsedPieces > DIVISIONS_PER_SECOND)
	{
		elapsedSeconds += elapsedPieces / DIVISIONS_PER_SECOND;
		elapsedPieces = elapsedPieces % DIVISIONS_PER_SECOND;
	}
	startTime.time += elapsedSeconds;
	startTime.millitm += (unsigned short) elapsedPieces;
}
