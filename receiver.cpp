/*
    Name: receiver.cpp
    Description: UDP Receiver for encoded Opus audio packets and decode
    
    SRI International
    InSys Intern: Amy Huang
    Date: 11/30/2022
*/


#include "receiver.h"
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

// TODO: Check for memory leaks 

/* Wait to receive data and push to byte buffer */
void byteToQueue()
{
    // set up for select() to wait for nothing and if there is nothing, timeout
    cout << "byteToQueue thread id: " << this_thread::get_id() << endl;

    while(1)
    {
        fd_set socks;
        FD_ZERO(&socks);
        FD_SET(sock, &socks);
    
        if(select(sock+1, &socks, NULL, NULL, &timeout)) 
        {
            int ret = recvfrom( sock, (char*)&recvbuff, recvbufflen, 0, (sockaddr*) &Sender_addr, &len );

            for(int i = 0; i < ret; i++)
            {
                bytebuf.push_back(recvbuff[i]);
            }
            if(queueEmpty)
            {
                queueEmpty = false;
                unpause();
            }     
            
        }else{
            cout << "timeout on UDP" <<endl;
        }
    }
}

/* 
    If there is data in the queue, pick up pattern and create new packet, 
    then add it to the packet queue.
*/
void packetMaker()
{
    bool tocfound = false;
    int datacount = 0; 
    int size = 0;

    Packet* pack;       // pointer to the end of the packet queue;
    Packet empty = {};

    while(1)
	{
        if (bytebuf.size() < 2)
        {
            queueEmpty = true;
            waitForUDP();
        }

		uint8_t last = bytebuf.front();
		bytebuf.pop_front();
		uint8_t cur = bytebuf.front();

        // If we found the size + toc byte, push the empty packet 
		if (cur == 0x08 && last <= 0x18 && last > 0x00 && (tocfound == false))
		{
            tocfound = true;
            size = (int)last;		// set data size to the new size

            packetbuf.push_back(empty);
            pack = &packetbuf.back();

			pack->datasize = size;
			pack->data[0] = cur;

			datacount++;
			continue;
		}
        // fill packet data array
		if (tocfound)
		{
			pack->data[datacount] = cur;
			datacount++;

            // packet completed
			if (datacount == size) { return;} 
		}
	}

}


/* Decode audio from queue and play it */
void playRecvAudio()
{
    opus_int16 decoded[PACKET_SAMPLES];
    opus_int32 decodeRet;

    if(packetbuf.front().datasize)
    {
        Packet& front = packetbuf.front();

        //print out packets
        cout << " Size: " << (int) front.datasize << " [" ;
        for(int i = 0; i < front.datasize; i++)
        {
            cout << (int) front.data[i] << " ";
        }
        cout << "]" << endl;


        decodeRet = opus_decode(decoder, front.data, front.datasize, decoded, PACKET_SAMPLES, 0);

        if (decodeRet == OPUS_INVALID_PACKET)
		{
			cout << "Corrupt packet" << endl;
			// Try again by treating the packet as lost
			decodeRet = opus_decode(decoder, NULL, 0, decoded, PACKET_SAMPLES, 0);
		}
    }
    // Check for Opus error from above
	if (decodeRet < 0)
		throw std::runtime_error(string("opus_decode failed: ") + opus_strerror(decodeRet));

    packetbuf.pop_front();

    writeAudioStream(decoded, PACKET_SAMPLES);
    
}



int main()
{
    /* This section is to set up the socket */

    WSADATA wsaData;
    WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
    
    sock = socket( AF_INET, SOCK_DGRAM, 0 );    
    char broadcast = '1';

    Recv_addr.sin_family       = AF_INET;
    Recv_addr.sin_port         = htons( MYPORT );
    Recv_addr.sin_addr.s_addr  = INADDR_ANY;

    if ( bind( sock, (sockaddr*) &Recv_addr, sizeof (Recv_addr) ) < 0 )
    {
        cout << "Error in BINDING" << WSAGetLastError();
       _getch();
        closesocket(sock);
        return 0;
    }

    /* Socket Section Ends */

    // Init portaudio will print out a lot of meta data
	PaError paErr = Pa_Initialize();
	if (paErr)
		throw std::runtime_error(string("Could not start audio. Pa_Initialize error: ") + Pa_GetErrorText(paErr));


    cout << "Ready to recieve messages!" << endl;

    cout << "*** Playback started" << endl;
	cout << "Sound out: " << Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice())->name << endl;


    int opusErr;
	decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &opusErr);
	if (opusErr != OPUS_OK)
		throw std::runtime_error(string("opus_decoder_create error: ") + opus_strerror(opusErr));

    beginAudioStream(false, true);

    cout << "Main thread id: " << this_thread::get_id() << endl;
   
   // create an independent thread to receive bytes
    thread p(byteToQueue);
    p.detach();
    
    chrono::seconds dura(2);
    this_thread::sleep_for(dura);
    cout<< "waited 2 sec" << endl;

    do
    {
        packetMaker();
        playRecvAudio();

    } while(1);
    
    endAudioStream();

    closesocket(sock);
    WSACleanup();
}



