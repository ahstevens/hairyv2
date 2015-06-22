#include"Isotrack.h"
#include<iostream>
#include<cstdio>

using namespace std;

Isotrack::State::State()
{
    position[0] = 0.0;
    position[1] = 0.0;
    position[2] = 0.0;
    orientation[0] = 0.0;
    orientation[1] = 0.0;
    orientation[2] = 0.0;
    orientation[3] = 0.0;
}

Isotrack::State::State(State const &s)
{
    memcpy(position,s.position,3*sizeof(double));
    memcpy(orientation,s.orientation,4*sizeof(double));
}

Isotrack::Isotrack()
:hComm(NULL),CriticalSection(NULL),state(4)
{
}

Isotrack::State Isotrack::GetState(int sensor)
{
    sensor -= 1;
    EnterCriticalSection(CriticalSection);
    State ret = state[sensor];
    LeaveCriticalSection(CriticalSection);
    return ret;
}

bool Isotrack::Initialize(std::string const &port)
{
        // Open com port
        hComm = CreateFile( port.c_str(),GENERIC_READ | GENERIC_WRITE,0,0,OPEN_EXISTING,0,0);
        if (hComm == INVALID_HANDLE_VALUE)
        {
                cerr << "Error opening port" << endl;
                return false;
        }

        // set timeouts
        COMMTIMEOUTS cto = { MAXDWORD, 0, 0, 0, 0 };

        if(!SetCommTimeouts(hComm, &cto))
        {
                cerr << "Error opening port" << endl;
                return false;
        }

        // set DCB

        DCB dcb = {0};
        dcb.DCBlength = sizeof(dcb);
        GetCommState(hComm, &dcb);
        dcb.BaudRate = 115200;
        dcb.ByteSize = 8;
        dcb.Parity = NOPARITY;
        dcb.StopBits = ONESTOPBIT;

        if(!SetCommState(hComm, &dcb))
        {
            cerr << "Error opening port" << endl;
            return false;
        }

        write("\r");
        write("c");
        Sleep(250);

        std::vector<unsigned char> junk = read(100);
        while(!junk.empty())
            junk = read(100);

        write("S");
        std::vector<unsigned char> s = read(1);
        std::string status(s.begin(),s.end());
        while(status.empty() || *status.rbegin() != '\n')
        {
            std::vector<unsigned char> s = read(1);
            status += std::string(s.begin(),s.end());
        }
        if(status.size() != 55)
        {
            std::cerr << "Error retriveing status: " << status.size() << "\t" << status;
            return false;
        }

        CriticalSection = new CRITICAL_SECTION;
        InitializeCriticalSection(CriticalSection); 

        LPDWORD id = 0;
        
        if (status.substr(21,32) == std::string(32,' '))
            hPositionThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)IsotrackPositionThread,this,0,id);
        else
            hPositionThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)FastrackPositionThread,this,0,id);

        return true;
}

void Isotrack::write(std::string const &s)
{
    DWORD SizeWritten;
    WriteFile(hComm, s.c_str(), s.size(), &SizeWritten, 0);
}

std::vector<unsigned char> Isotrack::read(int i, bool wait)
{
    DWORD SizeRead = 0;
    int total_read = 0;
    std::vector<unsigned char> ret(i,0);
    ReadFile(hComm, &ret[total_read], i-total_read, &SizeRead, 0);
    total_read += SizeRead;
    while(wait && total_read < i)
    {
        ReadFile(hComm, &ret[total_read], i-total_read, &SizeRead, 0);
        total_read += SizeRead;
        if (total_read < i)
            Sleep(1);
    }
    ret.resize(total_read);
    return ret;
}

long WINAPI Isotrack::IsotrackPositionThread(void *iso)
{
    Isotrack *isotrack = reinterpret_cast<Isotrack*>(iso);

    isotrack->write("f"); // binary mode
    isotrack->write("u"); // metric
    isotrack->write("O2,11\r\n"); // output position quaternion
    isotrack->write("H1,0,1,0\r\n"); // set hemisphere for sensor 1
    isotrack->write("H2,0,1,0\r\n"); // set hemisphere for sensor 2
    isotrack->write("C"); // continuous mode

    std::vector<unsigned char> data;
    while(true)
    {
        std::vector<unsigned char> c = isotrack->read(1,true);
        if(c[0] > 127)
        {
            if(data.size() == 20)
            {
                std::vector<unsigned char> decode;
                int block = 0;
                int byte = 0;
                bool done = false;
                while(!done)
                {
                    int overflow = block*8+7;
                    if(overflow > data.size())
                        overflow = data.size()-1;
                    decode.push_back( (data[block*8+byte]&0x7f) | (((data[overflow]>>byte)&0x01)<<7));
                    ++byte;
                    if(byte >= 7)
                    {
                        ++block;
                        byte = 0;
                    }
                    if(block*8+byte >= (data.size() - 1))
                    {
                        int station = decode[1]-'1';
                        short * ints = reinterpret_cast<short*>(&decode[3]);
                        EnterCriticalSection(isotrack->CriticalSection); 
                        for(int i = 0; i < 3; ++i)
                            isotrack->state[station].position[i] = ints[i]*0.005075838;
                        for(int i = 0; i < 4; ++i)
                            isotrack->state[station].orientation[i] = ints[3+i]/32767.0;
                        LeaveCriticalSection(isotrack->CriticalSection);
                        done = true;
                    }
                }
            }
            data.clear();
        }
        data.insert(data.end(),c.begin(),c.end());
    }
    return 0;
}

long WINAPI Isotrack::FastrackPositionThread(void *iso)
{
    Isotrack *isotrack = reinterpret_cast<Isotrack*>(iso);
    EnterCriticalSection(isotrack->CriticalSection); 
    LeaveCriticalSection(isotrack->CriticalSection);
    return 0;
}
