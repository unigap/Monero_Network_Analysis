#include "main.h"
#include "bst.h"
#include "check1003.h"

struct in_addr dest_ip;
int s1;
struct timeval read_timeout2;

/* Hasieratu iterazioa zuhaitzaren erroarekin eta statraren arabera, 1003 exekutatu edo jarraitu egitura osoa iteratzen behin eta berriro
 * Ez du hasteko argumenturik behar, zuhaitza baizik... 1001 exekutatua izana (bestela 1001-en erantzuna [recvfrom] 0 da - konexioa itxi da...)
 * stat 0: Aurkitua izan da beste nodo bati 1001 bidaliz
 * stat 1: pareen zerrenda eskatu zaio (1001: Handshake)
 * stat 2: ez du PING erantzun -> mapatik ezabatuko da (egituratik ez)
 * stat 3: mapatik ezabatu da
*/
void * start1003(void *args) {
	// log fitxategia ireki
	FILE *log;
	log = fopen("log1003", "w");
	if(log == NULL) {
		printf("Errorea log1003 fitxategia sortzean: %s\n", strerror(errno));
		fflush(stdout);
		pthread_exit(NULL);
	}
	time_t denb = time(NULL) + atoi((char *)args);
	//fprintf(log,"Fitxategia sortuta\n");
	//fflush(log);
	while(1) {
		hautatu_knodoa(root, log, denb); // iteratu zuhaitza errekurtsiboki
		sleep(1);
	}
	fflush(log);
	fclose(log); // log1003
	pthread_exit(NULL);
}

/* Metodo errekurtsibo honekin PING bidaliko zaion nodoa aukeratuko da (pre-order)
 * stat 1 denean exekutatuko da, 1003 komandoa bidaliz (PING) konektatuta jarraitzen duen aztertzeko
 * Ez bada PING erantzuna jasotzen, 2 statrara pasa nodoa
*/
void * hautatu_knodoa(struct bst_ip *un, FILE *log, time_t denb) {
	long err = 0;
	time_t denb2 = time(NULL);
	if(denb < denb2) {
		fprintf(log, "TERMINATED\n");
		fflush(log);
		fclose(log);
		pthread_exit(NULL);
	}
	if(un == NULL) {
		//sleep(1);
		;//continue;
	}
	else {
		if(un->stat == 1) {
			//printf("%d %s\n", un->stat, inet_ntoa(un->nodip));
			//fflush(stdout);
			//pthread_mutex_lock(&(une->lock)); // blokeakorra (wait-ekin kontrolatu daiteke)
			//pthread_mutex_trylock(&(une->lock)); // ez blokeakorra
			//fprintf(log, "1003 to %s",inet_ntoa(un->nodip));
			//fflush(log);
			err = ( long ) konprobatu_ping(inet_ntoa(un->nodip), un->port, log);
			if(err > 0) {
				fprintf(log, B_RED"Error: %ld\n"RESET, err);
				fflush(log);
                		pthread_mutex_lock(&(un->lock));   // blokeakorra (wait-ekin kontrolatu daiteke)
				if(un->stat < 3) // ezabatuta ez badago
                			un->stat = 2; // ezin nodoarekin konektatu -> mapatik ezabatu
                		pthread_mutex_unlock(&(un->lock)); // askatu
				close(s1); // errorea gertatu denean socket-a itxi
			}
			//stat = konprobatu_ping(inet_ntoa(une->nodip), une->port, log);
			//pthread_cond_wait(&(une->cond), &(une->lock)); // signal-aren zain geratzeko
			//pthread_mutex_unlock(&(une->lock));
		}
		if(un->left != NULL) {
			hautatu_knodoa(un->left, log, denb);
		}
		if(un->right != NULL) {
			hautatu_knodoa(un->right, log, denb);
		}
	}
}

/* Levin protokoloaren 1003 komandoa bidaltzeko eta erantzuna jasotzeko metodoa
 * Itzulera kodeak zein errore gertatu den jakinaraziko du
 * Errorerik gertatzen bada nodoa mapatik ezabatuko da (2 statrara pasaz)
*/
void * konprobatu_ping(char *target, int port, FILE *log) {

	fprintf(log, "\nCreating socket... \t Destination node: %s %d\n", target, port);
	fflush(log);
	// Create a raw socket to send
        s1 = socket (AF_INET, SOCK_STREAM , IPPROTO_TCP); // s: socket descriptor
        if(s1 < 0)
        {
                fprintf(log, "Error creating socket. Error number: %d   Error message: %s \n" , errno , strerror(errno));
		fflush(log);
		return (void *) 1;
        }
	fprintf(log, "Socket descriptor: %d\n", s1); // egiaztatu desberdinak direla..
        fflush(log);

        read_timeout2.tv_sec = 1;
        //read_timeout2.tv_usec = 500000;
        int one = 1;
        setsockopt(s1, SOL_SOCKET, SO_RCVTIMEO, &read_timeout2, sizeof read_timeout2);
        setsockopt(s1, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &one, sizeof(one));


	// Datagram to represent the packet
        unsigned char datagram[329];  // 1003 header bidaltzeko
        unsigned char datagram1[50];  // 1003 data bidaltzeko

	// Levin header
        struct levhdr *lvh = (struct levhdr *) (datagram);

        // Payload
        struct data1003 *dat = (struct data1003 *) (datagram1);

	// Dest addr struct
        struct sockaddr_in  dest;

	if(inet_addr(target) == -1) // is not IP
    	{
		fprintf(log, "Bad IP");
		fflush(log);
		return (void *) 2;
    	}

	if(port == 0) {
		fprintf(log, "Bad port number");
		fflush(log);
		return (void *) 2;
	}

	dest_ip.s_addr = inet_addr( target );

	//sleep(1);
	//usleep(30000);
	// aurretik konprobatzen da
/*	pthread_mutex_lock(&(root->lock));
	while(statra_lortu(dest_ip) < 1) {
		//printf("Itxaroten..."); // behin bakarrik inprimatuko du
		pthread_cond_wait( &(root->cond), &(root->lock) ); // seinalearen zain geratuko da haria
		//;//sleep(1);
	}
	pthread_mutex_unlock(&(root->lock));
*/
	memset (datagram, 0, 329);     /* zero out the buffer */
	memset (datagram1, 0, 266);    /* zero out the buffer */

	// Fill in the Levin Header (PING request)
	lvh->sign = 0x0101010101012101;     // 8 bytes
	lvh->length = 0x0a;                 // 8 bytes
	lvh->exp_resp = 0x01;               // 1 byte request
	lvh->comm_cod = htonl(0xeb030000);  // 4 bytes
	lvh->retn_cod = htonl(0x01000000);  // 4 bytes (request!!! DOKU+)
	lvh->reserved = htonl(0x01000000);  // 4 bytes
	lvh->endchars = htonl(0x01000000);  // 4 bytes

	// Levin Data/Payload
	unsigned char datt[10] = {0x01, 0x11, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x00};
	memcpy(dat->d, datt, 10); // honela esleitu, bestela 0x00 interpretatzean bukatzen da

	int i;

	// Makina honen socket-aren informazioa zehaztu (portua: 38080)
	struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = INADDR_ANY;
	my_addr.sin_port = htons(38080);

	if (bind(s1, (struct sockaddr*) &my_addr, sizeof(my_addr)) == 0) {
		fprintf(log,"Port binded\t\t");
	}
	else {
		fprintf(log,"Couldn't bind port\n");
		fflush(log);
	}

	// helburuko nodoari buruzko informazioa bete
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = dest_ip.s_addr;
	dest.sin_port = htons( port );

/*	// 3-way handshake (SYN-ACK)
	if(connect(s1, (struct sockaddr *)&dest, sizeof(struct sockaddr)) < 0){
		fprintf(log, "Errorea helburuko nodoarekin konektatzean: %s \n", strerror(errno));
		fflush(log);
		//statra_aldatu(dest_ip, 2); // if
		return (void *) 4;
	}
*/
	// segundu 1 eman konektatzeko
	int kod = connect_with_timeout(s1, (struct sockaddr *)&dest, sizeof(struct sockaddr), 1000);

	if(kod >= 0){
		fprintf(log, " Connected: %d\n", kod);
		fflush(log);
	}
	else {
		fprintf(log, "Couldn't connect: %d\n", kod);
		fflush(log);
		return (void *) 3;
	}

	// Send the Levin PING (1003) request header packet
	int sizesend = sizeof(struct levhdr);
	if ( sendto (s1, datagram, sizeof(struct levhdr) , 0 , (struct sockaddr *) &dest, sizeof (dest)) < 0)
	{
		fprintf (log, "Couldn't send 1003 header. Error number: %d  Error message: %s \n" , errno , strerror(errno));
		fflush(log);
		return (void *) 5;
	}

	char * m = inet_ntoa(dest.sin_addr);
	fprintf(log, "1003 request header sent\t\t Packet length: %d (10), %02x (16).\n", sizesend, sizesend);
	fflush(log);

	sizesend = sizeof(struct data1003);
	// Send the Levin PING (1003) request data
	if ( sendto (s1, datagram1,  sizeof(struct data1003) , 0 , (struct sockaddr *) &dest, sizeof (dest)) < 0)
	{
		fprintf (log, "Couldn't send 1003 data. Error number: %d   Error message: %s \n" , errno , strerror(errno));
		fflush(log);
		return (void *) 6;
	}

	//m = inet_ntoa(dest.sin_addr);
	fprintf(log, "1003 request data sent \t\t\t Packet length: %d (10), %02x (16).\n", sizesend, sizesend);
	fflush(log);
	//printf("==========================================================================\n");


	// RECV
	//ez bada ezer jasotzen edo mezuaren tamaina 0 -> nodoa ezabatu
	unsigned char *recbuf1 = (unsigned char*)malloc(72); // (38+33=71 bytes)
	unsigned char *recbuf2 = (unsigned char*)malloc(38);
	//unsigned char *recbufh = (unsigned char*)malloc(33); // levin header
	int sizedd = sizeof(dest);
	int b1 = recvfrom(s1, recbuf1, 72, 0, (struct sockaddr *) &dest, &sizedd);
	//int bh = recvfrom(s1, recbufh, 33, 0, (struct sockaddr *) &dest, &sized);
	int b2;

	if (b1 <= 0) {
                fprintf(log, "Couldn't receive response (recvfrom length %d): %s \n", b1, strerror(errno));
		fflush(log);
		return (void *) 7;
        }

	char leng1[7];
	sprintf(leng1, "0x%.2x%.2x", recbuf1[9],recbuf1[8]);
	long lend1 = strtol(leng1, NULL, 16);

	if(b1 == 33) { // goiburukoa soilik bidali bada
		b2 = recvfrom(s1, recbuf2, 38, 0, (struct sockaddr *) &dest, &sizedd);
		if(b2 <= 0) return (void *) 7;
	}

	fprintf(log, "1003 response received \t\t\t Packet length: %ld (10), %02lx (16)\t\t '%s_1003' temporary file\n", lend1,lend1, target);
	fflush(log);
	close(s1); // socket-a itxi

	char fileizena1[50];
	strcpy(fileizena1, target);
	strcat(fileizena1, "_1003");
	FILE *em = fopen(fileizena1, "w");
	if(em == NULL)
	{
		fprintf(log, "Error creating output file: %s\n", strerror(errno));
		fflush(log);
		return (void *) 8;
	}
	// mezua fitxategian idatzi
	fwrite(recbuf1, sizeof(char), b1, em);

	if(b1 == 33){
//		fwrite(recbuf2, sizeof(char), (int)lend1, em);
		fwrite(recbuf2, sizeof(char), b2, em);
	}
//	fflush(stdout);

	fflush(em);
	fclose(em);
	unlink(fileizena1); // XX.XXX.XX.XXX_1003 ezabatu
	fflush(log);
	free(recbuf1);
	free(recbuf2);
	//fclose(log);
	//pthread_exit(NULL);
	return (void *) 0; // errorerik gabe TERMINATED da
}

