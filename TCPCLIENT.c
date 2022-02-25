#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/msg.h>
#include<errno.h>

#define TIMEOUT 10

static void handler(int sig)
{
    printf("TimeOut\n");
    exit(-1);
}

void setTimeout()
{
    signal(SIGALRM,handler);
    alarm(TIMEOUT);
}

void cancelTimeout()
{
    alarm(0);
}


struct msgbuf{
    long lMtype;
    u_char cMessage[300];
};

int main(void)
{
    int i,j;

    //Create Msq
    key_t keyTcpMq=0x66662576;
    //收MSGHDL的電文
    int iTcpMqid;
    if((iTcpMqid=msgget(keyTcpMq,IPC_CREAT|0666))==-1)
    {
        perror("Msgget Error ");
        return -1;
    }
    printf("TcpMqid: %d\n", iTcpMqid);
    //收到SERVER後送給MSGHDL的電文Queue
    key_t keyMsgMq=0x77772576;
    int iMsgMqid;
    if((iMsgMqid=msgget(keyMsgMq,IPC_CREAT|0666))==-1)
    {
        perror("Msgget Error");
        return -1;
    }
    printf("MsgMqid : %d\n", iMsgMqid);


    //Set Socket
    int iSockfd;
    if((iSockfd=socket(AF_INET,SOCK_STREAM,0))==1)
    {
        perror("Socket Error");
        return -1;
    }

    //Set Info
    struct sockaddr_in sInfo;
    bzero(&sInfo,sizeof(sInfo));
    sInfo.sin_family=AF_INET;
    sInfo.sin_addr.s_addr=inet_addr("192.168.16.146");
    sInfo.sin_port=htons(52001);

    //connect
    if(connect(iSockfd,(struct sockaddr*)&sInfo, sizeof(sInfo))==-1)
    {
        perror("Connect Error");
        return -1;
    }
    printf("Server Connect!\n");

    //Receive from MSGHDL
    struct msgbuf sRecv;
    //收MSGHDL寫好的電文msgqueue
    msgrcv(iTcpMqid,&sRecv,sizeof(sRecv),1,0);
    //印出長度大小(16進制，放在陣列前兩位)
    printf("%x %x %c\n",sRecv.cMessage[0],sRecv.cMessage[1],sRecv.cMessage[2]);
    //將16進制長度轉成10進制裝進iSendsize(第一個陣列左移8位+)
    int iSendsize = (sRecv.cMessage[0]<<8)+sRecv.cMessage[1];
    //
    printf("Send len: %d\n",iSendsize);
    i=2;
    printf("Send : ");
    while(i<iSendsize+2)
    {
        printf("%c",sRecv.cMessage[i]);
        i++;
    }
    printf("\n");

    
    //Send to Server
    send(iSockfd,sRecv.cMessage,sizeof(sRecv.cMessage),0);

    //Receive from Server
    while(1)
    {
        u_char cRecvbuf[500];
        int iRet;
        setTimeout();
        iRet=recv(iSockfd,cRecvbuf,sizeof(cRecvbuf),0);
        printf("Receive New Line!\n");
        if(iRet==0)
        {
            printf("Connect Shutdown\n");
            break;
        }
        else if(iRet==-1)
        {
            printf("error no : %s\n",strerror(errno));
            break;
        }
        cancelTimeout();
        int iRecvsize=cRecvbuf[0]<<8;
        //cRecvbuf[0]='0';
        j=0;
        int iReadlen=0;
        if(cRecvbuf[1]!='\0')
        {
            iRecvsize=iRecvsize+cRecvbuf[1];
            //cRecvbuf[1]='0';
            i=2;
        }
        else
        {
            setTimeout();
            iRet=recv(iSockfd,cRecvbuf,sizeof(cRecvbuf),0);
            if(iRet==0)
            {
                printf("Connect Shutdown\n");
                break;
            }
            else if(iRet==-1)
            {
                printf("error no : %s\n",strerror(errno));
                break;
            }
            cancelTimeout();
            iRecvsize=iRecvsize+cRecvbuf[0];
            //cRecvbuf[0]='0';
            i=1;
        }
        //printf("iRecvsize = %d\n",iRecvsize);
        //printf("msgsize = %d\n",strlen(cRecvbuf));
        //printf("Undecode : %s\n",cRecvbuf);


        struct msgbuf sSend;
        iReadlen-=i; //len without len byte
        while(1)
        {
            if(cRecvbuf[i]=='\0' || (iReadlen+i)==iRecvsize)
            {
                iReadlen+=i;
                if(iReadlen==iRecvsize)
                    break;
                else
                {
                    setTimeout();
                    iRet=recv(iSockfd,cRecvbuf,sizeof(cRecvbuf),0);
                    if(iRet==0)
                    {
                        printf("Connect Shutdown\n");
                        break;
                    }
                    else if(iRet==-1)
                    {
                        printf("error no : %s\n",strerror(errno));
                        break;
                    }
                    cancelTimeout();
                    i=0;
                }
            }
            else
            {
                sSend.cMessage[j]=cRecvbuf[i];
                i++;
                j++;
            }
        }
        sSend.cMessage[j]='\0';
        printf("Recv len : %d\n",iRecvsize);
        printf("Recv : %s\n\n",sSend.cMessage);

        // if(sSend.cMessage[12]=='0' && sSend.cMessage[13]=='2' && sSend.cMessage[14]=='1' && sSend.cMessage[15]=='0')
        if(memcmp((sSend.cMessage+12), "0210", 4)==0);
        {
            sSend.lMtype=2;
            msgsnd(iMsgMqid,&sSend,sizeof(sSend),0);
            break;
        }
    }
    cancelTimeout();
    close(iSockfd);
    printf("Socket Close\n");
    return 0;
}
