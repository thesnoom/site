/*
 * Small utility to manage iptables, easily executable from admin.redcross.htb
 * v0.1 - allow and restrict mode
 * v0.3 - added check method and interactive mode (still testing!)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#define BUFFSIZE 360

int isValidIpAddress(char *ipAddress)
{
    	struct sockaddr_in sa;
    	int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
	return result != 0;
}

int isValidAction(char *action){
	int a=0;
	char value[10];
	strncpy(value,action,9);
	if(strstr(value,"allow")) a=1;
	if(strstr(value,"restrict")) a=2;
	if(strstr(value,"show")) a=3;
	return a;
}

void cmdAR(char **a, char *action, char *ip){
	a[0]="/sbin/iptables";
	a[1]=action;
	a[2]="INPUT";
       	a[3]="-p";
       	a[4]="all";
       	a[5]="-s";
	a[6]=ip;
	a[7]="-j";
       	a[8]="ACCEPT";
	a[9]=NULL;
	return;
}

void cmdShow(char **a){
	a[0]="/sbin/iptables" ;
	a[1]="-L";
       	a[2]="INPUT";
	return;
}

void interactive(char *ip, char *action, char *name){
	char inputAddress[16];
	char inputAction[10];
	printf("Entering interactive mode\n");
	printf("Action(allow|restrict|show): ");
	fgets(inputAction,BUFFSIZE,stdin);
	fflush(stdin);
	printf("IP address: ");
	fgets(inputAddress,BUFFSIZE,stdin);
	fflush(stdin);
	inputAddress[strlen(inputAddress)-1] = 0;
	if(! isValidAction(inputAction) || ! isValidIpAddress(inputAddress)){
		printf("Usage: %s allow|restrict|show IP\n", name);
		exit(0);
	}
	strcpy(ip, inputAddress);
	strcpy(action, inputAction);
	return;
}

int main(int argc, char *argv[]){
	int isAction=0;
	int isIPAddr=0;
	pid_t child_pid;
	char inputAction[10];
	char inputAddress[16];
	char *args[10];
	char buffer[200];

	if(argc!=3 && argc!=2){
		printf("Usage: %s allow|restrict|show IP_ADDR\n", argv[0]);
		exit(0);
	}
	if(argc==2){
		if(strstr(argv[1],"-i")) interactive(inputAddress, inputAction, argv[0]);
	}
	else{
		strcpy(inputAction, argv[1]);
		strcpy(inputAddress, argv[2]);
	}
	isAction=isValidAction(inputAction);
	isIPAddr=isValidIpAddress(inputAddress);
	if(!isAction || !isIPAddr){
		printf("Usage: %s allow|restrict|show IP\n", argv[0]);
		exit(0);
	}
	puts("DEBUG: All checks passed... Executing iptables");
	if(isAction==1) cmdAR(args,"-A",inputAddress);
	if(isAction==2) cmdAR(args,"-D",inputAddress);
	if(isAction==3) cmdShow(args);
	
	child_pid=fork();
	if(child_pid==0){
		setuid(0);
		execvp(args[0],args);
		exit(0);
	}
	else{
		if(isAction==1) printf("Network access granted to %s\n",inputAddress);
		if(isAction==2) printf("Network access restricted to %s\n",inputAddress);
		if(isAction==3) puts("ERR: Function not available!\n");
	}
}
