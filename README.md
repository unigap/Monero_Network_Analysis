# Monero Network Analysis
<b> This is a educational project </b>

 Execution of this program initializes monero peer-to-peer network analisys:
 *   Main thread: This thread initialize all threads and catch two signals: SIGINT and SIGTERM -> write logbst file (nodes info).
     - 1st thread: For each node request the peer list with 1001 message (+ or - 250 [IP, Port]) and store the information received on binary search tree. -> log1001
     - 2nd thread: Check with 1003 message if each node still available. If not response is received, then node will be removed from map.                  -> log1003
     - 3rd thread: Get coordenates of each node and print them on standard output (to comunicate by pipe with the other program: locate.py)                -> logmap
     - 4th thread: Wait for 2002 message (recv 2002 notification, count transactions: 500 byte each transaction)                                           -> log2002

 * Compile:
 ```sh
 gcc main.c bst.c request1001.c check1003.c location.c recv2002.c -lpthread -o main
 ```
 * Run:     
 ```c
 ./main <IP> <PORT> <time12>
 ```
 * IP: monero destination node IP address
 * PORT: natural int, often number 18080
 * time12: time limit to request 1001 and 1003 (threads 1,2) and then receive transactions (thread 4) from those nodes
 
 Combine this execution output with the Python program to locate nodes on map (locate.py):

Examples:
 Help: 
 ```
 ./main # Displays monero seed nodes to start execution use one of them 
 ```
 Run: 
 ```
 ./main 212.83.175.67 18080 10 | python locate.py # 10 seg collecting nodes (locating them on map) and then receiving transactions from available nodes
```
 SEED_NODES = https://community.xmr.to/xmr-seed-nodes



<b> Dependencies: </b>


