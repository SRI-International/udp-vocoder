/*
    Name: audio.cpp
    Description: Audio stream functions for receiver.cpp
    
    SRI International
    InSys Intern: Amy Huang
    Date: 11/30/2022
*/

#include "audio.h"

using namespace std;


void beginAudioStream(bool input, bool output)
{
	assert(input || output);

	if (stream)
		endAudioStream();

	const int inChannels = input ? CHANNELS : 0;
	const int outChannels = output ? CHANNELS : 0;

	PaError paErr;
	paErr = Pa_OpenDefaultStream(&stream, inChannels, outChannels, paInt16, SAMPLE_RATE, PACKET_SAMPLES, NULL, NULL);
	if (paErr)
		throw std::runtime_error(string("Pa_OpenDefaultStream error: ") + Pa_GetErrorText(paErr));

	paErr = Pa_StartStream(stream);
	if (paErr)
		throw std::runtime_error(string("Pa_StartStream error: ") + Pa_GetErrorText(paErr));
}

void writeAudioStream(void* buffer, std::ulong samples)
{
	assert(stream);
	PaError paErr = Pa_WriteStream(stream, buffer, samples);
	if (paErr != paNoError)
	{
		if (paErr == paOutputUnderflowed) {
			//log << "Pa_WriteStream output underflowed" << endl;
		}
		else
			throw std::runtime_error(string("Pa_WriteStream failed: ") + Pa_GetErrorText(paErr));
	}
}

void endAudioStream()
{
	assert(stream);
	PaError paErr = Pa_CloseStream(stream);
	if (paErr)
		throw std::runtime_error(string("Pa_CloseStream error: ") + Pa_GetErrorText(paErr));
	stream = NULL;
}