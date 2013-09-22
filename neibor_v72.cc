#include  <unistd.h>
#include  <sys/types.h>       /* basic system data types */
#include  <sys/socket.h>      /* basic socket definitions */
#include  <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
#include  <arpa/inet.h>       /* inet(3) functions */
#include <netdb.h> /*gethostbyname function */
#include <iostream>
#include <cstdlib>
#include <errno.h>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <time.h>
#include <vector>
#include <pthread.h>

#define BUFF_SIZE   256
#define MAXCONN     100
using namespace std;
char buffer[BUFF_SIZE];

struct clove_rel{
int seq_number;
int pre_port;
int next_port;
string pre_IP;
string next_IP;
string filename;//the filename refer to two file filename.M and filename.key
        clove_rel(){
            seq_number=0;
            pre_port = 18885;
            next_port = 18886;
            pre_IP = "127.0.0.1";
            next_IP = "127.0.0.1";
            filename="0";
        }
};

vector< struct clove_rel > cloves_vector;

struct proxy_rel{
    int id;
    int neib1_port;
    string neib1_IP;
    int neib2_port;
    string neib2_IP;
    proxy_rel(){
        id = 0;
        neib1_port = 18885;
        neib1_IP = "127.0.0.1";
        neib2_port = 18886;
        neib2_IP = "127.0.0.1";
    }
};
vector< struct proxy_rel > proxy_pool;

pthread_rwlock_t v_lock;
pthread_mutex_t pool_lock = PTHREAD_MUTEX_INITIALIZER;
void reply_out_handle(const string &reply_IP,const int &reply_port,const int &reply_ID);
void proxy_forward_handle(struct clove_rel &in_clove);
void * listen_handle(void *arg);
void *in_handle(void * inconnfd);	// for incoming socket
void proxy_out_handle(int outconnfd,struct sockaddr_in &my_sockaddr, int seq_number, string out_filename);	// for outgoing proxy discovery socket
int  init_out_task(struct sockaddr_in &my_sockaddr, const int outport,const string &outIP,const int tasktype); 
//for outgoing task, tasktype: 1 for proxy_discovery, 2 for proxy_confirmation, 3 for file query.
const int NeiborNUM=6;          // number of its neighbors
int neibornum=NeiborNUM;
string myrole;
struct neibor_info{
    int port;
    string IP;
};
vector< struct neibor_info > neibor_list;
struct sockaddr_in my_addr;

int main(int argc, char **argv)
{
    pid_t mypid;
    int test=0;
    int mylistenfd;
    string str1;
    string myIP = "127.0.0.1";
    string neibIP;
    int myport = 0;
    int neibport = 0;
    cout<<"please input my role in this simulation: initiator, neighbor, proxy."<<endl;
    cin>>myrole;
  //  myrole="neighbor";
    cout<<"please input my IP address:"<<endl;
    cin>>myIP;
   // myIP="127.0.0.1";
    cout<<"my IP is set to: "<<myIP<<endl;
    cout<<"please input my port:"<<endl;
    cin>>myport;
  //  myport=18886;
    cout<<"my port is set to: "<<myport<<endl;
   for( ; ;)
   {
       cout<<"please input a neighbor IP address(/'n/'to exit neibors setting):"<<endl;
       cin>>neibIP;
       if(neibIP=="n")
           break;
        // neibIP="114.212.191.117";
       cout<<"please input a neighbor  port:"<<endl;
       cin>>neibport;
        //neibport=18885;
        struct neibor_info neibor1;
        neibor1.port=neibport;
        neibor1.IP=neibIP;
        neibor_list.push_back(neibor1);
   }
   neibornum=neibor_list.size();
   cout<<"my neiborlist is set as"<<"(size "<<neibornum<<"):"<<endl;
   for(auto it=neibor_list.begin();it!=neibor_list.end();++it)
       cout<<(*it).IP<<":"<<(*it).port<<endl;
   //listen on my port **************************************************
    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    inet_pton(AF_INET, myIP.data(),&my_addr.sin_addr);
    my_addr.sin_port = htons(myport);
     cout<<"main my addr "<<inet_ntoa(my_addr.sin_addr)<<" : "<<ntohs(my_addr.sin_port)<<endl;
     bzero(buffer,sizeof(buffer));
    memcpy(buffer,&my_addr,sizeof(my_addr));
     mylistenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(mylistenfd<0){
       	 	cout<<"creat socket failed!"<<endl;
   	        exit(1);
        }
   	struct sockaddr_in my_laddr;
     bzero(&my_laddr,sizeof(my_laddr));
     my_laddr.sin_family = AF_INET;
     my_laddr.sin_addr.s_addr = htonl(INADDR_ANY);
     my_laddr.sin_port = htons(myport);
   	 
    if(bind(mylistenfd,(struct sockaddr *) &my_laddr, sizeof(my_laddr))<0){
       		cout<<"bind error"<<endl;
   	        exit(1);
     }
	 if(listen(mylistenfd,MAXCONN)< 0){
             cout<<"listen error"<<endl;
             exit(1);
     }
     cout<<" I am listening on my port "<<myport<<" ..."<<endl;
    

   //deal with the coming connections************************************
     int lockid = pthread_rwlock_init(&v_lock,NULL);
     if(lockid != 0)
         return(lockid);
     pthread_t thid1;
     int *lfd=&mylistenfd;
     int pc_ok = pthread_create(&thid1,NULL,listen_handle,(void *)lfd);
     if(pc_ok != 0)
         cout<<"failed during handling the outcoming conenctions!"<<endl;
    
     mypid=fork();
    //initiator start the proxy discovery procedure***********************
    if(mypid == 0){     
            if(myrole=="initiator"){
                init_out_task(my_addr, neibport, neibIP, 1);
            }
            exit(0);
    } 
    // interactive interface**********************************************
    for( ; ; ){
        if(myrole=="initiator")
        {
        cout<<"input your filename to search or /'exit/' to exit"<<endl;
        string str1;
        cin>>str1;
        if(str1=="exit"){
        break;
        }
        }
        else
        {
            cout<<"input /'exit/' to terminate"<<endl;
            string str2;
            cin>>str2;
            if(str2=="exit")
                break;
        }
    }
    close(mylistenfd);
    return 0;
}

void * listen_handle(void *arg)
{
    struct sockaddr_in inpeer_addr;
    socklen_t inpeer_len = sizeof(inpeer_addr);
    pthread_t inthid;    
    int lis_connfd =*( (int *) arg);
    for( ; ;)  {
         int   inconnfd=accept(lis_connfd,(struct sockaddr *) &inpeer_addr,&inpeer_len);
        if(inconnfd < 0){
            cout<<"accept error"<<endl;
            continue;
        }
        cout<<"receive connections from :"<<inet_ntoa(inpeer_addr.sin_addr)<<":"<<ntohs(inpeer_addr.sin_port)<<endl;
        int *infd=(int *)malloc(sizeof(int));
        *infd=inconnfd;
        pthread_create(&inthid,NULL,in_handle,(void *)infd);
       // in_handle(inconnfd);// add codes here to deal with the incoming connections.
       // close(inconnfd);
     }
}
void * in_handle( void * arg)
{
    pthread_detach(pthread_self());
    int inconnfd =*( (int *) arg);
    free(arg);
    struct sockaddr_in pre_addr;
   // bzero(&pre_addr,sizeof(pre_addr));
    cout<<"have a in connection"<<endl;
    char in_buffer[BUFF_SIZE];
    bzero(in_buffer, sizeof(in_buffer));
    int in_length=recv(inconnfd, in_buffer, BUFF_SIZE, 0);
    if(in_length < 0)
    {
        cout<<"read from inconnection failed"<<endl;
    }
    //neighbor or( proxy to be ) deal with proxy_discovery message
 if(strncmp("Proxy_discovery",in_buffer,15) == 0)
    {
        cout<<"recceive a proxy_discovery message"<<endl;
        bzero(in_buffer,sizeof(in_buffer));
        in_length=recv(inconnfd,in_buffer,BUFF_SIZE,0);
        memcpy(&pre_addr,in_buffer,sizeof(pre_addr));
        cout<<"accept from "<<inet_ntoa(pre_addr.sin_addr)<<" : "<<ntohs(pre_addr.sin_port)<<endl;
        bzero(in_buffer,sizeof(in_buffer));
        in_length=recv(inconnfd,in_buffer,BUFF_SIZE,0);
        int inint=atoi(in_buffer);
        cout<<"receive seq:"<<inint<<endl;
        // add code to receive the clove file;#####################################################
        struct clove_rel in_clove;
        in_clove.seq_number = inint;
        in_clove.pre_port = ntohs(pre_addr.sin_port);
        in_clove.pre_IP =string( inet_ntoa(pre_addr.sin_addr));
        bool IsTwo = false;
        int same_port=0;
        string same_IP;
        close(inconnfd);//close the specific accept socket
        // pthread_rwlock_wrlock(&v_lock);
        // cout<<in_clove.pre_IP<<endl;
        if(!cloves_vector.empty())
        {
            for(auto it = cloves_vector.begin(); it != cloves_vector.end();++it)
                if( (*it).seq_number == inint )
                {
                    IsTwo = true;
                    same_port = (*it).pre_port;
                    same_IP = (*it).pre_IP;
                }
        }
        if(IsTwo)
        {
            // decide whether to be a proxy of the initiator
            cloves_vector.push_back(in_clove);
            long proxyID = rand()%100000+1;
            reply_out_handle(in_clove.pre_IP,in_clove.pre_port,proxyID);
            reply_out_handle(same_IP,same_port,proxyID);
            
        }
        else
        {
            proxy_forward_handle(in_clove);
        
        }
       // pthread_rwlock_unlock(&v_lock);
    }
 // neighbor forward 
 else if(strncmp("proxy_agree",in_buffer,11) == 0)
    {
        cout<<"receive a proxy_agree message"<<endl;
        bzero(in_buffer,sizeof(in_buffer));
        in_length=recv(inconnfd,in_buffer,BUFF_SIZE,0);
        int proxyid=atoi(in_buffer);
        cout<<"receive proxyid (from proxy):"<<proxyid<<endl;
        if(myrole=="initiator")
        {

        }
        else
        {
        //forward a agree message

        }
    close(inconnfd);//close the specific accept socket

    }
 else if(strncmp("File_Delivery",in_buffer,13) == 0)
    {
        cout<<"receive a File_Delivery message"<<endl;
         while(true)
        {
            break;//receive the file cloves
        }   
        
        close(inconnfd);//close the specific accept socket

    }
   // close(inconnfd);//close the specific accept socket
        return NULL;

}
//judge whether to be a proxy, and sned agree reply message if necessary:
void reply_out_handle(const string &reply_IP,const int &reply_port,const int &reply_ID)
{

    cout<<"the proxy begin reply..."<<endl;
    struct sockaddr_in reply_addr;
    bzero(&reply_addr,sizeof(reply_addr));
    reply_addr.sin_family = AF_INET;
    reply_addr.sin_port = htons(reply_port);
    inet_pton(AF_INET,reply_IP.data(),&reply_addr.sin_addr);
    int replyfd = socket(AF_INET,SOCK_STREAM,0);
    if(replyfd<0)
    {
        cout<<"proxy_reply socket creating failed!"<<endl;
        return ;
    }
    if(connect(replyfd,(struct sockaddr *) &reply_addr, sizeof(reply_addr)) < 0)
    {
        cout<<"proxy_reply socket connecting failed!"<<endl;
        return;
    }
    cout<<"have a proxy reply connection out"<<endl;
    char reply_buffer[BUFF_SIZE];
    bzero(reply_buffer,sizeof(reply_buffer));
    send(replyfd,"proxy_agree",12,0);
    stringstream sid;
    string idnum;
    sid<<reply_ID;
    sid>>idnum;
    cout<<"proxy id:"<<idnum<<endl;
    send(replyfd,idnum.c_str(),20,0); 
    //
}
//randomly forward to one of the neighbors of the peer:
void proxy_forward_handle(struct clove_rel &in_clove)
{
    if(neibornum==0)
    {
        cout<<"add a clove"<<endl;
        cloves_vector.push_back(in_clove);
        return ;
    }
    int forwardid = rand()%neibornum;
    auto it2=neibor_list.begin();
    bool IsRepeat=(in_clove.pre_IP==(*(it2+forwardid)).IP)&&((in_clove.pre_port)==(*(it2+forwardid)).port);
    while(IsRepeat)
    {
        forwardid = rand()%neibornum;
        IsRepeat=(in_clove.pre_IP==(*(it2+forwardid)).IP)&&((in_clove.pre_port)==(*(it2+forwardid)).port);
    }
    string forward_IP=(*(it2+forwardid)).IP;
    int forward_port=(*(it2+forwardid)).port;
    in_clove.next_port=forward_port;
    in_clove.next_IP=forward_IP;
    struct sockaddr_in forward_addr;
    cout<<"begin forwarding a proxy_discovery message"<<endl;
    bzero(&forward_addr,sizeof(forward_addr));
    forward_addr.sin_family = AF_INET;
    forward_addr.sin_port = htons(forward_port);
    inet_pton(AF_INET,forward_IP.data(),&forward_addr.sin_addr);
    int forwdfd=socket(AF_INET, SOCK_STREAM,0);
    if(forwdfd<0)
    {
        cout<<"forward out socket creating failed!"<<endl;
        return ;
    }
    if(connect(forwdfd,(struct sockaddr *) &forward_addr, sizeof(forward_addr)) < 0)
    {
        cout<<"forward out socket connecting failed!"<<endl;
        return ;
    }
    cout<<"have a proxy forward connection out"<<endl;
    cloves_vector.push_back(in_clove);
   // char out_buffer[BUFF_SIZE];
   // bzero(out_buffer,sizeof(out_buffer));
    send(forwdfd,"Proxy_discovery",16,0);
   // memcpy(out_buffer,&my_addr,sizeof(my_addr));
    cout<<"send my addr "<<inet_ntoa(my_addr.sin_addr)<<" : "<<ntohs(my_addr.sin_port)<<endl;
    send(forwdfd,buffer,BUFF_SIZE,0);
   // bzero(buffer,BUFF_SIZE);
    stringstream ss;
    string seqnum;
    ss<<in_clove.seq_number;
    ss>>seqnum;
    cout<<seqnum<<endl;
    send(forwdfd,seqnum.c_str(),BUFF_SIZE,0); 
    //send cloves and so on...

}

void proxy_out_handle(int outconnfd,struct sockaddr_in &my_sockaddr,int seq_number,string out_filename)
{
    cout<<"have a proxy out connection"<<endl;
    char out_buffer[BUFF_SIZE];
    bzero(out_buffer,sizeof(out_buffer));
    send(outconnfd,"Proxy_discovery",16,0);
    memcpy(out_buffer,&my_sockaddr,sizeof(my_sockaddr));
    cout<<"send my addr "<<inet_ntoa(my_sockaddr.sin_addr)<<" : "<<ntohs(my_sockaddr.sin_port)<<endl;
    send(outconnfd,out_buffer,BUFF_SIZE,0);
    bzero(out_buffer,BUFF_SIZE);
    cout<<seq_number<<endl;
    stringstream ss;
    string seqnum;
    ss<<seq_number;
    ss>>seqnum;
    cout<<seqnum<<endl;
    send(outconnfd,seqnum.c_str(),BUFF_SIZE,0);
    
    
    //send cloves and so on...
   // close(outconnfd); //out not positively close a connection
}

int  init_out_task(struct sockaddr_in &my_sockaddr,const int outport,const string& outIP,const int tasktype)
{
    struct sockaddr_in out_addr;
    int outconnfd=0;
    if(tasktype==1)
    {
        // IDA the file M to eiborMUM parts
            srand(time(NULL));
            int seq_number=rand()%1000;
            for(int i=0;i<NeiborNUM;i++){
                seq_number=rand()%1000;
                bzero(&out_addr,sizeof(out_addr));
                out_addr.sin_family = AF_INET;
                out_addr.sin_port = htons(outport+1);
                inet_pton(AF_INET, outIP.data(),&out_addr.sin_addr);
                outconnfd=socket(AF_INET, SOCK_STREAM,0);
                stringstream ss1;
                string si1;
                ss1<<i;
                ss1>>si1;
                string fname=string("M.00")+si1;
                cout<<"this filename:"<<fname<<endl;
                if(outconnfd < 0)
                {
                    cout<<"out socket creating error"<<endl;
                    continue;
                }
    /*            if(bind(outconnfd, (struct sockaddr *)&my_sockaddr, sizeof(my_sockaddr)))//bind local address
                {
                    cout<<"bind local addr error"<<endl;
                    continue;
                }
      */
                if(connect(outconnfd,(struct sockaddr *) &out_addr, sizeof(out_addr)) <0){
                cout<<"neighbor connect error"<<endl;
                continue;
                }
                 //    cout<<"init_out_task my addr "<<inet_ntoa(my_sockaddr.sin_addr)<<" : "<<ntohs(my_sockaddr.sin_port)<<endl;
                pid_t  childpid=fork();
                if(childpid==0){
                        proxy_out_handle(outconnfd,my_sockaddr,seq_number,fname);
                        exit(0);
                 }
                close(outconnfd);
             }
    }
    return 0;

}
