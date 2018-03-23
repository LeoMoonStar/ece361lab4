/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: zhan2500
 *
 * Created on March 18, 2018, 2:05 PM
 */

/*
 ** selectserver.c -- a cheezy multiperson chat server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT "9034"   // port we're listening on

// get sockaddr, IPv4 or IPv6:

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

int main(void) {
    //printf("here\n");
    fd_set master; // master file descriptor list
    fd_set read_fds; // temp file descriptor list for select()
    int fdmax; // maximum file descriptor number

    //int listener; // listening socket descriptor
    //int newfd; // newly accept()ed socket descriptor
    //struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[256]; // buffer for client data
    int nbytes;

    char s[INET6_ADDRSTRLEN];

    int i, j, rv;
    int sockfd;

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master); // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    /*
        for (p = ai; p != NULL; p = p->ai_next) {
            listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            sockfd=listener;
            if (listener < 0) {
                continue;
            }

            // lose the pesky "address already in use" error message
            setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int));

            if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
                close(listener);
                continue;
            }

            break;
        }
     */
    /*

        // if we got here, it means we didn't get bound
        if (p == NULL) {
            fprintf(stderr, "selectserver: failed to bind\n");
            exit(2);
        }
     */

    for (p = ai; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("client: connect");
            close(sockfd);
            continue;
        }

        break;
    }


    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }
    //printf("here2\n");
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr), s, sizeof s);

    freeaddrinfo(ai); // all done with this

    /*
        // listen
        if (listen(listener, 10) == -1) {
            perror("listen");
            exit(3);
        }
     */


    FD_SET(sockfd,&read_fds);
    FD_SET(sockfd, &master);
    FD_SET(0,&master);
    
    //printf("here3\n");

    // keep track of the biggest file descriptor
    fdmax = sockfd; // so far, it's this one

    //printf("fdmax: %d", fdmax);
    //printf("here4\n");

    // what to send 
    char input[500];
    int numbytes;
    
    //printf("your username:\n");
    char user[50];
    fgets(user,50,stdin);
    send(sockfd, user, strlen(user)+1, 0);
    
    // main loop
    for (;;) {
        //printf("here5\n");
        read_fds = master; // copy it
        //printf("here7\n");
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            //printf("here6\n");
            perror("select");
            exit(4);
        }
        //printf("here6\n");
        //printf("here1\n");
        // run through the existing connections looking for data to read
        for (i = 0; i <= fdmax; i++) {
            //printf("here9\n");
            if (FD_ISSET(i, &read_fds)) {
                if (i == 0) { //if this is keyboard input
                    bzero(input, 500);
                    fgets(input, 500, stdin);
                    //printf("here11\n");
                    //printf("my input is : %s", input);
                    //send data to port
                    if ((numbytes = send(sockfd, input, strlen(input)+1, 0)) == -1) {
                        printf("here10\n");
                        perror("talker: send");
                        exit(1);
                    }
                    bzero(input, 500);

                } else {
                    //printf("here12\n");
                    // handle data from a server
                    //nbytes = recv(i, buf, sizeof buf, 0);
                    // printf("here13\n");
                    if ((nbytes = recv(sockfd, buf, sizeof buf, 0)) <= 0) {
                        //printf("here14\n");

                        // got error or connection closed by server
                        if (nbytes == 0) {
                            // connection closed
                            //printf("selectserver: socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        //close(i); // bye!
                        //FD_CLR(i, &master); // remove from master set
                    } else {
                        // we got some data from a client
                        //printf("here15\n");
                        //printf("%s\n", buf);
                    }
                    //printf("here14\n");

                } // END handle data from client
            }// END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
    
    return 0;
}

