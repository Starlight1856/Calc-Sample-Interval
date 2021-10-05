//************************************************************************
//**
//** Program to test interval determination using calculation of mode value, using
//** CCalctime class ; calctime.cpp  calctime.h
//**
//** This program simulates, using a udp loopback, the timing 'jitter' values experienced
//** when measuring the intervals between data samples in attempting to determine data rate.
//**
//** It was found that attempting to use an average or mean value did not work, as some
//** time intervals were either too low, or excessively high to to using a non-realtime
//** operating system. However it was discovered that an accurate value could be determined
//** if the mode, or most common value was calculated. In calculating the mode a window
//** boundry time is set, and the most common time which appears within this time is determined.
//**
//** e.g. if a window boundry of 1 mS is set, the most common time interval starting from the minium
//** is determined; suppose actual sample data rate is 100mS and minimum time measured is 90mS
//** Time         samples
//** 90 -> 91       1
//** 91 -> 92       0
//** 92 -> 93       1
//** 93 -> 94       0
//** 95 -> 96       0
//** 97 -> 98       2
//** 99 -> 100      3
//** 100 -> 101    26
//** 101 -> 102     5
//** 103 -> 102     4
//** etc.
//** Here the mode value would be determined as 100mS or a 10Hz sample rate
//**
//***************************************************************************************


#include <iostream>
#include <time.h>
#include <unistd.h>
#include <sys/signalfd.h>
#include <signal.h>

using namespace std;
#include <vector>
#include <algorithm>
#include <numeric>
#include <thread>
#include <string>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT     8080
#define MAXLINE 1024

#include "calctime.h"


void timed_test(string msg);




int main()
{
    int sockfd;
    char buffer[MAXLINE];
    CCalcAutoRate calc_rate;


    //**********************************************
    //** Setup for test: window 1 milisec, 51 samples
    //**********************************************
    calc_rate.Initialise(1.0, 51);


    //** Setup server
    struct sockaddr_in servaddr, cliaddr;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
        {
        cout << "socket creation failed" << endl;
        exit(EXIT_FAILURE);
        }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,
               sizeof(servaddr)) < 0 )
      {
      cout<< "bind failed" << endl;
      exit(EXIT_FAILURE);
      }


    cout << "Hello world!" << endl;


    //** Start client thread
    thread t1(timed_test, "thread1");


    //** start server
    socklen_t len = sizeof(servaddr);

    //** timeout in 5 sec if no comms
    fd_set readFd;
    FD_ZERO(&readFd);
    FD_SET(sockfd, &readFd);
    struct timeval timeout;
    timeout.tv_sec = 5;  //** timeout 5 sec
    timeout.tv_usec = 0;

    if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0)
       {
       cout << "setsockopt failed" << endl ;
       }

    int32_t n;

    while(1)
      {
      if(FD_ISSET(sockfd, &readFd))
        {
        n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                &len);


        if( n < 0 )
          {
          cout << "No data; timed out"<< endl;
          break;
          }

        cout << buffer << "   " << n << endl;


        //** After specified number of samples have been processed mode is calculated.
        if( calc_rate.AutoRate() )
          {
          cout<<"Mode Result = "<< calc_rate.GetModeResult()<< "mS  "<<"Freq is "<< calc_rate.GetFreq() << "Hz" <<endl;
          break;
          }
        }

      }

    close( sockfd );

    //** Wait for client thread to close.
    t1.join();


    return 0;
}

//********************************************
//* Client thread
//********************************************
void timed_test(string msg)
{

    int sockfd;
    char testmsg[30];
    struct sockaddr_in     servaddr;

    strncpy(testmsg, "TestMessage123456789\0", 21);

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    //** set address
    inet_pton(AF_INET,"127.0.0.1", &(servaddr.sin_addr) ); //** Loop back
    //inet_pton(AF_INET,"192.168.1.148", &(servaddr.sin_addr) );

    //** Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    //servaddr.sin_addr.s_addr = INADDR_ANY;



    //** send test message
    for(int i=0; i<60; i++)
      {
      sendto(sockfd, (const char *)testmsg, strlen(testmsg),
        MSG_CONFIRM, (const struct sockaddr *) &servaddr,
            sizeof(servaddr));

      //** Simulates jitter observed on acquisition
      if( i == 10)
        {
        usleep(70000);
        }
      else if( i == 20)
        {
        usleep(200000);
        }
      else
        {
        usleep(100000);
        }

      }

  close( sockfd );
}
