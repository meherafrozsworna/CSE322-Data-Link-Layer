//100  //20
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: SLIGHTLY MODIFIED
 FROM VERSION 1.1 of J.F.Kurose

   This code should be used for PA2, unidirectional or bidirectional
   data transfer protocols (from A to B. Bidirectional transfer of data
   is for extra credit and is not required).  Network properties:
   - one way network delay averages five time units (longer if there
       are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
       or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
       (although some can be lost).
**********************************************************************/

#define BIDIRECTIONAL 1 /* change to 1 if you're doing extra credit */
/* and write a routine called B_output */

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct pkt
{
    char data[20];
};

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct frm
{
    int type;
    int seqnum;
    int acknum;
    int checksum;
    char payload[20];
};

/********* FUNCTION PROTOTYPES. DEFINED IN THE LATER PART******************/
void starttimer(int AorB, float increment);
void stoptimer(int AorB);
void tolayer1(int AorB, struct frm packet);
void tolayer3(int AorB, char datasent[20]);

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/


/********* for A ***********/
int seqnum_A = 0;
int acknum_A = 0 ;
int ack_recieve_A = 1;
struct frm packet_A_current ;
int have_piggyback_frame_A = -10 ;

/********* for B ***********/
int seqnum_B = 0;
int acknum_B = 0;
int ack_recieve_B = 1;
struct frm packet_B_current ;
int have_piggyback_frame_B = 10 ;


/********  others  *********/
int piggyback = 1 ;
int crc_show = 0 ;
int polynomial ;
int frame_count ;
char gen[1000] ;
//char gen[] = "1101";


#define TIMER_INCREMENT 15.0
/*****---------------(pgback --> timer 10 , pkt interval = 20) ----------------*****/
/*****---------------(pgback --> timer 15 , pkt interval = 100) ----------------*****/



void substr(const char *str, int position, int c, char sub[])
{
    int i = 0;
    while(i<c) {
      sub[i] = str[position+i];
      i++;
   }
   sub[i] = '\0';
}

void xor(char a[] , char b[], char result[])
{
    int i , j ;
    for(i = 0 ; i < strlen(a) ; i++)
    {
        if(a[i] == b[i])
            result[i] = '0';
        else
            result[i] = '1';

    }
    result[strlen(a)] = '\0';
    //printf("****%s\n",result);
    //printf("%d\n",strlen(result));

}

int binary_to_int(char bin[])
{
    int i ,j , result= 0 , k = 0;
    for( i = strlen(bin)-1 ;i >=0 ; i--)
    {
        if(bin[i]== '1'){
            result += pow(2,k);
        }
        k++;
    }
    return result ;
}


int crc(char data[])
{

    int data_len = strlen(data) ;

    int gen_len = strlen(gen);

    //printf("%d\n",gen_len);

    int i ,j ;
    char xor_result[1000];
    /*for( i = 0 ; i < gen_len-1 ; i++)
    {
        data[i+data_len] = '0';
    }
    data[i+data_len] = '\0';
    */
    //printf("data : %s\n",data);
    data_len = strlen(data) ;
    //printf("%d\n",data_len);
    for(i= 0; i < data_len-gen_len+1 ; i++)
    {
        if(data[i] == '1'){
            char sub[1000] ;
            substr(data,i,gen_len,sub);
            //printf("%s*************\n",sub);
            xor(sub,gen,xor_result);
            //printf("%s    ****  ***\n",xor_result);

            for(j = 0 ; j < gen_len ; j++)
            {
                data[i+j] = xor_result[j];
            }

            //printf("data : %s\n",data);
        }

    }

    char rem[1000] ;
    //printf("in crc  data after -->crc : %s\n",data);
    substr(data,data_len-gen_len+1,gen_len-1,rem);
    //printf("in crc  rem : %s\n",rem);
    //printf("returning : %d\n",binary_to_int(rem));

    if(crc_show == 1){
        printf("in crc  rem : %s\n",rem);
    }

    return binary_to_int(rem);
}


int frame_to_binary_data(struct frm frame , char data[])
{
    char payload[20] ;
    strcpy(payload,frame.payload);

    //char generator[] = "1001";
    //char data[99999] ;

    int i ,j , k=0;
    for(i= 0 ; i < strlen(payload) ; i++)
    {
        char a[100];
        int n = (int)payload[i];
        //printf("%d\n",n);
        for(j=0;n>0;j++)
        {
            //a[j]=n%2;
            if(n%2 == 0){
                a[j] = '0';
            }else{
                a[j] = '1';
            }
            n=n/2;
        }

        for(j=j-1;j>=0;j--)
        {
            //printf("%d",a[i]);
            data[k] = a[j];
            k++;

        }
    }


    //seq --.0
    int seq_ack_type[3];
    seq_ack_type[0] = frame.seqnum ;
    seq_ack_type[1] = frame.acknum ;
    seq_ack_type[2] = frame.type ;

    int p ;
    for(p = 0 ; p < 3 ; p++)
    {
        char a[100];
        int n = seq_ack_type[p] ;
        //printf("%d\n",n);
        for(j=0;n>0;j++)
        {
            //a[j]=n%2;
            if(n%2 == 0){
                a[j] = '0';
            }else{
                a[j] = '1';
            }
            n=n/2;
        }
        for(j = j ; j < 16 ; j++) // 16 bit
        {
            a[j] = '0';
        }

        for(j=j-1;j>=0;j--)
        {
            //printf("%d",a[i]);
            data[k] = a[j];
            k++;
        }
    }

    data[k] = '\0';

    for( i = 0 ; i < strlen(gen)-1 ; i++)
    {
        data[i+k] = '0';
    }
    data[i+k] = '\0';

    //printf("\nBinary of Given Number is=%s   *\n\n",data);
    //int rem = crc(data, generator) ;

}

int calculate_crc_rem(struct frm frame)
{
    char data[99999];
    frame_to_binary_data(frame,data);
    if(crc_show == 1){
        printf("data : %s\n",data);
        printf("generator polynomial : %s\n",gen);
    }
    int rem = crc(data);
    if(crc_show == 1){
        printf("rem : %d\n",rem);
    }
    return rem ;
}


void int_to_binary(int n , char bin[])
{
    int i=0 , j=0 ;
    char a[100];
    for(j=0;n>0;j++)
    {
        //printf("n --> %d\n",n);
        if(n%2 == 0){
            a[j] = '0';
        }else{
            a[j] = '1';
        }
        n = n/2;
    }

    for(j=j-1;j>=0;j--)
    {
        //printf("a[i]  --> %c  \n",a[i]);
        bin[i] = a[j];
        i++;
    }
    bin[i] = '\0';

}
int varify_crc_receive(struct frm frame)
{
    char data[99999];
    frame_to_binary_data(frame,data);

    //printf("data  : %s\n\n",data);

    char rem[999] ;

    int_to_binary(frame.checksum,rem);

    //printf("varify --> rem  : %d\n\n",frame.checksum);
    //printf("varify --> rem  : %s\n\n",rem);

    int k = strlen(data)-1 , i;
    for(i = strlen(rem) -1 ; i>=0 ; i--)
    {
        data[k] = rem[i] ;
        k--;
    }

    if(crc_show == 1){
        printf("Check in receiver : \n");
        printf("data  : %s\n\n",data);
        printf("generator polynomial : %s\n",gen);
    }
    //printf("data  : %s\n\n",data);
    int result = crc(data);
    if(crc_show == 1){
        printf(" crc rem after receive : %d\n\n",result);
    }

    if(result == 0){
        return 1 ;
    }

    if(crc_show == 1){
        printf("\nerror occur\n\n ");
    }
    return 0;
}

/*
int calculateChecksum(struct frm frame)
{
    int checksum = 0;
    int i;
    for(i = 0 ; i < 20 ; i++){
        checksum+= (int)frame.payload[i];
    }
    checksum+= frame.seqnum ;
    checksum+= frame.acknum ;
    checksum+=frame.type;

    return checksum ;
}
*/

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct pkt packet)
{
    printf("AAAAAAAAAAAAAAAAAAAAAAAAAA\n\n");
    frame_count-- ;
    //printf("%d  %d\n\n",piggyback,have_piggyback_frame_A);
    //printf("%d %d\n\n",seqnum_A, acknum_A);

    if(ack_recieve_A == 1){
        printf("\n\n______________________________________________________\n\n");
        struct frm frame ;

        if( seqnum_A == 0 ){
            frame.seqnum = 0 ;
        }else{
            frame.seqnum = 1 ;
        }

        if(piggyback == 1 && have_piggyback_frame_A == 1 )
        {
            frame.acknum = 1- acknum_A ;
            /*if(acknum_A == 0){
                acknum_A = 1 ;
            }else{
                acknum_A = 0 ;
            }*/
            frame.type = 2 ;   // changed
        }else{
            frame.acknum = -10 ;
            frame.type = 0 ;   // changed
        }

        strcpy(frame.payload,packet.data);
        frame.checksum = calculate_crc_rem(frame);
        tolayer1(0,frame);
        printf("A --> (A_output) frame going to layer1 , frame :  %s\n",frame.payload );
        ack_recieve_A = 0;

        starttimer(0,TIMER_INCREMENT);
        packet_A_current = frame ;

    }else{
        printf("\n\n____________________________________________________________\n");
        printf("A --> (A_output) frame dropped , Another frame in transmission ???\n\n");
    }



}

/* need be completed only for extra credit */
void B_output(struct pkt packet)
{

    printf("BBBBBBBBBBBBBBBBBB\n\n");
    frame_count-- ;
    //printf("%d %d\n\n",seqnum_B, acknum_B);

    //printf("%d  %d\n\n",piggyback,have_piggyback_frame_B);
    if(ack_recieve_B == 1){
        printf("\n\n______________________________________________________\n\n");
        struct frm frame ;


        if( seqnum_B == 0 ){
            frame.seqnum = 0 ;
        }else{
            frame.seqnum = 1 ;
        }

        if(piggyback == 1 && have_piggyback_frame_B == 1 )
        {
            printf("piggyback == 1 && have_piggyback_frame_B == 1\n");
            frame.acknum = 1 - acknum_B ;
            /*if(acknum_B == 0){
                acknum_B = 1 ;
            }else{
                acknum_B = 0 ;
            }*/
            frame.type = 2 ;   // changed

        }else{
            frame.acknum = -10 ;
            frame.type = 0 ;   // changed
        }


        strcpy(frame.payload,packet.data);
        frame.checksum = calculate_crc_rem(frame);
        tolayer1(1,frame);
        printf("B --> (B_output) frame going to layer1 , frame :  %s\n",frame.payload );
        ack_recieve_B = 0;

        starttimer(1,TIMER_INCREMENT);
        packet_B_current = frame ;

    }else{
        printf("\n\n____________________________________________________________\n");
        printf("B --> (B_output) frame dropped , Another frame in transmission ???\n\n");
    }
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct frm frame)
{
    printf("frame type in A input   %d\n\n",frame.type);
    //printf("%d %d  ---   %d  %d\n\n",seqnum_A, acknum_A,frame.seqnum, frame.acknum);
    printf("%s  \n",frame.payload);

    int result = varify_crc_receive(frame) ;
    struct frm ackPacket ;
    ackPacket.seqnum = frame.seqnum ;
    strcpy(ackPacket.payload,frame.payload);

    if(result == 0 ){
        ackPacket.type = 1 ;
        printf("A --> (A_input) packet corrupted in B side \n");
        if(acknum_A == 0){
            ackPacket.acknum = 1 ;
        }else{
            ackPacket.acknum = 0 ;
        }

        if(piggyback == 1 && have_piggyback_frame_A==1){
            ackPacket.acknum = acknum_A ;
        }

        ackPacket.checksum = calculate_crc_rem(ackPacket);
        //printf("B---> %d %d\n",ackPacket.seqnum , ackPacket.acknum);
        tolayer1(0,ackPacket);
        printf("A --> (A_input) nack is send to A \n");
        return ;
    }

    if(frame.type == 0)
    {
        //ackPacket.type = 1 ;
        if(frame.seqnum != acknum_A)
        {
            ackPacket.type = 1 ;

            printf("A --> (A_input) duplicate packet received \n");
            if(acknum_A == 0){
                ackPacket.acknum = 1 ;
            }else{
                ackPacket.acknum = 0 ;
            }

            if(piggyback == 1){
                ackPacket.acknum = acknum_A ;/***.....karon ack already toggle hoise , jeta aout theke pathabo..............................?***/
            }

            ackPacket.checksum = calculate_crc_rem(ackPacket);
            //printf("B---> %d %d\n",ackPacket.seqnum , ackPacket.acknum);
            tolayer1(0,ackPacket);
            printf("A --> (A_input) nack is send to A \n");
            return ;
        }

        if(acknum_A == 0){
            ackPacket.acknum = 0 ;
        }else{
            ackPacket.acknum = 1 ;
        }

        ackPacket.checksum = calculate_crc_rem(ackPacket);

        printf("A --> (A_input) packet recieved : %s\n",frame.payload);
        printf("A --> (A_input) sending packet to layer 5\n");
        tolayer3(0,frame.payload);

        //printf("B---> %d %d",ackPacket.seqnum , ackPacket.acknum);

        if(piggyback == 0)
        {
            ackPacket.type = 1 ;
            printf("A --> (A_input) sending ack to B\n");
            tolayer1(0,ackPacket);

            if(acknum_A == 0){
                acknum_A = 1;
            }else{
                acknum_A = 0;
            }
            return ;

        }
        else
        {

            printf("piggyback\n\n");
            have_piggyback_frame_A = 1 ;
            //piggyback_ack_A = acknum_A ;
            if(acknum_A == 0){
                acknum_A = 1;
            }else{
                acknum_A = 0;
            }
            return ;
        }

    }
    else if(frame.type == 1)
    {
        have_piggyback_frame_A = 1 ;

        if(seqnum_A == frame.acknum ){
            stoptimer(0);
            //printf("VVVVVVVVVV     A   VVVVVVVVVV       A        VVVVV\n\n");
            ack_recieve_A = 1;
            printf("A --> (A_input) Ack recieved from B .\n");

            if(seqnum_A == 0){
                seqnum_A = 1 ;
            }else{
                seqnum_A = 0 ;
            }

        }
        else{
            printf("A --> (A_input) packet corrupted or got a nack \n");
        }
        return;

    }
    else if(frame.type == 2)
    {
        have_piggyback_frame_A = 1;
        if(frame.seqnum != acknum_A || seqnum_A != frame.acknum)
        {
            ackPacket.type = 1 ;

            printf("A --> (A_input) duplicate packet received or nack receive \n");
            if(acknum_A == 0){
                ackPacket.acknum = 1 ;
            }else{
                ackPacket.acknum = 0 ;
            }

            if(piggyback == 1)
            {
                //ackPacket.acknum = acknum_A;  // ---------------------risky
            }

            ackPacket.checksum = calculate_crc_rem(ackPacket);
            //printf("B---> %d %d\n",ackPacket.seqnum , ackPacket.acknum);
            tolayer1(0,ackPacket);
            printf("A --> (A_input) nack is send to A \n");
            return ;
        }

        ack_recieve_A = 1;
        stoptimer(0);
        printf("A --> (A_input) Ack & data received from B .\n");

        if(seqnum_A == 0){
            seqnum_A = 1 ;
        }else{
            seqnum_A = 0 ;
        }


        printf("A --> (A_input) packet recieved : %s\n",frame.payload);
        printf("A --> (A_input) sending packet to layer 5\n");
        tolayer3(0,frame.payload);

        //added
        if(acknum_A == 0){
            acknum_A = 1;
        }else{
            acknum_A = 0;
        }


        if(frame_count == 0){
            ackPacket.type = 1 ;

            if(acknum_A == 0){
                ackPacket.acknum = 1 ;
            }else{
                ackPacket.acknum = 0 ;
            }

            ackPacket.checksum = calculate_crc_rem(ackPacket);
            printf("A --> (A_input) sending ack to B\n");
            tolayer1(0,ackPacket);

            /*if(acknum_A == 0){
                acknum_A = 1;
            }else{
                acknum_A = 0;
            }*/
        }



    }


}

/* called when A's timer goes off */
void A_timerinterrupt(void)
{
    printf("A --> (A_timerinterrupt) A timer , retransmission \n");
    starttimer(0,TIMER_INCREMENT);
    tolayer1(0,packet_A_current);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init(void)
{
    seqnum_A = 0;
    ack_recieve_A = 1;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct frm frame)
{

    printf("B --> (B_input) frame %d\n\n",frame.type);
    printf("B --> (B_input) start %s\n\n",frame.payload);

    //printf("%d %d  ---   %d  %d\n\n",seqnum_B, acknum_B,frame.seqnum, frame.acknum);
    //printf("%s  \n",frame.payload);


    int result = varify_crc_receive(frame) ;
    struct frm ackPacket ;
    ackPacket.seqnum = frame.seqnum ;
    strcpy(ackPacket.payload,frame.payload);

    if(result == 0 ){
        printf("B --> (B_input) packet corrupted in B side \n");
        if(acknum_B == 0){
            ackPacket.acknum = 1 ;
        }else{
            ackPacket.acknum = 0 ;
        }

        if(piggyback == 1){
            ackPacket.acknum = acknum_B ;
        }

        ackPacket.checksum = calculate_crc_rem(ackPacket);
        //printf("B---> %d %d\n",ackPacket.seqnum , ackPacket.acknum);
        tolayer1(1,ackPacket);
        printf("B --> (B_input) nack is send to A \n");
        return ;
    }

    if(frame.type == 0)
    {
        //ackPacket.type = 1 ;

        printf("B --> (B_input) %d\n\n",frame.type);
        if(frame.seqnum != acknum_B)
        {
            ackPacket.type = 1 ;
            printf("B --> (B_input) duplicate packet received \n");

            if(acknum_B == 0){
                ackPacket.acknum = 1 ;
            }else{
                ackPacket.acknum = 0 ;
            }

            if(piggyback == 1){
              ackPacket.acknum = acknum_B ;
            }

            ackPacket.checksum = calculate_crc_rem(ackPacket);
            //printf("B---> %d %d\n",ackPacket.seqnum , ackPacket.acknum);
            tolayer1(1,ackPacket);
            printf("B --> (B_input) nack is send to A \n");
            return ;
        }
        if(piggyback == 0)
        {
            ackPacket.type = 1 ;
            if(acknum_B == 0){
                ackPacket.acknum = 0 ;
            }else{
                ackPacket.acknum = 1 ;
            }

            ackPacket.checksum = calculate_crc_rem(ackPacket);

            printf("B --> (B_input) packet recieved : %s\n",frame.payload);
            printf("B --> (B_input) sending packet to layer 5\n");
            tolayer3(1,frame.payload);
            printf("B --> (B_input) sending ack message to A\n");

            //printf("B---> %d %d",ackPacket.seqnum , ackPacket.acknum);
            tolayer1(1,ackPacket);

            if(acknum_B == 0){
                acknum_B = 1;
            }else{
                acknum_B = 0;
            }
            return ;

        }
        else
        {
            printf("B --> (B_input) packet recieved : %s\n",frame.payload);
            printf("B --> (B_input) sending packet to layer 5\n");

            printf("piggyback   BBBB\n\n");
            have_piggyback_frame_B = 1 ;

            //added
            if(acknum_B == 0){
                acknum_B = 1;
            }else{
                acknum_B = 0;
            }
            return;
        }

    }
    else if(frame.type == 1)
    {
        //printf("VVVVVVVVVV     B    VVVVVVVVVV       B        VVVVV\n\n");
        have_piggyback_frame_B = 1 ;
        printf("B --> (B_input) %d\n\n",frame.type);
        if(seqnum_B == frame.acknum ){
            stoptimer(1);
            ack_recieve_B = 1;
            printf("B --> (B_input) Ack recieved from B .\n");

            if(seqnum_B == 0){
                seqnum_B = 1 ;
            }else{
                seqnum_B = 0 ;
            }

        }
        else{
            printf("B --> (B_input) packet corrupted or got a nack \n");
        }
        return;

    }
    else if(frame.type == 2)
    {

        have_piggyback_frame_B = 1;
        if(frame.seqnum != acknum_B || seqnum_B != frame.acknum)
        {
            ackPacket.type = 1 ;

            printf("B --> (B_input) duplicate packet received or nack receive \n");
            if(acknum_B == 0){
                ackPacket.acknum = 1 ;
            }else{
                ackPacket.acknum = 0 ;
            }

            if(piggyback == 1){

                //ackPacket.acknum = acknum_B ; //..............................>risky
                printf("%d   %d\n",acknum_B,ackPacket.acknum);
            }

            ackPacket.checksum = calculate_crc_rem(ackPacket);
            //printf("B---> %d %d\n",ackPacket.seqnum , ackPacket.acknum);
            tolayer1(1,ackPacket);
            printf("B --> (B_input) nack is send to A \n");
            return ;
        }

        stoptimer(1);
        ack_recieve_B = 1;
        printf("B --> (B_input) Ack & data received from A ****************.\n");

        if(seqnum_B == 0){
            seqnum_B = 1 ;
        }else{
            seqnum_B = 0 ;
        }


        printf("B --> (B_input) packet recieved : %s\n",frame.payload);
        printf("B --> (B_input) sending packet to layer 5\n");
        tolayer3(1,frame.payload);


        //added
        if(acknum_B == 0){
            acknum_B = 1;
        }else{
            acknum_B = 0;
        }

        if(frame_count == 0){
            ackPacket.type = 1 ;

            if(acknum_B == 0){
                ackPacket.acknum = 1 ;
            }else{
                ackPacket.acknum = 0 ;
            }

            ackPacket.checksum = calculate_crc_rem(ackPacket);
            printf("B --> (B_input) sending ack to A\n");
            tolayer1(1,ackPacket);

            /*if(acknum_B == 0){
                acknum_B = 1;
            }else{
                acknum_B = 0;
            }*/
        }
        return;

    }


}

/* called when B's timer goes off */
void B_timerinterrupt(void)
{
    printf("B --> (B_timerinterrupt) B timer , retransmission \n");
    starttimer(1,TIMER_INCREMENT);
    tolayer1(1,packet_B_current);
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init(void)
{
    acknum_B = 0;
}

/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
    - emulates the tranmission and delivery (possibly with bit-level corruption
        and packet loss) of packets across the layer 3/4 interface
    - handles the starting/stopping of a timer, and generates timer
        interrupts (resulting in calling students timer handler).
    - generates message to be sent (passed from later 5 to 4)

THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you should have
to, and you defeinitely should not have to modify
******************************************************************/

struct event
{
    float evtime;       /* event time */
    int evtype;         /* event type code */
    int eventity;       /* entity where event occurs */
    struct frm *pktptr; /* ptr to packet (if any) assoc w/ this event */
    struct event *prev;
    struct event *next;
};
struct event *evlist = NULL; /* the event list */

/* possible events: */
#define TIMER_INTERRUPT 0
#define FROM_LAYER5 1
#define FROM_LAYER3 2

#define OFF 0
#define ON 1
#define A 0
#define B 1

int TRACE = 1;     /* for my debugging */
int nsim = 0;      /* number of messages from 5 to 4 so far */
int nsimmax = 0;   /* number of msgs to generate, then stop */
float time = 0.000;
float lossprob;    /* probability that a packet is dropped  */
float corruptprob; /* probability that one bit is packet is flipped */
float lambda;      /* arrival rate of messages from layer 5 */
int ntolayer3;     /* number sent into layer 3 */
int nlost;         /* number lost in media */
int ncorrupt;      /* number corrupted by media*/

void init();
void generate_next_arrival(void);
void insertevent(struct event *p);

int main()
{
    struct event *eventptr;
    struct pkt msg2give;
    struct frm pkt2give;

    int i, j;
    char c;

    init();
    A_init();
    B_init();

    while (1)
    {
        eventptr = evlist; /* get next event to simulate */
        if (eventptr == NULL)
            goto terminate;
        evlist = evlist->next; /* remove this event from event list */
        if (evlist != NULL)
            evlist->prev = NULL;
        if (TRACE >= 2)
        {
            printf("\nEVENT time: %f,", eventptr->evtime);
            printf("  type: %d", eventptr->evtype);
            if (eventptr->evtype == 0)
                printf(", timerinterrupt  ");
            else if (eventptr->evtype == 1)
                printf(", fromlayer5 ");
            else
                printf(", fromlayer3 ");
            printf(" entity: %d\n", eventptr->eventity);
        }
        time = eventptr->evtime; /* update time to next event time */
        if (eventptr->evtype == FROM_LAYER5)
        {
            if (nsim < nsimmax)
            {
                if (nsim + 1 < nsimmax)
                    generate_next_arrival(); /* set up future arrival */
                /* fill in msg to give with string of same letter */
                j = nsim % 26;
                for (i = 0; i < 20; i++)
                    msg2give.data[i] = 97 + j;
                msg2give.data[19] = 0;
                if (TRACE > 2)
                {
                    printf("          MAINLOOP: data given to student: ");
                    for (i = 0; i < 20; i++)
                        printf("%c", msg2give.data[i]);
                    printf("\n");
                }
                nsim++;
                if (eventptr->eventity == A)
                    A_output(msg2give);
                else
                    B_output(msg2give);
            }
        }
        else if (eventptr->evtype == FROM_LAYER3)
        {
            pkt2give.type = eventptr->pktptr->type;
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;
            for (i = 0; i < 20; i++)
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
            if (eventptr->eventity == A) /* deliver packet by calling */
                A_input(pkt2give); /* appropriate entity */
            else
                B_input(pkt2give);
            free(eventptr->pktptr); /* free the memory for packet */
        }
        else if (eventptr->evtype == TIMER_INTERRUPT)
        {
            if (eventptr->eventity == A)
                A_timerinterrupt();
            else
                B_timerinterrupt();
        }
        else
        {
            printf("INTERNAL PANIC: unknown event type \n");
        }
        free(eventptr);
    }

terminate:
    printf(
        " Simulator terminated at time %f\n after sending %d msgs from layer5\n",
        time, nsim);
}

void init() /* initialize the simulator */
{
    int i;
    float sum, avg;
    float jimsrand();

    printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");
    printf("CRC  : ");
    scanf("%d",&crc_show);
    printf("PiggyBack  : ");
    scanf("%d",&piggyback);
    printf("Generator polynomial  : ");
    scanf("%s",&gen);
    printf("Enter the number of messages to simulate: ");
    scanf("%d",&nsimmax);
    frame_count = nsimmax ;
    printf("Enter  packet loss probability [enter 0.0 for no loss]:");
    scanf("%f",&lossprob);
    printf("Enter packet corruption probability [0.0 for no corruption]:");
    scanf("%f",&corruptprob);
    printf("Enter average time between messages from sender's layer5 [ > 0.0]:");
    scanf("%f",&lambda);
    printf("Enter TRACE:");
    scanf("%d",&TRACE);

    srand(9999); /* init random number generator */
    sum = 0.0;   /* test random number generator for students */
    for (i = 0; i < 1000; i++)
        sum = sum + jimsrand(); /* jimsrand() should be uniform in [0,1] */
    avg = sum / 1000.0;
    if (avg < 0.25 || avg > 0.75)
    {
        printf("It is likely that random number generation on your machine\n");
        printf("is different from what this emulator expects.  Please take\n");
        printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
        exit(1);
    }

    ntolayer3 = 0;
    nlost = 0;
    ncorrupt = 0;

    time = 0.0;              /* initialize time to 0.0 */
    generate_next_arrival(); /* initialize event list */
}

/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand(void)
{
    double mmm = RAND_MAX;
    float x;                 /* individual students may need to change mmm */
    x = rand() / mmm;        /* x should be uniform in [0,1] */
    return (x);
}

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/

void generate_next_arrival(void)
{
    double x, log(), ceil();
    struct event *evptr;
    float ttime;
    int tempint;

    if (TRACE > 2)
        printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");

    x = lambda * jimsrand() * 2; /* x is uniform on [0,2*lambda] */
    /* having mean of lambda        */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtime = time + x;
    evptr->evtype = FROM_LAYER5;
    if (BIDIRECTIONAL && (jimsrand() > 0.5))
        evptr->eventity = B;
    else
        evptr->eventity = A;
    insertevent(evptr);
}

void insertevent(struct event *p)
{
    struct event *q, *qold;

    if (TRACE > 2)
    {
        printf("            INSERTEVENT: time is %lf\n", time);
        printf("            INSERTEVENT: future time will be %lf\n", p->evtime);
    }
    q = evlist;      /* q points to header of list in which p struct inserted */
    if (q == NULL)   /* list is empty */
    {
        evlist = p;
        p->next = NULL;
        p->prev = NULL;
    }
    else
    {
        for (qold = q; q != NULL && p->evtime > q->evtime; q = q->next)
            qold = q;
        if (q == NULL)   /* end of list */
        {
            qold->next = p;
            p->prev = qold;
            p->next = NULL;
        }
        else if (q == evlist)     /* front of list */
        {
            p->next = evlist;
            p->prev = NULL;
            p->next->prev = p;
            evlist = p;
        }
        else     /* middle of list */
        {
            p->next = q;
            p->prev = q->prev;
            q->prev->next = p;
            q->prev = p;
        }
    }
}

void printevlist(void)
{
    struct event *q;
    int i;
    printf("--------------\nEvent List Follows:\n");
    for (q = evlist; q != NULL; q = q->next)
    {
        printf("Event time: %f, type: %d entity: %d\n", q->evtime, q->evtype,
               q->eventity);
    }
    printf("--------------\n");
}

/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
void stoptimer(int AorB /* A or B is trying to stop timer */)
{
    struct event *q, *qold;

    if (TRACE > 2)
        printf("          STOP TIMER: stopping timer at %f\n", time);
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB))
        {
            /* remove this event */
            if (q->next == NULL && q->prev == NULL)
                evlist = NULL;          /* remove first and only event on list */
            else if (q->next == NULL) /* end of list - there is one in front */
                q->prev->next = NULL;
            else if (q == evlist)   /* front of list - there must be event after */
            {
                q->next->prev = NULL;
                evlist = q->next;
            }
            else     /* middle of list */
            {
                q->next->prev = q->prev;
                q->prev->next = q->next;
            }
            free(q);
            return;
        }
    printf("Warning: unable to cancel your timer. It wasn't running.\n");
}

void starttimer(int AorB /* A or B is trying to start timer */, float increment)
{
    struct event *q;
    struct event *evptr;

    if (TRACE > 2)
        printf("          START TIMER: starting timer at %f\n", time);
    /* be nice: check to see if timer is already started, if so, then  warn */
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB))
        {
            printf("Warning: attempt to start a timer that is already started\n");
            return;
        }

    /* create future event for when timer goes off */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtime = time + increment;
    evptr->evtype = TIMER_INTERRUPT;
    evptr->eventity = AorB;
    insertevent(evptr);
}

/************************** TOLAYER3 ***************/
void tolayer1(int AorB, struct frm packet)
{
    struct frm *mypktptr;
    struct event *evptr, *q;
    float lastime, x;
    int i;

    ntolayer3++;

    /* simulate losses: */
    if (jimsrand() < lossprob)
    {
        nlost++;
        if (TRACE > 0)
            printf("          TOLAYER1: packet being lost\n");
        return;
    }

    /* make a copy of the packet student just gave me since he/she may decide */
    /* to do something with the packet after we return back to him/her */
    mypktptr = (struct frm *)malloc(sizeof(struct frm));
    mypktptr->type = packet.type;
    mypktptr->seqnum = packet.seqnum;
    mypktptr->acknum = packet.acknum;
    mypktptr->checksum = packet.checksum;
    for (i = 0; i < 20; i++)
        mypktptr->payload[i] = packet.payload[i];
    if (TRACE > 2)
    {
        printf("          TOLAYER1: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
               mypktptr->acknum, mypktptr->checksum);
        for (i = 0; i < 20; i++)
            printf("%c", mypktptr->payload[i]);
        printf("\n");
    }

    /* create future event for arrival of packet at the other side */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtype = FROM_LAYER3;      /* packet will pop out from layer3 */
    evptr->eventity = (AorB + 1) % 2; /* event occurs at other entity */
    evptr->pktptr = mypktptr;         /* save ptr to my copy of packet */
    /* finally, compute the arrival time of packet at the other end.
       medium can not reorder, so make sure packet arrives between 1 and 10
       time units after the latest arrival time of packets
       currently in the medium on their way to the destination */
    lastime = time;
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == FROM_LAYER3 && q->eventity == evptr->eventity))
            lastime = q->evtime;
    evptr->evtime = lastime + 1 + 9 * jimsrand();

    /* simulate corruption: */
    if (jimsrand() < corruptprob)
    {
        ncorrupt++;
        if ((x = jimsrand()) < .75)
            mypktptr->payload[0] = 'Z'; /* corrupt payload */
        else if (x < .875)
            mypktptr->seqnum = 999999;
        else
            mypktptr->acknum = 999999;
        if (TRACE > 0)
            printf("          TOLAYER1: packet being corrupted\n");
    }

    if (TRACE > 2)
        printf("          TOLAYER1: scheduling arrival on other side\n");
    insertevent(evptr);
}

void tolayer3(int AorB, char datasent[20])
{
    int i;
    if (TRACE > 2)
    {
        printf("          TOLAYER3: data received: ");
        for (i = 0; i < 20; i++)
            printf("%c", datasent[i]);
        printf("\n");
    }
}
