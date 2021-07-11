# Monero Network Analysis


## Table of Contents

  - [Introduction](#introduction)
  - [Educational project using Levin protocol](#educational-project-using-levin-protocol)
    - [Reverse engineering](#reverse-engineering)
    - [Main program (C)](#main-program-c)
    - [Secondary program (Python)](#secondary-program-python)
    - [Execute both using pipe](#execute-both-using-pipe)
  - [Dependencies](#dependencies)
  - [Log files](#log-files)
  - [Output images](#output-images)
  - [Conclusion](#conclusion)
  - [Support](#support)
  - [Related sites](#related-sites)


<!--Monero, a digital currency that is secure, private, and untraceable.-->

## Introduction

[What is Monero?](https://www.getmonero.org/get-started/what-is-monero/)

It's an open-source cryptocurrency project focused on private and censorship-resistant transactions. That is because Monero uses various privacy-enhancing technologies to hide information of the blockchain and ensure the anonymity of its users.

Privacy enhancing technologies: [Stealth Address](https://www.getmonero.org/resources/moneropedia/stealthaddress.html), [Ring Signatures](https://www.getmonero.org/resources/moneropedia/ringsignatures.html), [Ring Confidential Transactions](https://www.getmonero.org/resources/moneropedia/ringCT.html), [Support over TOR/I2P](https://github.com/monero-project/monero/blob/master/docs/ANONYMITY_NETWORKS.md) and [Dandelion++](https://localmonero.co/knowledge/monero-dandelion)

Learn more about Monero: [Moneropedia](https://www.getmonero.org/resources/moneropedia/)

See also the documentation of the application level protocol that allows **peer-to-peer** communication between Monero nodes: [Levin Protocol](https://github.com/monero-project/monero/blob/master/docs/LEVIN_PROTOCOL.md)


## Educational project using Levin Protocol

With this project I intend to analyze Monero P2P network; cryptocurrency and privacy techniques, and application layer protocols using network programming (in the C programming language). Moreover, I want to contribute to the general knowledge about Monero and its great features that protect users.

### Reverse engineering

The first step was to execute ```monerod``` and check the TCP communication with ```tcpflow``` and ```hexdump``` as executing the scripts on the folder called 'extract_levin_communication'. This needs superuser privileges and maybe install ```tcpflow``` and ```hexdump``` tools.

Example:
```
$ sh init.sh 90 eth0          # Listen for 90 seconds the Monero P2P communication (port 18080), specifying the interface eth0 (by default wi-fi interface)
tcpflow: listening on eth0
Terminating tcpflow process
Processing monero packets...  Output: em<i> & comm_em<i> 
Extracting IPs from data...   Output: ipak 
Getting geolocation of IPs... Output: iplocs 
```

We can analyze all this files and also we can run the following command to know **which Levin messages** (identified with command number) were received during the tcpflow execution:
```
$ grep -E "Command number:" comm_em* | cut -d ':' -f 3 | sort | uniq
1001 
1002 
1007  
2002 
2003 
2004 
2006 
2007 
2008 
```

In this way, we can see the sending process performed in each case and use it for develope a tool designed to extract Monero nodes and place them on a world map.


### Main program (C)

 Execution of this program initializes **monero peer-to-peer network analisys**:
 *   Main thread: Initialization of all threads and catch two signals due to exit: ```SIGINT``` and ```SIGTERM```

     When one of those signal is received (also executing ```terminate.sh``` script), the main program writes Monero node list (stored in a binary search tree) ⟶ ```logbst``` file (information of each node) and terminate.
     
     - 1<sup>st</sup> thread: Request recursively the peer list of each Monero node with 1001 message (+ or - 250 [IP, Port]) and store the information received on binary search tree.

       Record events ⟶ ```log1001``` file
     - 2<sup>nd</sup> thread: Check with 1003 message if each node is still available. If no response is received, then node will be removed from the map.
     
       Record events ⟶ ```log1003``` file
     - 3<sup>rd</sup> thread: Get coordenates of each node and print them on standard output (to comunicate by pipe with the other program (```locate.py```)
     
       Record events ⟶ ```logmap``` file
     - 4<sup>th</sup> thread: Wait for 2002 message (recv 2002 notification, "count" transactions + sec. factor: 500 byte each transaction)
     
       Record events ⟶ ```log2002``` file

 * Compile:
```
$ gcc main.c bst.c request1001.c check1003.c location.c recv2002.c -lpthread -o main
```

 * Run:     
```
$ ./main <IP> <PORT> <time12>
```

> - IP: destination monero node IP address
> - PORT: destination node's port number (natural int), often number 18080
> - time12: time limit to request 1001 and 1003 messages (1<sup>st</sup> and 2<sup>nd</sup> threads) and then start receiving transactions (with 4<sup>th</sup> thread) from those nodes that were available at the moment.


### Secondary program (Python)

```locate.py``` program reads the standard input to locate nodes on the map with the following format:

```
<Method> <Longitude> <Latitude> <IP> <City/Tr>
```
> - Method:
>     * ```a```: add node on the map
>     * ```r```: remove node from the map
>     * ```t```: add transacts to an existing node
>     * ```q```: quit the execution (no need for arguments)
> - Longitude: value between -180.0 and 180.0
> - Latitude: value between -90.0 and 90.0
> - IP: IP address of the node
> - City/Tr: city (with ```a``` and ```r``` methods) or number of transactions (with ```t``` method)


### Execute both using pipe

Combine the execution of the ```main``` program with the Python program to locate the discovered nodes on the map (```locate.py```).

Some examples of execution: 
```
$ ./main    # Print help message
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
```
$ ./main 212.83.175.67 18080 10 | python3 locate.py    # 10 seconds to collect nodes from Monero P2P network + locate them on map
                                                       # and then receive transactions from available nodes  + locate them on map
Reading coordinates...
<Method> <Longitude> <Latitude> <IP> <City/Tr>:
Receiving transactions...
Transactions received: 2 ( 3.93.33.238 2 )
...
```

Run ```terminate.sh``` script to terminate the execution of the main program and send quit method (```q```) to Python program (and thus completes all output files):
```
$ sh terminate.sh
```


## Dependencies

This project is tested on Ubuntu and Debian

To install all dependencies and start the execution you can follow these steps:

```
$ sudo apt-get install git
$ sudo apt-get install gcc
$ sudo apt-get install geoip-bin
$ git clone https://github.com/unigap/Monero_Network_Analysis
$ cd Monero_Network_Analysis/src/
$ gcc main.c bst.c request1001.c check1003.c location.c recv2002.c -lpthread -o main
```

Next we will install dependencies for the Python program (it doesn't matter if it's python or python3):

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

Now you can test the Python program ```locate.py```:

```
$ python3 locate.py
$ python3 locate.py 0.5    # with a lower resolution
```

Finally, combine the execution of both programs with **pipe**:
```
$ ./main 212.83.175.67 18080 10 | python3 locate.py
```

## Log files

Each process stores execution steps and execution errors in a file:

* ```log1001```: shows the handshake requests and responses (for each node)
  ```
  Creating socket... 	 Destination node: 212.83.175.67 18080
  Socket descriptor: 8
  Port binded		 Connected
  1001 request header sent		 Packet length: 33 (10),    21 (16).
  1001 request data sent 			 Packet length: 226 (10),   e2 (16).
  1007 request received (ignore) 		 Packet length: 10 (10),    0a (16).
  1001 response header received 		 Packet length: 33 (10),    21 (16).
  Receiving 1001 response data... 	 Packet length: 15494 (10), 3c86 (16)  '212.83.175.67' temporary file.
  ```
* ```log1003```: shows the ping requests and responses (for each node)
  ```
  Creating socket... 	 Destination node: 212.83.175.67 18080
  Socket descriptor: 9
  Port binded		 Connected: 1
  1003 request header sent		 Packet length: 33 (10), 21 (16).
  1003 request data sent 			 Packet length: 10 (10), 0a (16).
  1003 response received 			 Packet length: 38 (10), 26 (16)    '212.83.175.67_1003' temporary file.
  ```
* ```logmap```: shows the coordinates of each node
  ```
  Getting coordinates... Node: 212.83.175.67 18080
  In order to obtain location information run the following script: 'sh get_maxmind.sh'
  The command to be executed in the pipe:
  geoiplookup -f ./geoip/maxmind4.dat 212.83.175.67 | cut -d ',' -f 5,7-8 | awk -F ',' '{print $3, $2, $1}' | sed s/,//g 
  Pipe opened.
  Lon: 2.338700	Lat: 48.858200	City:  N/A
  ```
* ```log2002_<id>```: shows the transactions received from each node and sub-thread (with id)
  ```
  
  ```
* ```logbst```: shows the binary search tree (main data-structure) in-order traversal 
  ```
  
  ```
* ```mapinfo```: shows the nodes that were added on the map before the end of execution
  ```
  
  ```


## Output images

<img src="../main/imgs/map.svg" alt="Monero nodes around the world - Example of execution 1 (svg)">
<img src="../main/imgs/mapa.png" alt="Monero nodes around the world - Example of execution 2 (png)">

## Conclusion


## Support

Monero donation address: ```4BDFKWqasRBSPWhRxuRbtpKD4dubfi3htXtdzQQHvfDE8Jjgp6cqHN9gRBDncfU9G2FPeRz3wx35TCmJKkv8Ma8SKLyXmUb```


## Related sites

[Monero Website](https://www.getmonero.org/)

[Monero Github project](https://github.com/monero-project/monero)

[Monero seed nodes](https://community.xmr.to/xmr-seed-nodes)

[Monerodocs](https://monerodocs.org/)

[MoneroWorld](https://moneroworld.com/)


