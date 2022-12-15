/*
    Name: Broadcaster.cpp
    Description: UDP Broadcaster for encoded Opus audio packets
    
    SRI International
    InSys Intern: Amy Huang
    Date: 11/30/2022
*/

#include "winsock2.h"
#include <iostream>
#include <conio.h>
#include <fstream>
#include "audio.cpp"

using namespace std;

#define MYPORT 9009     // the port users will be connecting to
#define WRITE_FILE 0    // Change to 1 if you want to write the opus file (read and write can't be on the same time)
#define READ_FILE 0     // Change to 1 if you want to read from opus file

#pragma comment(lib, "Ws2_32.lib")


struct Packet
{
	uint8_t datasize;
	uint8_t data[20]; //AUDIO packet payload
	
};


int main()
{
    /* SOCKET SECTION */
    WSADATA wsaData;
    WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
    SOCKET sock;
    sock = socket( AF_INET, SOCK_DGRAM, 0 );
    char broadcast = '1';
    if ( setsockopt( sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast) ) < 0 )
    {
        cout<<"Error in setting Broadcast option";
        closesocket(sock);
        return 0;
    }

    struct sockaddr_in Recv_addr;
    // struct sockaddr_in Sender_addr;
    int len = sizeof( struct sockaddr_in );

    Recv_addr.sin_family       = AF_INET;
    Recv_addr.sin_port         = htons( MYPORT );
    Recv_addr.sin_addr.s_addr =   INADDR_BROADCAST;

    /* END OF SOCKET SECTION */


    // Init portaudio
	PaError paErr = Pa_Initialize();
	if (paErr)
		throw std::runtime_error(string("Could not start audio. Pa_Initialize error: ") + Pa_GetErrorText(paErr));

    // Init opus encoder
    int opusErr;
	encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_VOIP, &opusErr);
	if (opusErr != OPUS_OK)
		throw std::runtime_error(string("opus_encoder_create error: ") + opus_strerror(opusErr));

    opusErr = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(8000));
	if (opusErr != OPUS_OK) {
		throw std::runtime_error(string("opus set bitrate error: ") + opus_strerror(opusErr));
	}

    opusErr = opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(0));
	if (opusErr != OPUS_OK) {
		throw std::runtime_error(string("opus set complexity error: ") + opus_strerror(opusErr));
	}

    opusErr = opus_encoder_ctl(encoder, OPUS_SET_MAX_BANDWIDTH(OPUS_BANDWIDTH_WIDEBAND));
	if (opusErr != OPUS_OK) {
		throw std::runtime_error(string("opus set max bandwidth error: ") + opus_strerror(opusErr));
	}

    opusErr = opus_encoder_ctl(encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
	if (opusErr != OPUS_OK) {
		throw std::runtime_error(string("opus set signal error: ") + opus_strerror(opusErr));
	}

    beginAudioStream(true, false);
    cout << "Sound in : " << Pa_GetDeviceInfo( Pa_GetDefaultInputDevice() )->name << endl;

#if WRITE_FILE
    char output[50];
    cout << "File name to write to: " << endl;
    cin >> output;
    ofstream rec_file;
    
#endif

#if READ_FILE
    char input[50];
    cout << "File name to read from: " << endl;
    cin >> input;
    ifstream audio_file;

    audio_file.open(input, ios::binary);
    uint8_t sendMSG[25];
       
    while(audio_file)
    {
        Packet pack;
        audio_file.read((char*)&pack.datasize, 1);
        audio_file.read((char*)pack.data, pack.datasize);

        cout << "size: " << int(pack.datasize) << " Packet: "; 
        for(int i = 0; i < pack.datasize ; i++)
        {
            cout << (int)pack.data[i] << " ";

        }
        cout << endl;
        sendto( sock, (char*)&pack, int(pack.datasize + 1) , 0,(sockaddr*) &Recv_addr, sizeof( Recv_addr ) );
    }


#else

    while(1)
    {
        if(Pa_GetStreamReadAvailable(stream) >= PACKET_SAMPLES)
        {
            opus_int16 microphone[PACKET_SAMPLES];

            PaError paErr = Pa_ReadStream(stream, microphone, PACKET_SAMPLES);
		    if (paErr && paErr != paInputOverflowed)
		    	throw std::runtime_error(string("Pa_ReadStream error: ") + Pa_GetErrorText(paErr));

    
            Packet pack;

            opus_int32 enc = opus_encode(encoder, microphone, PACKET_SAMPLES, pack.data, sizeof(pack.data));
		    if (enc < 0)
		    	throw std::runtime_error(string("opus_encode error: ") + opus_strerror(enc));

            pack.datasize = enc;
            cout << "size: " << int(pack.datasize) << " Packet: "; 
            for(int i = 0; i < pack.datasize ; i++)
            {
                cout << (int)pack.data[i] << " ";
            }
            cout << endl;

#if WRITE_FILE
            rec_file.open(output, ios::app | ios::binary);
            rec_file.write((char*)&pack.datasize, sizeof(pack.datasize));
            rec_file.write((char*)pack.data, enc);
            rec_file.close();

#endif
                  
            
            sendto( sock, (char*)&pack, int(pack.datasize + 1) , 0,(sockaddr*) &Recv_addr, sizeof( Recv_addr ) );
        }
    }
 #endif
        

    endAudioStream();
    closesocket( sock );
    WSACleanup();
}