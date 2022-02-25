#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include <time.h>
#include <signal.h>

#define TIMEOUT 20
//超過二十秒沒有回應的話放棄連線
static void handler(int sig)
{
    printf("TimeOut\n");
    exit(-1);
}

void setTimeout()
{
    signal(SIGALRM, handler);
    alarm(TIMEOUT);
}

void cancelTimeout()
{
    alarm(1);
}
/*msgrcv函式需要的結構
1. 訊息種類
2. 訊息資料*/
struct msgbuf
{
    long lMtype;
    u_char cMessage[301];
};
void addBitmap(char*, int, int, char*);
char* decode(char*, int, int);
int charToInt(char);
int BCDtoInt(char*, int, int);

int main(void)
{
    key_t keyTcpMq = 0x66662576;
    int iTcpMqid;
    if ((iTcpMqid = msgget(keyTcpMq, IPC_CREAT | 0600)) == -1)
    //如果TCPCLIENT成功接收訊息就為佇列識別代號，否則傳回-1錯誤
    {
        perror(" TCP Msgget Error ");
        return -1;
    }
    printf("TcpMqid: %d\n", iTcpMqid);
    key_t keyMsgMq = 0x77772576;
    int iMsgMqid;
    //如果MSGMQ成功接收訊息就為佇列識別代號，否則傳回-1錯誤
    //權限先設自己可讀寫即可
    if ((iMsgMqid = msgget(keyMsgMq, IPC_CREAT | 0600)) == -1)
    {
        perror("Msgget Error");
        return -1;
    }
    printf("MsgMqid: %d\n", iMsgMqid);

    struct msgbuf sRev;
    /*msgrcv的函示宣告
     int msgrcv(int msqid, struct msgbuf *msgp, int msgsz, long msgtyp, int msgflg);
     引數msgtyp是用來指定所要讀取的訊息種類:
     msgtyp = 0傳回佇列內第一筆訊息。
     msgtyp > 0 傳回佇列內第一筆msgtyp與mtype相同的訊息。
     msgtyp < 0傳回佇列內第一筆mtype小於或等於	msgtyp絕對值的訊息。
     2表示為最新的訊息*/
    while (msgrcv(iMsgMqid, &sRev, sizeof(sRev), 2, IPC_NOWAIT) != -1)
    {
        printf("Last Message!\n");
    }
    /*先宣告cbuffer電文長度300，因為還不知道要傳多少字元，先定義空間長度三百*/
    u_char cbuffer[300] = "";
    u_char ctemp[50];
    /*開始組電文Header字與一些固定欄位
    sprintf 為複製字串到cbuffer,因第一個訊息為固定Header所以直接複製*/
    sprintf(cbuffer, "ISO026000060");
    /*將0200串到cbuffer字串之後*/
    strcat(cbuffer, "0200");
    /*取得cbuffer目前的長度，可以繼續放電文訊息*/
    int iHMsize = strlen(cbuffer);
    /*第二個欄位為bitvalue,先放好16個0，因為此欄位之後要轉化為bitvalue*/
    strcat(cbuffer, "0000000000000000");
    /*欄位三為processing code,這邊用000000 Purchase, Default account, Default account表示*/
    addBitmap(cbuffer, iHMsize, 3, "000000");
    /*金額，這邊為交易一百塊，後面兩個小數點為零*/
    addBitmap(cbuffer, iHMsize, 4, "000000010000");
    /*取得時間欄位，這邊有可能每一次抓一次時間可能會造成系統時間上不同步的問題*/
    time_t timep;
    struct tm *sTm;
    time(&timep);
    sTm = localtime(&timep);
    /*時間格式，格式為MMDDHHMMSS*
    1 + 月 日 時 分 秒, 1+ 是因為月份 顯示出來為 0110232646 為時間格式，這邊要做修改*/
    sprintf(ctemp, "%02d%02d%02d%02d%02d", 1 + sTm->tm_mon, sTm->tm_mday, sTm->tm_hour, sTm->tm_min, sTm->tm_sec);
    addBitmap(cbuffer, iHMsize, 7, ctemp);
    addBitmap(cbuffer, iHMsize, 11, "123321");

    time(&timep);
    sTm = localtime(&timep);
    sprintf(ctemp, "%02d%02d%02d", sTm->tm_hour, sTm->tm_min, sTm->tm_sec);
    addBitmap(cbuffer, iHMsize, 12, ctemp);
    sprintf(ctemp, "%02d%02d", 1 + sTm->tm_mon, sTm->tm_mday);
    addBitmap(cbuffer, iHMsize, 13, ctemp);

    time(&timep);
    sTm = localtime(&timep);
    sprintf(ctemp, "%02d%02d", 1 + sTm->tm_mon, sTm->tm_mday);
    addBitmap(cbuffer, iHMsize, 17, ctemp);
    addBitmap(cbuffer, iHMsize, 18, "4512");
    addBitmap(cbuffer, iHMsize, 22, "810");
    addBitmap(cbuffer, iHMsize, 25, "08");
    addBitmap(cbuffer, iHMsize, 32, "0887654321");
    addBitmap(cbuffer, iHMsize, 35, "20374245001751006=2112");
    addBitmap(cbuffer, iHMsize, 37, "654321123456");
    addBitmap(cbuffer, iHMsize, 41, "0123456789ABCDEF");
    addBitmap(cbuffer, iHMsize, 43, "1234567890123456789012TPE6789012345TWTW");
    addBitmap(cbuffer, iHMsize, 48, "0274567890123456789012          ");
    addBitmap(cbuffer, iHMsize, 49, "901");
    addBitmap(cbuffer, iHMsize, 61, "019BK77PRO200000000000");
    addBitmap(cbuffer, iHMsize, 63, "048& 0000200048! C000026 1881 5   1  ");

    printf("%s\n", cbuffer);

    struct msgbuf sSend;
    sSend.lMtype = 1;
    int iBufsize = strlen(cbuffer);
    printf("%d\n", iBufsize);
    sprintf(sSend.cMessage, "00%s", cbuffer);
    sSend.cMessage[0] = (iBufsize >> 8) & 0xff;
    sSend.cMessage[1] = iBufsize & 0xff;

    if (msgsnd(iTcpMqid, &sSend, sizeof(sSend), 0) == -1)
    {
        perror("Send to TcpMQ Error");
        return -1;
    }

    setTimeout();
    /*If msgtyp is equal to zero, the first message on the queue is received.
    If msgtyp is greater than 0, the first message of type, msgtyp, is received.
    If msgtyp is less than 0, the first message of the lowest type that is less than or equal to 
    the absolute value of msgtyp is received.
    
    The msgflg argument can be set to 0 (ignored), or:IPC_NOWAIT.*/
    msgrcv(iMsgMqid, &sRev, sizeof(sRev), 2, 0);
    cancelTimeout();
    printf("MSGHDL sRev: %s \n", sRev.cMessage);

    int iRevSize = strlen(sRev.cMessage);
    printf("Receive size:%d\n", iRevSize);

    int iProcess = 0;

    printf("Header:%s\n", decode(sRev.cMessage, iProcess, 12));
    iProcess += 12;

    printf("MTI: %s\n", decode(sRev.cMessage, iProcess, 4));
    iProcess += 4;

    int iBitNow = iProcess;
    int iBitValue;
    int iSecond = 0;
    int iMsgLen;

    //1~4
    iBitValue = charToInt(sRev.cMessage[iBitNow]);
    iBitNow++;
    if ((iBitValue & 8))
    {
        printf("Bitmap: %s\n", decode(sRev.cMessage, iProcess, 32));
        iProcess += 32;
        iSecond = 1;
    }
    else
    {
        printf("Bitmap: %s\n", decode(sRev.cMessage, iProcess, 16));
        iProcess += 16;
    }
    if ((iBitValue & 4))
    {
        iMsgLen = BCDtoInt(sRev.cMessage, iProcess, 2);
        iProcess += 2;
        printf("P-2 Primary Account Number: %s\n", decode(sRev.cMessage, iProcess, iMsgLen));
        iProcess += iMsgLen;
    }
    if ((iBitValue & 2))
    {
        printf("P-3 Processing Code: %s\n", decode(sRev.cMessage, iProcess, 6));
        iProcess += 6;
    }
    if ((iBitValue & 1))
    {
        printf("P-4 Transaction Amount: %s\n", decode(sRev.cMessage, iProcess, 12));
        iProcess += 12;
    }

    //5~8
    iBitValue = charToInt(sRev.cMessage[iBitNow]);
    iBitNow++;
    if ((iBitValue & 2))
    {
        printf("P-7 Transmission Date and Time: %s\n", decode(sRev.cMessage, iProcess, 10));
        iProcess += 10;
    }

    //9~12
    iBitValue = charToInt(sRev.cMessage[iBitNow]);
    iBitNow++;
    if ((iBitValue & 2))
    {
        printf("P-11 System Trace Audit Number: %s\n", decode(sRev.cMessage, iProcess, 6));
        iProcess += 6;
    }
    if ((iBitValue & 1))
    {
        printf("P-12 Local Transaction Time %s\n", decode(sRev.cMessage, iProcess, 6));
        iProcess += 6;
    }
    //13~16
    iBitValue = charToInt(sRev.cMessage[iBitNow]);
    iBitNow++;
    if ((iBitValue & 8))
    {
        printf("P-13 Local Transaction Date: %s\n", decode(sRev.cMessage, iProcess, 4));
        iProcess += 4;
    }
    if ((iBitValue & 4))
    {
        printf("P-14 Expiration Date: %s\n", decode(sRev.cMessage, iProcess, 4));
        iProcess += 4;
    }
    if ((iBitValue & 2))
    {
        printf("P-14 Settlement Date:%s\n", decode(sRev.cMessage, iProcess, 4));
        iProcess += 4;
    }
    if ((iBitValue & 2))
    {
        printf("P-15 Settlement Date: %s\n", decode(sRev.cMessage, iProcess, 4));
        iProcess += 4;
    }
    //17~20
    iBitValue = charToInt(sRev.cMessage[iBitNow]);
    iBitNow++;
    if ((iBitValue & 8))
    {
        printf("P-17 Capture Date: %s\n", decode(sRev.cMessage, iProcess, 4));
        iProcess += 4;
    }
    if ((iBitValue & 4))
    {
        printf("P-18 Merchant Category Code: %s\n", decode(sRev.cMessage, iProcess, 4));
        iProcess += 4;
    }
    //21~24
    iBitValue = charToInt(sRev.cMessage[iBitNow]);
    iBitNow++;
    if ((iBitValue & 4))
    {
        printf("P-22 Point of Service Entry Mode: %s\n", decode(sRev.cMessage, iProcess, 3));
        iProcess += 3;
    }
    //25~28
    iBitValue = charToInt(sRev.cMessage[iBitNow]);
    iBitNow++;
    if ((iBitValue & 8))
    {
        printf("P-25 Point of Service Condition Mode: %s\n", decode(sRev.cMessage, iProcess, 2));
        iProcess += 2;
    }
    if ((iBitValue & 1))
    {
        printf("P-28 Transaction Fee Amount: %s\n", decode(sRev.cMessage, iProcess, 9));
        iProcess += 9;
    }
    //29~32
    iBitValue = charToInt(sRev.cMessage[iBitNow]);
    iBitNow++;
    if ((iBitValue & 1))
    {
        iMsgLen = BCDtoInt(sRev.cMessage, iProcess, 2);
        iProcess += 2;
        printf("P-32 Acquiring Institution Identification Code:%s\n", decode(sRev.cMessage, iProcess, iMsgLen));
        iProcess += iMsgLen;
    }
    //33~36
    iBitValue = charToInt(sRev.cMessage[iBitNow]);
    iBitNow++;
    if ((iBitValue & 2))
    {
        iMsgLen = BCDtoInt(sRev.cMessage, iProcess, 2);
        iProcess += 2;
        printf("P-35 Track 2 Data: %s\n", decode(sRev.cMessage, iProcess, iMsgLen));
        iProcess += iMsgLen;
    }

    //37~40
    iBitValue = charToInt(sRev.cMessage[iBitNow]);
    iBitNow++;
    if ((iBitNow & 8))
    {
        printf("P-37 Retrival Reference Number: %s\n", decode(sRev.cMessage, iProcess, 12));
        iProcess += 12;
    }
    if ((iBitValue & 4))
    {
        printf("P-38 Approve Code: %s\n", decode(sRev.cMessage, iProcess, 6));
        iProcess += 6;
    }
    if ((iBitValue & 2))
    {
        printf("P-39 Response Code: %s\n", decode(sRev.cMessage, iProcess, 2));
        iProcess += 2;
    }

    //41~44
    iBitValue = charToInt(sRev.cMessage[iBitNow]);
    iBitNow++;
    if ((iBitValue & 8))
    {
        printf("P-41 Card Acceptor Terminal Identification: %s\n", decode(sRev.cMessage, iProcess, 16));
        iProcess += 16;
    }
    if ((iBitValue & 4))
    {
        printf("P-42 Card Acceptor Identification Code: %s\n", decode(sRev.cMessage, iProcess, 15));
        iProcess += 15;
    }
    if ((iBitValue & 2))
    {
        printf("P-43 Card Acceptor Name/Location: %s\n", decode(sRev.cMessage, iProcess, 40));
        iProcess += 40;
    }
    if ((iBitValue & 1))
    {
        printf("P-44 Additional Response Data: %s\n", decode(sRev.cMessage, iProcess, 27));
        iProcess += 27;
    }
    //45~48
    iBitValue = charToInt(sRev.cMessage[iBitNow]);
    iBitNow++;
    if ((iBitValue & 1))
    {
        iMsgLen = BCDtoInt(sRev.cMessage, iProcess, 3);
        iProcess += 3;
        printf("P-48 Additional data: %s\n", decode(sRev.cMessage, iProcess, iMsgLen));
        iProcess += iMsgLen;
    }
    //49~52
    iBitValue = charToInt(sRev.cMessage[iBitNow]);
    iBitNow++;
    if ((iBitValue & 8))
    {
        printf("P-49 Transaction Currency Code: %s\n", decode(sRev.cMessage, iProcess, 3));
        iProcess += 3;
    }
    if ((iBitValue & 1))
    {
        printf("P-52 Personal Identification Number (PIN) Data:%s\n", decode(sRev.cMessage, iProcess, 16));
        iProcess += 16;
    }

    //53~56
    iBitValue = charToInt(sRev.cMessage[iBitNow]);
    iBitNow++;
    if ((iBitValue & 8))
    {
        printf("P-53 Security Related Control Information: %s\n", decode(sRev.cMessage, iProcess, 16));
        iProcess += 16;
    }
    //57~60
    iBitValue = charToInt(sRev.cMessage[iBitNow]);
    iBitNow++;
    if ((iBitValue & 1))
    {
        iMsgLen = BCDtoInt(sRev.cMessage, iProcess, 3);
        iProcess += 3;
        printf("P-60 POS terminal data/From Host Maintenance (FHM) Data: %s\n", decode(sRev.cMessage, iProcess, iMsgLen));
        iProcess += iMsgLen;
    }

    //61~64
    iBitNow = charToInt(sRev.cMessage[iBitNow]);
    iBitNow++;
    if ((iBitValue & 8))
    {
        iMsgLen = BCDtoInt(sRev.cMessage, iProcess, 3);
        iProcess += 3;
        printf("P-61 Card Issure-Category-Response Code: %s\n", decode(sRev.cMessage, iProcess, iMsgLen));
        iProcess += iMsgLen;
    }
    if ((iBitNow & 4))
    {
        printf("P-62 Postal Code: %s\n", decode(sRev.cMessage, iProcess, 13));
        iProcess += 13;
    }
    if ((iBitValue & 2))
    {
        iMsgLen = BCDtoInt(sRev.cMessage, iProcess, 3);
        iProcess += 3;
        //&^
        iProcess += 2;

        int iTokenNum = BCDtoInt(sRev.cMessage, iProcess, 5);
        int iTokenNow = 1;
        iProcess += 5;
        printf("Token num: %d, Token Len: %d\n", iTokenNum, BCDtoInt(sRev.cMessage, iProcess, 5));
        iProcess += 5;
        while (iTokenNow < iTokenNum)
        {
            //!^
            iProcess += 2;
            char *cTokenID = malloc(3);
            cTokenID = decode(sRev.cMessage, iProcess, 2);
            iProcess += 2;
            int iNowTokenLen = BCDtoInt(sRev.cMessage, iProcess, 5);
            iProcess += 5;

            //^
            iProcess++;
            printf("Token ID: %s, Token Len: %d, Message: %s\n", cTokenID, iNowTokenLen, decode(sRev.cMessage, iProcess, iNowTokenLen));
            iProcess += iNowTokenLen;

            iTokenNow++;
        }
        // printf("P-63 POS Additional Data: %s\n",decode(sRev.cMessage,iProcess,iMsgLen));
        // iProcess+=iMsgLen;
    }

    if (iSecond == 1)
    {
        //65~68
        iBitValue = charToInt(sRev.cMessage[iBitNow]);
        iBitNow++;
        if ((iBitValue & 4))
        {
            printf("S-66 Settlement Code: %s\n", decode(sRev.cMessage, iProcess, 1));
            iProcess++;
        }
        //69~72
        iBitValue = charToInt(sRev.cMessage[iBitNow]);
        iBitNow++;
        if ((iBitValue & 4))
        {
            printf("S-70 Network Management Information Code: %s\n", decode(sRev.cMessage, iProcess, 3));
            iProcess += 3;
        }
        //73~76
        iBitValue = charToInt(sRev.cMessage[iBitNow]);
        iBitNow++;
        if ((iBitValue & 8))
        {
            printf("S-73 Action Date: %s\n", decode(sRev.cMessage, iProcess, 6));
            iProcess += 6;
        }
        if ((iBitValue & 4))
        {
            printf("S-76 Number Debits: %s\n", decode(sRev.cMessage, iProcess, 10));
            iProcess += 10;
        }

        //77~80
        iBitValue = charToInt(sRev.cMessage[iBitNow]);
        iBitNow++;
        if ((iBitValue & 8))
        {
            printf("S-77 Reversal Number Debits: %s\n", decode(sRev.cMessage, iProcess, 10));
            iProcess += 10;
        }
        if ((iBitValue & 4))
        {
            printf("S-79 Reversal Number Transfer: %s\n", decode(sRev.cMessage, iProcess, 10));
            iProcess += 10;
        }

        //81~84
        iBitValue = charToInt(sRev.cMessage[iBitNow]);
        iBitNow++;
        if ((iBitValue & 8))
        {
            printf("S-81 Number Authorizations: %s\n", decode(sRev.cMessage, iProcess, 10));
            iProcess += 10;
        }
        //85~88
        iBitValue = charToInt(sRev.cMessage[iBitNow]);
        iBitNow++;
        if ((iBitValue & 4))
        {
            printf("S-86 Amount Credits: %s\n", decode(sRev.cMessage, iProcess, 16));
            iProcess += 16;
        }
        if ((iBitValue & 2))
        {
            printf("S-87 Reversal Amount Credits: %s\n", decode(sRev.cMessage, iProcess, 16));
            iProcess += 16;
        }
        if ((iBitValue & 1))
        {
            printf("S-88 Amount Debits: %s\n", decode(sRev.cMessage, iProcess, 16));
            iProcess += 16;
        }

        //89~92
        iBitValue = charToInt(sRev.cMessage[iBitNow]);
        iBitNow++;
        if ((iBitValue & 8))
        {
            printf("S-89 Reversal Amount Debits: %s\n", decode(sRev.cMessage, iProcess, 16));
            iProcess += 16;
        }
        if ((iBitValue & 4))
        {
            printf("S-90 Original Data Elements: %s\n", decode(sRev.cMessage, iProcess, 42));
            iProcess += 42;
        }
        if ((iBitValue & 2))
        {
            printf("S-91 File Update Code: %s\n", decode(sRev.cMessage, iProcess, 1));
            iProcess++;
        }

        //93~96
        iBitValue = charToInt(sRev.cMessage[iBitNow]);
        iBitNow++;
        if ((iBitValue & 2))
        {
            printf("S-95 Replacement Amount: %s\n", decode(sRev.cMessage, iProcess, 42));
            iProcess += 42;
        }

        //97~100
        iBitValue = charToInt(sRev.cMessage[iBitNow]);
        iBitNow++;
        if ((iBitValue & 8))
        {
            printf("S-97 Net Settlement Credit (or Debit) Amount: %s\n", decode(sRev.cMessage, iProcess, 17));
            iProcess += 17;
        }
        if ((iBitValue & 2))
        {
            iMsgLen = BCDtoInt(sRev.cMessage, iProcess, 2);
            iProcess += 2;
            printf("S-99 Settlement Institution Identification Code: %s\n", decode(sRev.cMessage, iProcess, iMsgLen));
            iProcess += iMsgLen;
        }

        //101~104
        iBitValue = charToInt(sRev.cMessage[iBitNow]);
        iBitNow++;
        if ((iBitValue & 8))
        {
            iMsgLen = BCDtoInt(sRev.cMessage, iProcess, 2);
            iProcess += 2;
            printf("S-101 File Name: %s\n", decode(sRev.cMessage, iProcess, iMsgLen));
        }
    }
    return 0;
}

void addBitmap(char *dst, int iHMsize, int P, char *sour)
{
    int bit = (P - 1) / 4;
    int bitOne = 1 << (4 - (P - (4 * bit)));
    int iNP;
    if (dst[iHMsize + bit] >= '0' && dst[iHMsize + bit] <= '9')
        iNP = ((dst[iHMsize + bit] - 'A' + 10) | bitOne);
    //printf("%d%d%d\n",P,bitOne,iNP);
    if (iNP > 9)
    {
        dst[iHMsize + bit] = iNP - 10 + 'A';
    }
    else
    {
        dst[iHMsize + bit] = iNP + '0';
    }
    if (P == 1)
    {
        strcat(dst, "0000000000000000");
        return;
    }
    else
    {
        strcat(dst, sour);
        return;
    }
}
char *decode(char *sRev, int iNow, int iLen)
{
    char *temp = malloc(iLen + 1);
    int i = 0;
    while (i < iLen)
    {
        temp[i] = sRev[i + iNow];
        i++;
    }
    temp[i] = "\0";
    iNow += iLen;
    return temp;
}
int charToInt(char hex)
{
    if (hex >= '0' && hex <= '9')
        return hex - '0';
    else
        return hex - 'A' + 10;
}
int BCDtoInt(char *sRev, int iNow, int iLen)
{
    int temp = 0;
    int i = 0;
    while (i < iLen)
    {
        temp = temp * 10 + sRev[i + iNow] - '0';
        i++;
    }
    iNow += iLen;
    return temp;
}
