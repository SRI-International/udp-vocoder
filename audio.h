/*
    Name: audio.h
    Description: Header file for audio.cpp
    
    SRI International
    InSys Intern: Amy Huang
    Date: 11/30/2022
*/

#include "include/portaudio.h"
#include "include/opus/opus.h"
#include <cassert>
#include <stdexcept>

#pragma comment(linker, "/NODEFAULTLIB:LIBCMT")
#pragma comment(lib, "Msvcrt.lib")
#pragma comment(lib, "../lib/portaudio.lib")
#pragma comment(lib, "../lib/opus.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "Advapi32.lib")



namespace std 
{
    typedef unsigned long  ulong;

    enum Constants 
    {
        CHANNELS             = 1,    //1 channel (mono) audio
        SAMPLE_RATE          = 8000, //The number of 16-bit samples per second
        PACKET_SAMPLES       = 160,  //Samples per packet (SAMPLE_RATE * PACKET_MS = samples) 8000 * 0.020 = 160
    };


}
OpusEncoder* encoder;
OpusDecoder* decoder;
PaStream*    stream;

void beginAudioStream(bool input, bool output);
void writeAudioStream(void* buffer, std::ulong samples);
void endAudioStream();
