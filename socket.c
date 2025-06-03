// This is all from beej’s network programming guide.
// TODO: finish over-commenting everything and make sure each line is understood

#include <stdio.h>
#include <stdlib.h>
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


#define PORT "80" 	// http port
#define BACKLOG 10	// how many pending connections queue will hold

void sigchld_handler(int s)
{
	(void)s; // quite unused variable warning
	
	// waitpid() might averwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	//NL we declare our two sockets
	int sockfd, new_fd; // "listen on sock_fd, new connection on new_fd" > why fd ?
	
	/* This defines three address structure
	 * Requires netdb.h:
	 * hints: struct addrinfo holding family, socktype and flags
	 * *servinfo: to be populated with getaddrinfo()
	 * *p: buffer for addrinfo structs
	*/
	struct addrinfo hints, *servinfo, *p;

	// 
	struct sockaddr_storage their_addr;

	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);		//Ensure all zeroes in hint’s structure
	hints.ai_family = AF_INET;			//IPvX
	hints.ai_socktype = SOCK_STREAM;		//TCP
	hints.ai_flags = AI_PASSIVE;			//Use local IP, then bind() & accept(), listens to * == INADDR_ANY == IN6ADDR_ANY_INIT
	
	/*
	 * This calls getaddrinfo with:
	 * NULL as the node
	 * PORT (80) as the port/service
	 * The address of hint structure
	 * The pointer to pointer to servinfo, which will hold the result of the call
	 * 	Why pointer to pointer? Because it can return multiple addrinfo structs
	 * It returns a status code to rv, then rv is error checked
	 * I pass we got one or more addrinfo structs at &servinfo
	*/

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	/*
	 * Loop through all the results and bind to the first we can
	 * We use *p as buffer
	*/
	for (p = servinfo; p != NULL; p = p->ai_next) { 					// start at servinfo, stop if P is NULL, assigns p.ai_next to p
		if (( sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1 ) {	// Assign a socket descriptor to sockfd with the first addrinfo attributes
			perror("server: socket");						// TODO check how whatever that check/continue works
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {	// Sets sockfd’s option SO_REUSEADDR in SOL_SOCKET level at 1
			perror("setsockopt");							// Allows bind to use an adress already used by another socket
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {				// Binds IP and port ?
			close(sockfd);								
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // Done with this structure
	
	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {		// Tell sockfd to listen to connections, up to BACKLOG concurrent ones
		perror("listen");
		exit(1);
	}

	// TODO I don’t really know this part but it is about avoiding zombie processes
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}
	
		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) { // this is the child process
			close(sockfd); // child does not need the listener
			if (send(new_fd, "Hello, world!\n", 13, 0) == -1)
				perror("send");
			close(new_fd);
			exit(0);
		}
		close(new_fd); // parent doesn’t need this
	}

	return 0;
}
