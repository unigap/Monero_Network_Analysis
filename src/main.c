/**********  main.c  **********

 * Execution of this program initializes monero peer-to-peer network analisys:
 * *   Main thread: This thread initialize all threads and catch two signals: SIGINT and SIGTERM -> write logbst file (nodes info).
 * *  - 1st thread: For each node request the peer list with 1001 message (+ or - 250 [IP, Port]) and store the information received on binary search tree. -> log1001
 * *  - 2nd thread: Check with 1003 message if each node still available. If not response is received, then node will be removed from map.                  -> log1003
 * *  - 3rd thread: Get coordenates of each node and print them on standard output (to comunicate by pipe with the other program: locate.py)                -> logmap
 * *  - 4th thread: Wait for 2002 message (recv 2002 notification, count transactions: 500 byte each transaction)                                           -> log2002

 * Compile: gcc main.c bst.c request1001.c check1003.c location.c recv2002.c -lpthread -o main
 * Run:     ./main <IP> <PORT> <time12>
 * IP: monero destination node IP address
 * PORT: natural int, often number 18080
 * time12: time limit to request 1001 and 1003 (threads 1,2) and then receive transactions (thread 4) from those nodes
 >>> Combine this execution output with the Python program to locate nodes on map (locate.py):

Examples:
 Help: ./main # Displays monero seed nodes to start execution, using one of them
 Run:  ./main 212.83.175.67 18080 10 | python locate.py # 10 seg collecting nodes (locating them on map) and then receiving transactions from available nodes
SEED_NODES = https://community.xmr.to/xmr-seed-nodes

*/

#include "main.h"
#include "bst.h"
#include "request1001.h"
#include "check1003.h"
#include "location.h"
#include "recv2002.h"

// Method to initialize bst_ip data structure //
void init(char *argv1, char *argv2) {
	root = malloc(sizeof *root);
	root->nodip.s_addr = inet_addr(argv1);
	root->port = atoi(argv2);
	root->stat = 0;
	root->lon = 0;
	root->lat = 0;
	root->tkop = -1;
	//root->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	//root->cond = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
	root->left = NULL;
	root->right = NULL;

}

// Catch SIGINT signal (Keyboard Interrupt) in order to terminate and write info on 'logbst' file //
void intHandler() {
	printf("q\n"); // to terminate python program (pipe)
	print_bst();
	exit(0);
}

// Catch SIGTERM signal (kill command's default signal) in order to terminate and write info on 'logbst' file //
void termHandler() {
	printf("q\n"); // to terminate python program (pipe)
	print_bst();
	exit(1);
}

// main function //
int main(int argc, char *argv[]) {

	// Argument validation //
	if(argc != 4 || inet_addr(argv[1]) == -1 || atoi(argv[2]) == 0 || atoi(argv[3]) == 0) {
		printf(B_RED"Destination IP, port and time limit for requests (seg) are necessary to initialize the execution!\n"RESET
		       "Destination node can be selected from the following Monero seed-nodes list: \n"
				"  - 66.85.74.134 18080\n"
				"  - 88.198.163.90 18080\n"
				"  - 95.217.25.101 18080\n"
				"  - 104.238.221.81 18080\n"
				"  - 192.110.160.146 18080\n"
				"  - 209.250.243.248 18080\n"
				"  - 212.83.172.165 18080\n"
				"  - 212.83.175.67 18080\n" );
		printf("q\n"); // to terminate python program (pipe)
		exit(1);
	}

	// Initialize main data-structure (binary search tree) //
	init(argv[1], argv[2]);

	int time = atoi(argv[3]);
	signal(SIGINT, intHandler);
	signal(SIGTERM, termHandler);

	// Initialize mutex //
	if(pthread_mutex_init(&(root->lock), NULL) != 0) {
		printf("mutex init failed\n");
		fflush(stdout);
		exit(1);
	}

// NOT USED	--
//	if(pthread_cond_init (&(root->cond), NULL) != 0) {
//		printf("cond init failed\n");
//		fflush(stdout);
//		exit(1);
//	}

	// create log file
	FILE *log;
	log = fopen("log0", "w");
	if(log == NULL) {
		printf("Error creating log0 file: %s\n", strerror(errno));
		fflush(stdout);
	}

	char *h_ip = inet_ntoa(root->nodip);
	fprintf(log, "Destination node: %s %d\n", h_ip, root->port);

	char sport[7];
	sprintf(sport, "%d", root->port);

	// Thread creation //
	char *message0 = (char *)malloc(10);
	sprintf(message0, "%d", time); // after time (seg) terminate Levin requests (1st and 2nd threads) and start 4th thread (3rd async)

	pthread_t thread1;

	if( pthread_create( &thread1 , NULL , start1001 , (void*) message0) < 0)
	{
		fprintf (log, "Couldn't create thread1. Error number: %d   Errore message: %s \n" , errno , strerror(errno));
		fflush(log);
		exit(0);
	}

	pthread_t thread2;

	if( pthread_create( &thread2 , NULL , start1003 , (void*) message0) < 0)
	{
		fprintf (log, "Couldn't create thread2. Error number: %d   Errore message: %s \n" , errno , strerror(errno));
		fflush(log);
		exit(0);
	}

	pthread_t thread3;

	if( pthread_create( &thread3 , NULL , start_location , (void*) message0) < 0)
	{
		fprintf (log, "Couldn't create thread3. Error number: %d   Errore message: %s \n" , errno , strerror(errno));
		fflush(log);
		exit(0);
	}

	// join 1,2 threads
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	// start receiving transactions
	char *message4 = "Thread 2002";
	pthread_t thread4;

	if( pthread_create( &thread4 , NULL , recv_transacts , (void*) message4) < 0)
	{
		fprintf (log, "Couldn't create thread4. Error number: %d   Errore message: %s \n" , errno , strerror(errno));
		fflush(log);
		exit(0);
	}

	// join threads and terminate
	pthread_join(thread3, NULL);
	pthread_join(thread4, NULL);
	print_bst(); // write info on logbst file
	free(message0);
	return 0;
}



/* Try to connect to a monero node with timeout (to send 1001 and 1003 messages: threads 1,2,4), 1000 ms
 * connect() can block the execution
 * With poll and fcntl (without select)
*/
int connect_with_timeout(int sockfd, const struct sockaddr *addr, socklen_t addrlen, unsigned int timeout_ms) {
        int rc = 0;
        // Set O_NONBLOCK
        int sockfd_flags_before;
        if((sockfd_flags_before=fcntl(sockfd,F_GETFL,0)<0)) return -6;
        if(fcntl(sockfd,F_SETFL,sockfd_flags_before | O_NONBLOCK)<0) return -7;
        // Start connecting (asynchronously)
        do {
                if (connect(sockfd, addr, addrlen)<0) {
                        // Did connect return an error? If so, we'll fail.
                        if ((errno != EWOULDBLOCK) && (errno != EINPROGRESS)) {
                                rc = -1;
                        }
                        // Otherwise, we'll wait for it to complete.
                        else {
                                // Set a deadline timestamp 'timeout' ms from now (needed b/c poll can be interrupted)
                                struct timespec now;
                                if(clock_gettime(CLOCK_MONOTONIC, &now)<0) { rc=-2; break; }
                                struct timespec deadline = { .tv_sec = now.tv_sec,
                                                             .tv_nsec = now.tv_nsec + timeout_ms*1000000l};
                                // Wait for the connection to complete.
                                do {
                                        // Calculate how long until the deadline
                                        if(clock_gettime(CLOCK_MONOTONIC, &now)<0) { rc=-3; break; }
                                        int ms_until_deadline = (int)(    (deadline.tv_sec  - now.tv_sec)*1000l
                                                                        + (deadline.tv_nsec - now.tv_nsec)/1000000l);
                                        if(ms_until_deadline<0) { rc=0; break; }
                                        // Wait for connect to complete (or for the timeout deadline)
                                        struct pollfd pfds[] = { { .fd = sockfd, .events = POLLOUT } };
                                        rc = poll(pfds, 1, ms_until_deadline);
                                        // If poll 'succeeded', make sure it *really* succeeded
                                        if(rc>0) {
                                                int error = 0; socklen_t len = sizeof(error);
                                                int retval = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
                                                if(retval==0) errno = error;
                                                if(error!=0) rc=-4;
                                        }
                                }
                                // If poll was interrupted, try again.
                                while(rc<=-1 && errno==EINTR);
                                // Did poll timeout? If so, fail.
                                if(rc==0) {
                                        errno = ETIMEDOUT;
                                        rc=-5;
                                }
                        }
                }
        } while(0);
        // Restore original O_NONBLOCK state
        if(fcntl(sockfd,F_SETFL,sockfd_flags_before)<0) return -1;
        // Success rc=0
        return rc;
}

