#define WINVER 0x0501
#include <stdio.h>
#include <stdlib.h>
#include "RoboMancer.h"
#include "RecordMobot.h"
#include "thread_macros.h"
#ifndef _MSYS
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#else
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#define PORT "5768"
#define BACKLOG 10

struct commsThread_s {
  int new_fd;
  recordMobot_t* mobot;
};

// get sockaddr, IPv4 or IPv6:
#if 0
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
#endif

void* commsThread(void* arg)
{
  struct commsThread_s* comms = (struct commsThread_s*)arg;
  /* This thread will receive whole messages from the connecting client and
   * forward those messages to the Mobot */
  /* Receive messages 1 byte at a time until the entire message is received */
  int err;
  uint8_t byte;
  uint8_t buf[100];
  int bytes = 0; int datasize = 100;
  while(1) {
    bytes = 0; datasize = 100;
    while(bytes < datasize) {
      err = recv(comms->new_fd, (char*)&byte, 1, 0);
      if(err <= 0) { 
        /* Mark the mobot as unbound */
        comms->mobot->bound = false;
        return NULL; 
      }
      buf[bytes] = byte;
      /* The second byte stores the message size */
      if(bytes == 1) {
        datasize = byte;
      }
      bytes++;
    }
    /* Send the message to the Mobot */
#if 0
    SendToIMobot((mobot_t*)comms->mobot, buf[0], &buf[2], bytes-3);
    /* Get a response from the Mobot */
    RecvFromIMobot((mobot_t*)comms->mobot, &buf[0], 100);
#endif
    MobotMsgTransaction((mobot_t*)comms->mobot, buf[0], &buf[2], bytes-3);
    /* Send the response to the connected party */
    send(comms->new_fd, (const char*)&buf[2], buf[3], 0);
  }
  return NULL;
}

void* listenThread(void* arg)
{
  int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  //struct sigaction sa;
  int yes=1;
  char s[INET6_ADDRSTRLEN];
  int rv;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return NULL;
  }

  // loop through all the results and bind to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
            p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
#ifdef _MSYS
          (const char*)&yes,
#else
          &yes,
#endif
          sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
#ifdef _MSYS
      closesocket(sockfd);
#else
      close(sockfd);
#endif
      perror("server: bind");
      continue;
    }

    break;
  }

  if (p == NULL)  {
    fprintf(stderr, "server: failed to bind\n");
    return NULL;
  }

  freeaddrinfo(servinfo); // all done with this structure

  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

#if 0
  sa.sa_handler = sigchld_handler; // reap all dead processes
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }
#endif


  while(1) {  // main accept() loop
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
      continue;
    }

    /*
    inet_ntop(their_addr.ss_family,
        get_in_addr((struct sockaddr *)&their_addr),
        s, sizeof s);
    printf("server: got connection from %s\n", s);
    */

    /* Check to see if there are available mobots to control */
    recordMobot_t* mobot;
    mobot = g_robotManager->getUnboundMobot();
    if(mobot == NULL) {
      /* No unconnected mobots found. Disconnect immediately */
#ifdef _MSYS
      closesocket(new_fd);
#else
      close(new_fd);
#endif
      continue;
    }
    mobot->bound = true;

#if 0
    if (!fork()) { // this is the child process
      close(sockfd); // child doesn't need the listener
      if (send(new_fd, "Hello, world!", 13, 0) == -1)
        perror("send");
      close(new_fd);
      exit(0);
    }
#endif
    /* Spawn the new thread */
    THREAD_T thread;
    struct commsThread_s *s;
    s = (struct commsThread_s*)malloc(sizeof(struct commsThread_s));
    s->new_fd = new_fd;
    s->mobot = mobot;
    THREAD_CREATE(&thread, commsThread, s);
  }
  return NULL;
}

int initializeComms(void)
{
  /* Initialize Winsock */
#ifdef _MSYS
  WSADATA wsaData;
  if(WSAStartup(MAKEWORD(2,0), &wsaData) != 0) {
    fprintf(stderr, "WSAStartup Failed.\n");
    exit(1);
  }
#endif
  /* Just start the listenThread */
  THREAD_T thread;
  THREAD_CREATE(&thread, listenThread, NULL);
}
