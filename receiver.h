/*
    Name: receiver.h
    Description: UDP Receiver for encoded Opus audio packets and decode
    
    SRI International
    InSys Intern: Amy Huang
    Date: 12/15/2022
*/
#include "winsock2.h"
#include <iostream>
#include <conio.h>
#include <deque>
#include <thread>
#include <time.h>
#include <condition_variable>
#include "audio.cpp"

namespace std 
{
    /*Container for Opus Encoded Packets*/
    struct Packet
    {
	    uint8_t datasize;
	    uint8_t data[20];
    };
    typedef deque<Packet> PacketBuffer;     // Audio packet queue
    typedef deque<uint8_t> ByteBuffer;      // Recieved data byte queue
}



#define MYPORT 9009    // the port users will be connecting to

/* Queues for Audio packets */
ByteBuffer              bytebuf;
PacketBuffer            packetbuf;

/* Variables for socket*/
struct sockaddr_in      Recv_addr;
struct sockaddr_in      Sender_addr;
uint8_t                 recvbuff[50];
int                     recvbufflen = 50;
SOCKET                  sock;

struct timeval          timeout={2,0}; //set timeout for 2 seconds
int len = sizeof( struct sockaddr_in );


/* Functions to block the main thread while waiting on packets*/
condition_variable      que;
mutex                   mtx;
bool                    queueEmpty;

void waitForUDP() 
{
    cout << "waiting on udp" << endl;
    unique_lock<mutex> lock(mtx);
    que.wait(lock, []{return bytebuf.size() > 1;});

}

void unpause()
{
    cout << "finished waiting for udp" << endl;
    unique_lock<mutex> lock(mtx);
    que.notify_all();
}


void byteToQueue();
void packetMaker();
void playRecvAudio();