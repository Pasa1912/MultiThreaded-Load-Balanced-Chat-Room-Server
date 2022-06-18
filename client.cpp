#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <signal.h>
#include <mutex>
#define PORT 6000
#define MAX_LEN 256
#define NUM_COLORS 6
using namespace std;

bool exit_flag=false;
thread t_send, t_recv;
int lb_socket, client_socket;
string def_col="\033[0m";
string colors[]={"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"};

void catch_ctrl_c(int signal);
string color(int code);
int eraseText(int cnt);
void send_message(int client_socket);
void recv_message(int client_socket);

int main()
{
	if((lb_socket=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("socket: ");
		exit(-1);
	}

	struct sockaddr_in lb;
	lb.sin_family=AF_INET;
	lb.sin_port=htons(PORT); // Port no. of loadbalancer
	lb.sin_addr.s_addr=INADDR_ANY;
	bzero(&lb.sin_zero,0);

	if((connect(lb_socket,(struct sockaddr *)&lb,sizeof(struct sockaddr_in)))==-1)
	{
		perror("connect: ");
		exit(-1);
	}
	signal(SIGINT, catch_ctrl_c);
	char name[MAX_LEN], room[MAX_LEN];
	cout<<"Enter your name : ";
	cin.getline(name,MAX_LEN);
	cout<<"Enter the Room Id: ";
	cin.getline(room,MAX_LEN);
	send(lb_socket,name,sizeof(name),0);
	send(lb_socket,room,sizeof(room),0);
	int serverPort;
	recv(lb_socket,&serverPort,sizeof(serverPort),0);
	close(lb_socket);

	cout<<"Server port recieved: "<<serverPort<<"\n";

	if((client_socket=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("socket: ");
		exit(-1);
	}
	struct sockaddr_in client;
	client.sin_family=AF_INET;
	client.sin_port=htons(serverPort); // Port no. of server
	client.sin_addr.s_addr=INADDR_ANY;
	bzero(&client.sin_zero,0);
	
	if((connect(client_socket,(struct sockaddr *)&client,sizeof(struct sockaddr_in)))==-1)
	{
		perror("connect: ");
		exit(-1);
	}
	signal(SIGINT, catch_ctrl_c);

	send(client_socket,name,sizeof(name),0);
	send(client_socket,room,sizeof(room),0);

	cout << colors[NUM_COLORS-1]<<"\n\t*********CHAT ROOM***********"<<"\n"<<def_col;

	thread t1(send_message, client_socket);
	thread t2(recv_message, client_socket);

	t_send=move(t1);
	t_recv=move(t2);

	if(t_send.joinable())
		t_send.join();
	if(t_recv.joinable())
		t_recv.join();
			
	return 0;
}

void catch_ctrl_c(int signal) 
{
	char str[MAX_LEN]="#exit";
	send(client_socket,str,sizeof(str),0);
	exit_flag=true;
	t_send.detach();
	t_recv.detach();
	close(client_socket);
	exit(signal);
}

string color(int code)
{
	return colors[code%NUM_COLORS];
}

int eraseText(int cnt)
{
	char back_space = 8;
	for(int i=0; i<cnt; i++)
	{
		cout<<back_space;
	}	
	return 1;
}

void send_message(int client_socket)
{
	while(true)
	{
		cout<<colors[1]<<"You : "<<def_col;
		char str[MAX_LEN];
		cin.getline(str,MAX_LEN);
		send(client_socket,str,sizeof(str),0);
		if(strcmp(str,"#exit")==0)
		{
			exit_flag=true;
			t_recv.detach();	
			close(client_socket);
			return;
		}	
	}		
}

void recv_message(int client_socket)
{
	while(1)
	{
		if(exit_flag)
			return;
		char othername[MAX_LEN], othermsg[MAX_LEN];
		int color_code;
		int bytes_received=recv(client_socket,othername,sizeof(othername),0);
		if(bytes_received<=0)
			continue;
		recv(client_socket,&color_code,sizeof(color_code),0);
		recv(client_socket,othermsg,sizeof(othermsg),0);
		eraseText(6);
		if(strcmp(othername,"#NULL")!=0)
			cout<<color(color_code)<<othername<<" : "<<def_col<<othermsg<<endl;
		else
			cout<<color(color_code)<<othermsg<<endl;
		cout<<colors[1]<<"You : "<<def_col;
		fflush(stdout);
	}	
}