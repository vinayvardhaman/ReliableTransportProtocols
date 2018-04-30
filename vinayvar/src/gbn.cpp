#include "../include/simulator.h"
#include <iostream>
#include <string.h>
#include <queue>
#include <list>
#include <cstdio>

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

std::list<msg> msglist;
std::_List_iterator<msg> it;

int base;
int nextseqno;
int expectedseqno;
int n;


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

    msglist.push_back(message);

    if(nextseqno < base+n)
    {
        it = msglist.begin();
        for(int i = 1; i<nextseqno ; i++)
            it++;

        pkt packet;
        strncpy(packet.payload, (*it).data, 20);
        packet.seqnum = nextseqno;
        packet.acknum = 0;

        checksum = Cal_checksum(packet);
        packet.checksum = checksum;

        tolayer3(0, packet);

        if(base == nextseqno)
        {
            starttimer(0, 15.0);
        }

        nextseqno++;

        printf("Msg Sent from A AO:\n");
        printf("A Seqnum: %d\n",packet.seqnum);
        printf("A Acknum: %d\n",packet.acknum);
        printf("A Checksum: %d\n",packet.checksum);
        printf("A Payload: %c\n\n",packet.payload[3]);

    }

}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
    pkt newpkt;
    int checksum = Cal_checksum(packet);
    if(checksum == packet.checksum)
    {
        base = packet.acknum + 1;

        if(base == nextseqno)
        {
            stoptimer(0);
        }
        else
        {
            stoptimer(0);
            starttimer(0,15.0);
        }
    }

}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    it = msglist.begin();
    for(int i = 1; i<base ; i++)
        it++;

    pkt resendpkt;
    starttimer(0,15.0);

    for(int i=base; i<nextseqno; i++,it++)
    {
        strncpy(resendpkt.payload, (*it).data,20);
        resendpkt.seqnum = i;
        resendpkt.acknum = 0;
        resendpkt.checksum = Cal_checksum(resendpkt);

        tolayer3(0,resendpkt);

        printf("TimeOut\n");
        printf("Msg%d ReSent from A ATO:\n",i);
        printf("A Seqnum: %d\n",resendpkt.seqnum);
        printf("A Acknum: %d\n",resendpkt.acknum);
        printf("A Checksum: %d\n",resendpkt.checksum);
        printf("A Payload: %c\n\n",resendpkt.payload[3]);
    }

}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    n = getwinsize();
    base = 1;
    nextseqno = 1;

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

    if(checksum == packet.checksum && expectedseqno == packet.seqnum)
    {

        Ack.seqnum = 0;
        Ack.acknum = expectedseqno;
        memset(Ack.payload, 0, 20);
        Ack.checksum = packet.seqnum;

        tolayer5(1, packet.payload);
        tolayer3(1, Ack);

        printf("Msg Delivered and Ack%d sent to A\n\n", packet.seqnum);

        expectedseqno++;
    }
    else
    {
        Ack.seqnum = 0;
        Ack.acknum = expectedseqno-1;
        memset(Ack.payload, 0, 20);
        Ack.checksum = expectedseqno-1;

        tolayer3(1, Ack);

        printf("Msg corrupted or ooo and Ack%d sent to A\n\n", expectedseqno-1);
    }


}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
    expectedseqno = 1;
}

