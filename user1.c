#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>

#define MAX_LISTEN 10

struct message
{
    int target_id;
    char buf[100];
};

//Message Warehouse
struct canku
{
    struct message recv_message[10];
    int read_pos,write_pos;
}messages;

//messages Initialization
void init()
{
    messages.read_pos=0;
    messages.write_pos=0;
}

//messages Destroy
void finish()
{
    messages.read_pos=0;
    messages.write_pos=0;
}

int sd;

int main()
{
    init();
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
    memset(server_ip.sin_zero,0,8);

    err=bind(sd,(struct sockaddr *)(&server_ip),sizeof(struct sockaddr));
    if(err==-1)
    {
        printf("bind failed\n");
        close(sd);
        return -1;
    }

    err=listen(sd,MAX_LISTEN);
    if(err==-1)
    {
        printf("listen failed\n");
        close(sd);
        return -1;
    }

    int length=sizeof(customer_ip);

    //Initialize shared variables,Size equals canku by luke
    int shmid=shmget(IPC_PRIVATE,sizeof(struct canku),IPC_CREAT|0777);
    if(shmid<0)
    {
        printf("shmget failed\n");
        return -1;;
    }

    struct canku * messages_ptr=&messages;

    while(1)
    {
        int temp_cd=accept(sd,(struct sockaddr *)(&customer_ip),&length);
        if(temp_cd==-1)
        {
            printf("accept failed,ereno: %d\n",temp_cd);
            close(sd);
            return -1;
        }

        printf("user %d online\n",temp_cd);

        
        pid_t pid=fork();
        if(pid==0)//Subprocess by luke
        {
            while(1)
            {
                messages_ptr=shmat(shmid,NULL,0);
                if((messages_ptr->write_pos+1)%10==messages_ptr->read_pos)
                {
                   shmdt(messages_ptr);
                   continue;
                }

                struct message temp_message;
                err=recv(temp_cd,&temp_message,sizeof(struct message),0);
                //err=read(temp_cd,&(recv_message[count-1]),sizeof(struct message));
                if(err!=-1)
                {
                    messages_ptr->recv_message[messages_ptr->write_pos].target_id=temp_message.target_id;
                    strcpy(messages_ptr->recv_message[messages_ptr->write_pos].buf,temp_message.buf);
                    printf("recv: read_pos: %d, write_pos: %d, target_id: %d, buf: %s\n",messages_ptr->read_pos,messages_ptr->write_pos+1,messages_ptr->recv_message[messages_ptr->write_pos].target_id,messages_ptr->recv_message[messages_ptr->write_pos].buf);
                    messages_ptr->write_pos++;
                    if(messages_ptr->write_pos==9)
                        messages_ptr->write_pos=0;
                }

                shmdt(messages_ptr);
            }
        }
        
        //Prevent the main thread from being while Live, unable to accept new connection request.
        pid_t pid1=fork();
        if(pid1==0)
        {
            while(1)
            {
                messages_ptr=shmat(shmid,NULL,0);
               if((messages_ptr->read_pos)%10==messages_ptr->write_pos)
               {
                   //printf("buff is empty\n");
                   shmdt(messages_ptr);
                   continue;
               }
                
                //strcpy(messages_ptr->recv_message[messages_ptr->read_pos].buf,"hello");
                err=send(messages_ptr->recv_message[messages_ptr->read_pos].target_id,messages_ptr->recv_message[messages_ptr->read_pos].buf,100,0);
                if(err==-1)
                {
                    //printf("send failed\n");
                }
                else
                {
                    printf("send: read_pos: %d, write_pos: %d ,message.target_id: %d, message.buf: %s\n",messages_ptr->read_pos+1,messages_ptr->write_pos,messages_ptr->recv_message[messages_ptr->read_pos].target_id,messages_ptr->recv_message[messages_ptr->read_pos].buf);
                    messages_ptr->read_pos++;
                    if(messages_ptr->read_pos==9)
                        messages_ptr->read_pos=0;
                }
                shmdt(messages_ptr);
            }
        }
    }

    close(sd);
    shmctl(shmid,IPC_RMID,NULL);
    finish();
    return 0;
}