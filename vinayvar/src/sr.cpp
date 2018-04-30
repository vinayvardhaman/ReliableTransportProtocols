#include "../include/simulator.h"
#include <iostream>
#include <string.h>
#include <queue>
#include <list>
#include <cstdio>
#include <cstring>

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

struct srmsg
{
    msg messge;
    bool isAcked;
};

struct pkttimer
{
    float timeout;
    int seqno;
};

msg *buff;
int nb;

std::list<msg> msglist;
std::_List_iterator<msg> it;
int nextseqno;
int n;

std:: list<bool> acklist;
std:: _List_iterator<bool> itack;

std:: list<bool> rcvlist;
std:: _List_iterator<bool> itrcv;

std::queue<msg> rcvbuff;

int sendbase;
int rcvbase;

int timeout_pktno;
std::list<pkttimer> timerlist;
std::_List_iterator<pkttimer> timerit;
float basetime;

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
    acklist.push_back(false);
    if(nextseqno < sendbase+n)
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

        pkttimer packettimer;


        if(packet.seqnum == 1)
        {
            basetime = get_sim_time();
            packettimer.timeout = basetime;
            packettimer.seqno = packet.seqnum;

            starttimer(0,30.0);
            timerlist.push_back(packettimer);
        }
        else
        {
            packettimer.timeout = get_sim_time();
            packettimer.seqno = packet.seqnum;
            timerlist.push_back(packettimer);
        }
        nextseqno++;
        printf("Msg Sent from A AO:\n");
        printf("A Seqnum: %d\n",packet.seqnum);
        printf("A Acknum: %d\n",packet.acknum);
        printf("A Checksum: %d\n",packet.checksum);
        printf("A Timer gst %f, TO %f\n",get_sim_time(),packettimer.timeout);
        printf("A Payload: %c\n\n",packet.payload[3]);

    }

}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{

    pkt sendpacket;
    int checksum = Cal_checksum(packet);
    if(checksum == packet.checksum)
    {
        printf("Ack%d Received at A and send base=%d\n\n",packet.acknum,sendbase);

        itack = acklist.begin();

        for(int i=1; i<packet.acknum; i++)
        {
            itack++;
        }

        *itack = true;

        if(sendbase == packet.acknum)
        {
            itack = acklist.begin();

            for(int i=1; i<sendbase; i++)
            {
                itack++;
            }

            for(; *itack == true; itack++)
            sendbase++;

            printf("Send base increased to %d\n\n",sendbase);

            /*
 *             if(nextseqno < sendbase+n && nextseqno<=msglist.size())
 *                         {
 *                                         itack = acklist.begin();
 *                                                         it = msglist.begin();
 *
 *                                                                         for(int i=1; i<nextseqno; i++)
 *                                                                                         {
 *                                                                                                             itack++;
 *                                                                                                                                 it++;
 *                                                                                                                                                 }
 *
 *                                                                                                                                                                 for(int i=nextseqno; i<sendbase+n && i<=msglist.size(); i++)
 *                                                                                                                                                                                 {
 *                                                                                                                                                                                                     pkttimer packettimer;
 *                                                                                                                                                                                                                         strncpy(sendpacket.payload,(*it).data,20);
 *                                                                                                                                                                                                                                             sendpacket.seqnum = i;
 *                                                                                                                                                                                                                                                                 sendpacket.acknum = 0;
 *                                                                                                                                                                                                                                                                                     sendpacket.checksum = Cal_checksum(sendpacket);
 *
 *                                                                                                                                                                                                                                                                                                         packettimer.seqno = sendpacket.seqnum;
 *                                                                                                                                                                                                                                                                                                                             packettimer.timeout = get_sim_time()-timerlist.back().timeout+20.0;
 *
 *                                                                                                                                                                                                                                                                                                                                                 tolayer3(0,sendpacket);
 *                                                                                                                                                                                                                                                                                                                                                                     timerlist.push_back(packettimer);
 *                                                                                                                                                                                                                                                                                                                                                                                         nextseqno++;
 *                                                                                                                                                                                                                                                                                                                                                                                                             it++;
 *
 *                                                                                                                                                                                                                                                                                                                                                                                                                                 printf("Msg%d Sent from AIN:\n",i);
 *                                                                                                                                                                                                                                                                                                                                                                                                                                                     printf("A Seqnum: %d\n",sendpacket.seqnum);
 *                                                                                                                                                                                                                                                                                                                                                                                                                                                                         printf("A Acknum: %d\n",sendpacket.acknum);
 *                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             printf("A Checksum: %d\n",sendpacket.checksum);
 *                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 printf("A Timeout; %f\n",packettimer.timeout);
 *                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     printf("A Payload: %c\n\n",sendpacket.payload[3]);
 *                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     }
 *                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 } */
        }

    }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    float prvtimeout;

    timerit = timerlist.begin();

    for(int i=1; i<timeout_pktno ;i++)
        timerit++;

    prvtimeout = (*timerit).timeout;

    itack = acklist.begin();

    for(int i=1; i<(*timerit).seqno; i++)
    {
        itack++;
    }

    if(*itack == false)
    {
        it = msglist.begin();

        for(int i=1; i<(*timerit).seqno; i++)
            it++;

        pkt resendpkt;
        resendpkt.seqnum = (*timerit).seqno;
        resendpkt.acknum = 0;
        strncpy(resendpkt.payload,(*it).data,20);
        resendpkt.checksum = Cal_checksum(resendpkt);

        tolayer3(0,resendpkt);

        pkttimer packettimer;
        packettimer.seqno = resendpkt.seqnum;
        packettimer.timeout = get_sim_time();
        timerlist.push_back(packettimer);


        timerit++;
        starttimer(0, (*timerit).timeout - prvtimeout);
        printf("timer for packet %d startedRT \n\n", (*timerit).seqno);

        timeout_pktno++;

        printf("TimeOut\n");
        printf("Msg ReSent from A ATO:\n");
        printf("A Seqnum: %d\n",resendpkt.seqnum);
        printf("A Acknum: %d\n",resendpkt.acknum);
        printf("A Checksum: %d\n",resendpkt.checksum);
        printf("A timeout: %f\n", packettimer.timeout);
        printf("A Timer gst %f, TO %f\n",get_sim_time(),packettimer.timeout);
        printf("A Payload: %c\n\n",resendpkt.payload[3]);
    }
    else
    {
        if(timerlist.size()>timeout_pktno)
        {
            timerit++;
            starttimer(0, (*timerit).timeout - prvtimeout);
            printf("timerlist size %lu",timerlist.size());
            timeout_pktno++;
        }
        else
        {
            (*timerit).timeout = get_sim_time();
            starttimer(0,30.0);
            printf("timer for packet %d ended and next packet not arrived:\n\n",(*timerit).seqno);
        }
    }

}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    n = getwinsize();
    printf("A Init %d",n);
    sendbase = 1;
    nextseqno = 1;
    acklist.push_back(false);
    timeout_pktno = 0;

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
        printf("Rcv Base:%d\n\n",rcvbase);
        if(packet.seqnum>=rcvbase && packet.seqnum<rcvbase+n)
        {
            Ack.seqnum = 0;
            Ack.acknum = packet.seqnum;
            memset(Ack.payload,0,20);
            Ack.checksum = Cal_checksum(Ack);

            tolayer3(1,Ack);
            printf("Msg Received and Ack%d sent to A\n\n",Ack.acknum);

            for(int i = rcvlist.size(); i<=packet.seqnum; i++)
            {
                rcvlist.push_back(false);
            }

            itrcv = rcvlist.begin();

            for(int i=1; i<packet.seqnum; i++)
                itrcv++;

            if(packet.seqnum != rcvbase)
            {
                if(*itrcv==false)
                {
                    int pos = packet.seqnum-rcvbase;
                    strncpy(buff[pos].data, packet.payload,20);

                    *itrcv = true;
                    printf("message buffered %c\n\n", packet.payload[5]);
                }

            }
            else
            {
                *itrcv = true;

                itrcv = rcvlist.begin();
                for(int i=1; i<rcvbase; i++)
                    itrcv++;

                int count = 0;
                for(;*itrcv== true;itrcv++,count++)
                    rcvbase++;

                tolayer5(1,packet.payload);

                pkt sendpkt;
                for(int i=1;i<count;i++)
                {
                    tolayer5(1,buff[i].data);
                }

                for(int i=0;i<n-count;i++)
                {
                    strncpy(buff[i].data,buff[i+count].data,20);
                }
                printf("Messages %d to %d conseq Msgs are delivered\n\n",packet.seqnum, count-1);
            }

        }
        else if(packet.seqnum>=rcvbase-n && packet.seqnum<rcvbase)
        {
            Ack.seqnum = 0;
            Ack.acknum = packet.seqnum;
            memset(Ack.payload,0,20);
            Ack.checksum = Cal_checksum(Ack);

            tolayer3(1,Ack);
            printf("Msg Received and Ack%d sent to A\n\n",Ack.acknum);

        }
    }

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
    nb = getwinsize();
    rcvbase = 1;
    rcvlist.push_back(false);
    buff = new msg[nb];
}

