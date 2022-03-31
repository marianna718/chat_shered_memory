#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct message
{
    int target_id;
    char buf[100];
};

int sd;
struct message send_message;

void * read_message(void * argv)
{
    while(1)
    {
        //Read messages from server
        char revBuf[100];
        read(sd,revBuf,100);
        printf("recevice from server: %s",revBuf);
    }
}

void * write_message(void * argv)
{
    while(1)
    {
        printf("input message: \n");
        memset(send_message.buf,0,128);
        send_message.target_id=-1;
        scanf("%d %s",&send_message.target_id,send_message.buf);

        write(sd,&send_message,sizeof(send_message));
        sleep(3);
    }
}

int main()
{
    struct sockaddr_in server_ip,customer_ip;
    int err;

    sd=socket(AF_INET,SOCK_STREAM,0);
    if(sd==-1)
    {
        printf("socket failed\n");
        close(sd);
        return -1;
    }

    //server_ip Initialization
    server_ip.sin_family=AF_INET;
    server_ip.sin_port=htons(5678);
    server_ip.sin_addr.s_addr=htonl(INADDR_ANY);
    //err=inet_aton("115.157.201.179",&server_ip.sin_addr.s_addr);
    memset(server_ip.sin_zero,0,8);

    err=connect(sd,(struct sockaddr *)(&server_ip),sizeof(server_ip));
    if(err==-1)
    {
        printf("connect failed\n");
        close(sd);
        return -1;
    }

    pid_t pid=fork();
    if(pid==0)
    {
        while(1)
        {
            //Read messages from server
            //printf("read message: \n");
            char revBuf[100];
            recv(sd,revBuf,100,0);
            //read(sd,revBuf,100);
            printf("recevice from server: %s\n",revBuf);
        }
    }
    while(1)
    {
        printf("input message: \n");
        memset(send_message.buf,0,128);
        send_message.target_id=-1;
        scanf("%d %s",&send_message.target_id,send_message.buf);

        if(send_message.target_id!=-1&&(strcmp(send_message.buf,"")!=0))
        {

            err=send(sd,&send_message,sizeof(send_message),0);
            if(err==-1)
            {
                printf("send failed\n");
            }
            //write(sd,&send_message,sizeof(send_message));

            send_message.target_id=-1;
            memset(send_message.buf,0,sizeof(send_message.buf));
        }

        sleep(3);
    }

    close(sd);
    return 0;
}