#include "main.h"
#include "bst.h"
#include "recv2002.h"

struct in_addr dest_ip;
struct timeval read_timeout4;

// Creates 20 threads to receive transactions (2002 messages) from available monero nodes
void * recv_transacts(void *args) {
	// create log file
	FILE *log;
	log = fopen("log2002", "w");
	if(log == NULL) {
		printf("Error creating log2002 file: %s\n", strerror(errno));
		fflush(stdout);
		pthread_exit(NULL);
	}
	int nodokop = count_avail(root);//
	fprintf(log,"Receiving transactions...\nAvailable nodes: %d\n", nodokop);
	fflush(log);
	pthread_t trhari[20];
	int i=0;
	int j=0;
	char mezu[20][7];
//	for(i=0; i<nodokop; i++) {
	for(i=0; i<20; i++) {
		sprintf(mezu[i], "%d", i);

		if( pthread_create( &trhari[i] , NULL , hasieratu_trnodoa , (void*) mezu[i]) < 0)
		{
			fprintf (log, "Couldn't create thread[%d]. Error number: %d   Error message: %s \n" , i, errno , strerror(errno));
			fflush(log);
		}
		fprintf(log, "thread4[%d] created\n", i);
		fflush(log);
	}

//	for(j=0; j<nodokop; j++) {
	for(j=0; j<20; j++) {
		pthread_join(trhari[j], NULL);
		fprintf(log, "%d. thread terminated\n",j);
	}

	fprintf(log, "Process terminated\n");
	fclose(log);
//	pthread_exit(NULL); // 3. haria geratuko da exekuzioan Ctrl+c sakatu arte
// edo			      // prozesu osoa bukatu q pantailan inprimatuz
	print_bst(); // berdin amaitu exekuzioa: logbzb idatzi
	printf("q\n");   // eta python kodea bukatzeko q inprimatu irteera estandarrean
	fflush(stdout);
	exit(0); // 3. haria bukaraziko du
}


void * hasieratu_trnodoa(void *args) {
	struct bst_ip *has = root;
	FILE *log;
	char izen[15];

	sprintf(izen, "log2002_%d", atoi(args));

	log = fopen(izen, "w");
	if(log == NULL) {
		printf("Error opening %s file: %s\n", izen, strerror(errno));
		fflush(stdout);
		pthread_exit(NULL);
	}

	while(1) { // interrupt (Ctrl+c) or kill (terminate.sh)
		fprintf(log, "=========\n");
		fflush(log);
		hautatu_tnodoa(has, atoi(args), log); // esleitu nodo atzigarri bat hari bakoitzari
	}
	fclose(log);

}


void * hautatu_tnodoa(struct bst_ip *un, int id, FILE *log) {
	long err = 0;
	int ego=0;

	if(un == NULL) {
		//sleep(1);
		fprintf(log, "There are not available nodes for %d thread\n", id); // ez sortu haria, nodoa NULL bada
		fflush(log);
		fclose(log);
		// return (void *) 1; // aurreko begiztara bueltatu.. ez da gertatuko
		pthread_exit(NULL); // haria itxi: kontatu ditugun nodo atzigarriak kontatu_mapan.c-k baten bat ezabatu badu koordenatuak lortu ezinagatik (oso arraroa), beste bi hariek prozesua bukatuta baitute hau hasterakoan
	}
	else {
//		////if(un->stat < 2 && un->tkop < 0) { // hasieratu haria
//		if(un->stat < 2) { // hasieratu haria nodo atzigarriarekin
		pthread_mutex_lock(&(un->lock));		// blokeatu nodoa
		if(un->stat < 2 && un->lon != 0 && un->lat != 0) { // hasieratu haria nodo atzigarriarekin eta mapan kokatuta
			ego = un->stat;
			un->stat = 4;    // erreserbatu jaso2002 prozesurako
			pthread_mutex_unlock(&(un->lock));         // askatu nodoa (statra horretan ezin atzitu)
			if(un->tkop < 0) un->tkop = 0; 		   // transakzioen jasoketa prozesua hasieratuta (bestela -1 delako)
			fprintf(log,"Output file name: %s_2002_%d\n", inet_ntoa(un->nodip), id); // ez sortu haria, nodoa NULL bada
			fflush(log);
			err = ( long ) jaso2002(un, log, id);
			pthread_mutex_lock(&(un->lock));		// blokeatu nodoa statra aldatzeko
			un->stat = ego; // hasierako statrara itzuli beste batzuk prozesua jarraitzeko
			pthread_mutex_unlock(&(un->lock)); 	// askatu nodoa
			if(err > 0) {
				fprintf(log, B_RED"Error on %d thread, code: %ld\n"RESET, id, err);
				fflush(log);
			}
		}
		else {
			pthread_mutex_unlock(&(un->lock)); 	// askatu nodoa
		}

		if(un->left != NULL) {
			hautatu_tnodoa(un->left, id, log);	// jarraitu bilaketa ezker azpi-zuhaitzarekin
		}
		if(un->right != NULL) {
			hautatu_tnodoa(un->right, id, log);	// jarraitu bilaketa eskuin azpi-zuhaitzarekin
		}

    }

}


void * jaso2002(struct bst_ip *un, FILE *log, int id) {

	// Create a socket to send
	int s2 = socket (AF_INET, SOCK_STREAM , IPPROTO_TCP); // s: socket descriptor
	if(s2 < 0)
	{
		fprintf(log, "Error creating socket, Error number: %d  Error message: %s \n" , errno , strerror(errno));
		fflush(log);
		return (void *) 1;
	}
	fprintf(log, "Socket descriptor: %d\n", s2); // egiaztatu desberdinak direla..
	fflush(log);

	read_timeout4.tv_sec = 5;
	//read_timeout4.tv_usec = 500000;
	int one = 1;
	setsockopt(s2, SOL_SOCKET, SO_RCVTIMEO, &read_timeout4, sizeof read_timeout4);
	setsockopt(s2, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &one, sizeof(one));

	// Datagram to represent the packet
	unsigned char datagram[329];  // 1001 header bidaltzeko
	unsigned char datagram1[266];  // 1001 data bidaltzeko

	// Levin header
	struct levhdr *lvh = (struct levhdr *) (datagram);

	// Payload
	struct datatr *dat = (struct datatr *) (datagram1);

	// Dest addr struct
	struct sockaddr_in  dest;

	char *target = inet_ntoa(un->nodip);
	int port = un->port;

	if(inet_addr(target) == -1) // is not IP
	{
		fprintf(log, "Bad IP");
		fflush(log);
		close(s2);
		return (void *) 2;
	}

	if(port <= 0) {
		fprintf(log, "Bad port number");
		fflush(log);
		close(s2);
		return (void *) 3;
	}

	dest_ip.s_addr = inet_addr( target );
	memset (datagram, 0, 329);     /* zero out the buffer */
	memset (datagram1, 0, 266);    /* zero out the buffer */

	// Fill in the Levin Header (PING request)
	lvh->sign = 0x0101010101012101;     // 8 bytes
	lvh->length = 0xe2;                 // 8 bytes
	lvh->exp_resp = 0x01;               // 1 byte request
	lvh->comm_cod = htonl(0xe9030000);  // 4 bytes
	lvh->retn_cod = htonl(0x00000000);  // 4 bytes
	lvh->reserved = htonl(0x01000000);  // 4 bytes
	lvh->endchars = htonl(0x01000000);  // 4 bytes

	// Levin Data/Payload
	unsigned char datt[226] = {0x01, 0x11, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x08, 0x09, 0x6e, 0x6f, 0x64, 0x65, 0x5f, 0x64, 0x61, 0x74, 0x61, 0x0c, 0x10, 0x0a, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x5f, 0x74, 0x69, 0x6d, 0x65, 0x05, 0x64, 0x93, 0x16, 0x60, 0x00, 0x00, 0x00, 0x00, 0x07, 0x6d, 0x79, 0x5f, 0x70, 0x6f, 0x72, 0x74, 0x06, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x6e, 0x65, 0x74, 0x77, 0x6f, 0x72, 0x6b, 0x5f, 0x69, 0x64, 0x0a, 0x40, 0x12, 0x30, 0xf1, 0x71, 0x61, 0x04, 0x41, 0x61, 0x17, 0x31, 0x00, 0x82, 0x16, 0xa1, 0xa1, 0x10, 0x07, 0x70, 0x65, 0x65, 0x72, 0x5f, 0x69, 0x64, 0x05, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x0c, 0x70, 0x61, 0x79, 0x6c, 0x6f, 0x61, 0x64, 0x5f, 0x64, 0x61, 0x74, 0x61, 0x0c, 0x10, 0x15, 0x63, 0x75, 0x6d, 0x75, 0x6c, 0x61, 0x74, 0x69, 0x76, 0x65, 0x5f, 0x64, 0x69, 0x66, 0x66, 0x69, 0x63, 0x75, 0x6c, 0x74, 0x79, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x63, 0x75, 0x72, 0x72, 0x65, 0x6e, 0x74, 0x5f, 0x68, 0x65, 0x69, 0x67, 0x68, 0x74, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x74, 0x6f, 0x70, 0x5f, 0x69, 0x64, 0x0a, 0x80, 0x41, 0x80, 0x15, 0xbb, 0x9a, 0xe9, 0x82, 0xa1, 0x97, 0x5d, 0xa7, 0xd7, 0x92, 0x77, 0xc2, 0x70, 0x57, 0x27, 0xa5, 0x68, 0x94, 0xba, 0x0f, 0xb2, 0x46, 0xad, 0xaa, 0xbb, 0x1f, 0x46, 0x32, 0xe3, 0x0b, 0x74, 0x6f, 0x70, 0x5f, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x08, 0x01};
	memcpy(dat->d, datt, 226); // honela esleitu, bestela 0x00 interpretatzean bukatzen da

	int i;

	// Makina honen socket-aren informazioa zehaztu (portua: 28080)
	struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = INADDR_ANY;
	my_addr.sin_port = htons(28080);

	if (bind(s2, (struct sockaddr*) &my_addr, sizeof(my_addr)) == 0) {
		fprintf(log,"Port binded\t\t");
	}
	else {
		fprintf(log,"Couldn't bind port\n");
		fflush(log);
		close(s2);
		return (void *) 4;
	}

	// helburuko nodoari buruzko informazioa bete
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = dest_ip.s_addr;
	dest.sin_port = htons( port );

	int kod = connect_with_timeout(s2, (struct sockaddr *)&dest, sizeof(struct sockaddr), 1000); // 1 s

	if(kod >= 0){
		fprintf(log, "Connected: %d\n", kod);
		fflush(log);
//		;
	}
	else {
		fprintf(log, "Couldn't connect: %d\n", kod);
		fflush(log);
		close(s2);
		return (void *) 5;
	}

	// Send the Levin handshake (1001) request header packet
	int sizesend = sizeof(struct levhdr);
	if ( sendto (s2, datagram, sizeof(struct levhdr) , 0 , (struct sockaddr *) &dest, sizeof (dest)) < 0)
	{
		printf("Couldn't send 1001 header. Error number: %d  Error message: %s \n" , errno , strerror(errno));
		fflush(stdout);
		close(s2);
		return (void *) 6;
	}

	char * m = inet_ntoa(dest.sin_addr);
	fprintf(log, "1001 request header sent\t\t Packet length: %d (10), %02x (16).\n", sizesend, sizesend);
	fflush(log);

	sizesend = sizeof(struct datatr);
	// Send the Levin handshake (1001) request data
	if ( sendto (s2, datagram1,  sizeof(struct datatr) , 0 , (struct sockaddr *) &dest, sizeof (dest)) < 0)
	{
		fprintf (log, "Couldn't send 1001 data. Error number: %d   Error message: %s \n" , errno , strerror(errno));
		fflush(log);
		close(s2);
		return (void *) 7;
	}

	fprintf(log, "1001 request data sent \t\t Packet length: %d (10), %02x (16).\n", sizesend, sizesend);
	fflush(log);

	unsigned char recbuf0[33]; // header
	unsigned char *recbuf1; // data
	int sized = sizeof(dest);
	int b0=0;
	int b1=0;

	char comc[7];
	long comn=0;
	char tamc[9];
	long tamn=0;

	char fileizena[50];
	strcpy(fileizena, target);
	strcat(fileizena, "_2002_");
	char cid[6];
	sprintf(cid, "%d", id);
	strcat(fileizena, cid);
	FILE *em = fopen(fileizena, "w");
	if(em == NULL)
	{

		fprintf(log, "Error creating output file: %s\n", strerror(errno));
		fflush(log);
		close(s2);
		return (void *) 8;
	}

	int ttam=0;

	// receive 2002 message and count transactions -> send to map program (printf("t <lon> <lat> <IP> <tkop>\n");)
	while(1) {
		b0 = recvfrom(s2, recbuf0, 33, 0, (struct sockaddr *) &dest, &sized);
		if(b0 > 0) {
//			fprintf(log, "Tamaina: %d\t", b0);
//			fflush(log);
			if(b0 == 8) { // header-a bukatzeko 25 byte faltan
				fwrite(recbuf0, sizeof(char), 8, em);
				b0 = recvfrom(s2, recbuf0, 25, 0, (struct sockaddr *) &dest, &sized);
	            		//if(b0 == 25)
				sprintf(comc, "0x%.2x%.2x", recbuf0[10], recbuf0[9]);
				comn = strtol(comc, NULL, 16);
				sprintf(tamc, "0x%.2x%.2x%.2x", recbuf0[2], recbuf0[1], recbuf0[0]);
				tamn = strtol(tamc, NULL, 16);

				fprintf(log,"%ld message received from %s:%d \t Packet length: %ld \n", comn, target, un->port, tamn);
				fflush(log);
				if(comn == 2002) {
					ttam=(tamn/500);
					un->tkop+=ttam; // suposatuz transakzio bakoitzak 500 byte (segurtasunarekin tamaina handiagoa...)
					fprintf(log,"2002 message received from %s:%d      tkop: %d\n", target, un->port, un->tkop);
					fflush(log);
					printf("t %f %f %s %d\n", un->lon, un->lat, target, ttam); // print current info to map program
					fflush(stdout);
				}
				// header-a fitxategian idatzi
				fwrite(recbuf0, sizeof(char), 25, em);

				// jaso data
				recbuf1 = (unsigned char*)malloc(tamn);
				b1=1;
				while(b1 > 0) {
					b1 = recvfrom(s2, recbuf1, tamn, 0, (struct sockaddr *) &dest, &sized);
					if (b1 <= 0){
						fprintf(log, "Error when receiving message (recvfrom, length %d): %s \n", b1, strerror(errno));
						fflush(log);
						close(s2);
						free(recbuf1);
						fclose(em);
						unlink(fileizena); // XX.XXX.XX.XXX_2002 remove file
						return (void *) 11;
					}
					fwrite(recbuf1, sizeof(char), b1, em);
					if(b1 >= tamn){
						break;
					}
					tamn-=b1; // jasotzeko falta diren byteak kontrolatu

				}
			}
			else if(b0 < 33) {
				;
			}
			else {
				sprintf(comc, "0x%.2x%.2x", recbuf0[18],recbuf0[17]);
				comn = strtol(comc, NULL, 16);
				sprintf(tamc, "0x%.2x%.2x%.2x", recbuf0[10], recbuf0[9],recbuf0[8]);
				tamn = strtol(tamc, NULL, 16);

				fprintf(log, "%ld message received from %s:%d \t Packet length: %ld \n", comn, target, un->port, tamn);
				fflush(log);
				if(comn == 2002) {
					ttam=(tamn/500);
                                        un->tkop+=ttam;
					fprintf(log,"2002 message received from %s:%d      tkop: %d\n", target, un->port, un->tkop);
					fflush(log);
					printf("t %f %f %s %d\n", un->lon, un->lat, target, ttam); // // print current info to map program
					fflush(stdout);
				}
				// header-a fitxategian idatzi
				fwrite(recbuf0, sizeof(char), 33, em);

				// jaso data
				recbuf1 = (unsigned char*)malloc(tamn);
				b1=1;
				while(b1 > 0) {
					b1 = recvfrom(s2, recbuf1, tamn, 0, (struct sockaddr *) &dest, &sized);
					if (b1 <= 0){
						fprintf(log, "Error when receiving message (recvfrom, length %d): %s \n", b1, strerror(errno));
						fflush(log);
						close(s2);
						free(recbuf1);
						fclose(em);
						unlink(fileizena); // XX.XXX.XX.XXX_2002 ezabatu
						return (void *) 9;
					}
					fwrite(recbuf1, sizeof(char), b1, em);
					if(b1 >= tamn){
						break;
					}
					tamn-=b1; // jasotzeko falta diren byteak kontrolatu

		                }
//				fprintf(log, "\n======\n");
//				fflush(log);

			}
		}
		else {
			fprintf(log, "Couldn't receive message, length: %d  err: %s\n", b0, strerror(errno));
			fflush(log);
			fclose(em);
			unlink(fileizena); // XX.XXX.XX.XXX_2002 ezabatu
			close(s2);
//			if(comn != 2002) printf("\033[1A"); // bueltatu kursorea posizio berdinera (lerro bat gorago)
//			printf("\033[1A"); // bueltatu kursorea posizio berdinera (lerro bat gorago)
			return (void *) 10;
		}

	}

	// ez da hona iristen (mezurik jasotzen ez duenean return 10...)
	fclose(em);
	unlink(fileizena); // XX.XXX.XX.XXX_2002 ezabatu
	free(recbuf1);
	pthread_exit(NULL);
	close(s2);
}







