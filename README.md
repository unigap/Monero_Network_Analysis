# Monero Network Analysis

<!--Monero, a digital currency that is secure, private, and untraceable.-->

[What is Monero?](https://www.getmonero.org/get-started/what-is-monero/)

It's a open-source cryptocurrency project focused on private and censorship-resistant transactions. That is because Monero uses various privacy-enhancing technologies to hide information of the blockchain and ensure the anonymity of its users.

Learn more about Monero: [Moneropedia](https://www.getmonero.org/resources/moneropedia/)

See also the documentation of the application level protocol that allows <b>peer-to-peer</b> communication between Monero nodes: [Levin protocol](https://github.com/monero-project/monero/blob/master/docs/LEVIN_PROTOCOL.md)

<h2><b> This is an educational project using Levin protocol</b></h2>

 Execution of this program initializes <b>monero peer-to-peer network analisys</b>:
 *   Main thread: Initialization of all threads and catch two signals due to exit: ```SIGINT``` and ```SIGTERM```

     When one of those signal is received (also executing ```terminate.sh```), program writes Monero node list (binary search tree) ⟶ ```logbst``` file (information of each node) and terminate.
     
     - 1<sup>st</sup> thread: For each node request the peer list with 1001 message (+ or - 250 [IP, Port]) and store the information received on binary search tree.

       Record events ⟶ ```log1001``` file
     - 2<sup>nd</sup> thread: Check with 1003 message if each node still available. If not response is received, then node will be removed from map.
     
       Record events ⟶ ```log1003``` file
     - 3<sup>rd</sup> thread: Get coordenates of each node and print them on standard output (to comunicate by pipe with the other program (```locate.py```)
     
       Record events ⟶ ```logmap``` file
     - 4<sup>th</sup> thread: Wait for 2002 message (recv 2002 notification, count transactions: 500 byte each transaction)
     
       Record events ⟶ ```log2002``` file

 * Compile:
```
$ gcc main.c bst.c request1001.c check1003.c location.c recv2002.c -lpthread -o main
```
 * Run:     
```
$ ./main <IP> <PORT> <time12>
```

   ·    IP: destination monero node IP address
   ·    PORT: natural int, often number 18080
   ·    time12: time limit to request 1001 and 1003 (threads 1,2) and then receive transactions (thread 4) from those nodes
 
 Combine this execution output with the Python program to locate nodes on map (```locate.py```):

 Help: 
```
$ ./main # Displays monero seed nodes to start execution use one of them (help message)
 ```
 Run: 
```
$ ./main 212.83.175.67 18080 10 | python locate.py # 10 seg collecting nodes (locating them on map) and then receiving transactions from available nodes
```

Execute terminate.sh script to terminate the execution of the main program and send quit method to Python program:
```
$ sh terminate.sh
```

[Monero Github project](https://github.com/monero-project/monero)

[Monero seed nodes](https://community.xmr.to/xmr-seed-nodes)

[Monerodocs](https://monerodocs.org/)

[MoneroWorld](https://moneroworld.com/)


<h2><b> Dependencies: </b></h2>

This project is tested on Ubuntu and Debian

To install all <b>dependencies</b> and start the execution you can follow this guide:

```
$ sudo apt-get install git
$ sudo apt-get install gcc
$ sudo apt-get install geoip-bin
$ git clone https://github.com/unigap/Monero_Network_Analysis
$ cd Monero_Network_Analysis/src/
$ gcc main.c bst.c request1001.c check1003.c location.c recv2002.c -lpthread -o main
$ ./main # print help message
<!--```
Output:
```-->
Destination IP, port and time limit for requests (seg) are necessary to initialize the execution!
Destination node can be selected from the following Monero seed-nodes list: 
  - 66.85.74.134 18080
  - 88.198.163.90 18080
  - 95.217.25.101 18080
  - 104.238.221.81 18080
  - 192.110.160.146 18080
  - 209.250.243.248 18080
  - 212.83.172.165 18080
  - 212.83.175.67 18080
```
Next we will install dependencies for the python program:
```
$ sudo apt-get install python3-pip
$ pip3 install numpy
$ pip3 install pandas
$ pip3 install matplotlib
$ pip3 install libgeos-dev
$ sudo apt-get install python3-gi-cairo
$ pip3 install Cython
$ pip3 install --upgrade pip
$ pip3 install --user https://github.com/matplotlib/basemap/archive/master.zip



```
Now you can test the python program
```
$ python3 locate.py
$ python3 locate.py 0.5
```

Finally, combine the execution of both programs with <b>pipe</b>:
```
$ ./main 212.83.175.67 18080 10 | python locate.py
```

<img src="..//main/imgs/mapa.png" alt="Monero nodes around the world - Example of execution 1">
<img src="../main/imgs/map.svg" alt="Monero nodes around the world - Example of execution 2">

