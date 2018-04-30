#include "../include/simulator.h"
#include <iostream>
#include <string.h>
#include <queue>
#include <cstdio>
using namespace std;

/* ******************************************************************
 *  ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose
 *
 *     This code should be used for PA2, unidirectional data transfer 
 *        protocols (from A to B). Network properties:
 *           - one way network delay averages five time units (longer if there
 *                are other messages in the channel for GBN), but can be larger
 *                   - packets can be corrupted (either the header or the data portion)
 *                        or lost, according to user-defined probabilities
 *                           - packets will be delivered in the order in which they were sent
 *                                (although some can be lost).
 *                                **********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

/* called from layer 5, passed the data to be sent to other side */

int Aseqno;
int Bseqno;
int Backno;
bool wait;
std::queue<msg> msgqueue;


int Cal_checksum(pkt packet)
{
    char data[21];
    int checksum;

    strncpy(data,packet.payload,20);
    data[20] = '\0';
    int len = strlen(data);

    checksum = packet.seqnum+packet.acknum;

    for(int i=0; i<len; i++)
    {
        checksum += packet.payload[i];
    }

    return checksum;
}

void A_output(struct msg message)
{
    char data[21];
    int checksum;

    strncpy(data,message.data,20);
    data[20] = '\0';
    printf("Message from A's layer 5: %s\n",data);

    int len = strlen(data);
    printf("Message Length: %d\n\n",len);

    pkt packet;
    strncpy(packet.payload,message.data,20);
    packet.seqnum = Aseqno;
    packet.acknum = 0;

    checksum = Cal_checksum(packet);

    packet.checksum = checksum;          

    if(wait == false && msgqueue.empty())
    {
       tolayer3(0,packet);
       starttimer(0,25.0);
       msgqueue.push(message);
       wait = true;
	
       printf("Msg Sent from A AO:\n");
       printf("A Seqnum: %d\n",packet.seqnum);
       printf("A Acknum: %d\n",packet.acknum);
       printf("A Checksum: %d\n\n",packet.checksum);
    }
    
    else
    {
 	 msgqueue.push(message);
    }

}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
    stoptimer(0);
    pkt newpacket;
    int checksum = Cal_checksum(packet);
    if(checksum == packet.checksum)
    {
        if (packet.acknum == Aseqno)
        {
            msgqueue.pop();
            Aseqno = (Aseqno+1)%2;
	        wait = false;
            if(!msgqueue.empty())
            {
                strncpy(newpacket.payload, msgqueue.front().data,20);
                newpacket.seqnum = Aseqno;
                newpacket.acknum = 0;
                newpacket.checksum = Cal_checksum(newpacket);

                tolayer3(0,newpacket);
                starttimer(0,25.0);

		        wait = true;
                printf("Msg Sent from A AIN:\n");
                printf("A Seqnum: %d\n",newpacket.seqnum);
                printf("A Acknum: %d\n",newpacket.acknum);
                printf("A Checksum: %d\n\n",newpacket.checksum);

            }

        }
    }
    else
    {
        if(!msgqueue.empty())
        {
            strncpy(newpacket.payload, msgqueue.front().data,20);
            newpacket.seqnum = Aseqno;
            newpacket.acknum = 0;
            newpacket.checksum = Cal_checksum(newpacket);

            tolayer3(0,newpacket);
            starttimer(0,25.0);
            wait = true;
	        printf("Msg ReSent from A AIR:\n");
            printf("A Seqnum: %d\n",newpacket.seqnum);
            printf("A Acknum: %d\n",newpacket.acknum);
            printf("A Checksum: %d\n\n",newpacket.checksum);
            

        }
    }

}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    pkt resendpkt;

    strncpy(resendpkt.payload, msgqueue.front().data,20);
    resendpkt.seqnum = Aseqno;
    resendpkt.acknum = 0;
    resendpkt.checksum = Cal_checksum(resendpkt);

    tolayer3(0,resendpkt);
    starttimer(0,25.0);
    wait = true;
    printf("TimeOut\n");
    printf("Msg ReSent from A ATO:\n");
    printf("A Seqnum: %d\n",resendpkt.seqnum);
    printf("A Acknum: %d\n",resendpkt.acknum);
    printf("A Checksum: %d\n\n",resendpkt.checksum);

}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
     Aseqno = 0;    
     wait = false;

    std::queue<msg> msgqueue;

}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    pkt Ack;
    char data[21];
    int checksum;

    strncpy(data,packet.payload,20);
    data[21] = '\0';
    printf("Message Received at B: %s\n",data);

    int len = strlen(data);
    printf("Message Length: %d\n\n",len);

    checksum = Cal_checksum(packet);

    if(checksum == packet.checksum)
    {

        Ack.seqnum = 0;
        Ack.acknum = packet.seqnum;
        memset(Ack.payload,0,20);
        Ack.checksum = packet.seqnum;

        tolayer3(1,Ack);

        if(Backno == packet.seqnum)
        {
              tolayer5(1,packet.payload);
              Backno = (Backno+1)%2;
        }
            
             printf("Msg Seqno: %d\n", packet.seqnum);
	         printf("BAck: %d\n\n", Backno);
     }

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
     Bseqno = 0;
     Backno = 0;
}

