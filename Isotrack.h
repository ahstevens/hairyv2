#ifndef ISOTRACK_H
#define ISOTRACK_H

#include<windows.h>
#include <string>
#include <vector>

class Isotrack
{
    public:
        struct State
        {
            double position[3];
            double orientation[4];

            State();
            State(State const &s);
        };

        Isotrack();

        bool Initialize(std::string const &port);
        State GetState(int sensor);

    private:
        HANDLE hComm;
        HANDLE hPositionThread;
        std::vector<State> state;

        CRITICAL_SECTION * CriticalSection;

        static long WINAPI FastrackPositionThread(void *iso);
        static long WINAPI IsotrackPositionThread(void *iso);

        void write(std::string const &);
        std::vector<unsigned char> read(int i, bool wait = false);
};

#endif
