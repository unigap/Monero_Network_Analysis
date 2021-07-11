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
  - [Related sites](#related-sites)


<!--Monero, a digital currency that is secure, private, and untraceable.-->

## Introduction

[What is Monero?](https://www.getmonero.org/get-started/what-is-monero/)

It's an open-source cryptocurrency project focused on private and censorship-resistant transactions. That is because Monero uses various privacy-enhancing technologies to hide information of the blockchain and ensure the anonymity of its users.

Learn more about Monero: [Moneropedia](https://www.getmonero.org/resources/moneropedia/)

See also the documentation of the application level protocol that allows **peer-to-peer** communication between Monero nodes: [Levin Protocol](https://github.com/monero-project/monero/blob/master/docs/LEVIN_PROTOCOL.md)


## Educational project using Levin Protocol

With this project I intend to analyze Monero P2P network; cryptocurrency and privacy techniques, network programming and application layer protocols. Moreover, I want to contribute to the general knowledge about Monero and its great features.

### Reverse engineering

The first step was to execute ```monerod``` and check the TCP communication with ```tcpflow``` and ```hexdump``` as executing the scripts on the folder called extract_levin_communication:

Example:
```
$ sh init.sh 90 eth0          # Listen for 90 seconds the Monero P2P communication (port 18080), specifying the interface eth0 (by default wi-fi interface)
tcpflow: listening on eth0
Terminating tcpflow process
Processing monero packets...  Output: em<i> & comm_em<i> 
Extracting IPs from data...   Output: ipak 
Getting geolocation of IPs... Output: iplocs 
```


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
> - PORT: natural int, often number 18080
> - time12: time limit to request 1001 and 1003 (threads 1,2) and then receive transactions (thread 4) from those nodes
 
### Secondary program (Python)

```locate.py``` program reads the standard input to locate nodes on the map with the following format:

```
<Method> <Longitude> <Latitude> <IP> <City/Tr>
```
> - Method:
>     * ```a```: add node on the map
>     * ```r```: remove node from the map
>     * ```t```: add transacts to an existing node
>     * ```q```: quit the execution
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
$ ./main 212.83.175.67 18080 10 | python locate.py    # 10 seconds to collect nodes from Monero P2P network + locate them on map
                                                      # and then receive transactions from available nodes  + locate them on map
Reading coordinates...
<Method> <Longitude> <Latitude> <IP> <City/Tr>:
Receiving transactions...
Transactions received: 2 (212.83.175.67 2)
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

Next we will install dependencies for the Python program:

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
$ python3 locate.py 0.5
```

Finally, combine the execution of both programs with **pipe**:
```
$ ./main 212.83.175.67 18080 10 | python locate.py
```

## Log files



## Output images

<img src="../main/imgs/mapa.png" alt="Monero nodes around the world - Example of execution 1">
<img src="../main/imgs/map.svg" alt="Monero nodes around the world - Example of execution 2">


## Related sites

[Monero](https://www.getmonero.org/)

[Monero Github project](https://github.com/monero-project/monero)

[Monero seed nodes](https://community.xmr.to/xmr-seed-nodes)

[Monerodocs](https://monerodocs.org/)

[MoneroWorld](https://moneroworld.com/)


