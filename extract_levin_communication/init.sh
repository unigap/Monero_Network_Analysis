#!/bin/sh
# To init this process, you need to connect to Monero network. This script captures (with tcpflow) the TCP communication between Monero nodes and our machine (P2P). Port number of our Monero connection must be 18080.
# $ sh init.sh <tcpflow_time> [<interfazea>]
# First argument specifies time limit to capture TCP communication received from Monero nodes.
# By default tcpflow captures traffic on wi-fi interface. If you want ethernet interface second argument would be that interface name.
# Then this script call to process_levin.sh script to comment the levin protocol headers, writing them on em<i> files.
# Later, command_conv.sh script will convert Levin message command number (hexadecimal) to decimal, writing them on comm_em<i> files.
# Due to get IP addresses ip_conv.sh script captures information of handshake (1001) response messages received, writing them on ipak file.
# Finally, to get location of each IP address, ipgeoloc.sh script has some resources, writing them on iplocs file  (4-6).
# To initialize the process you need superuser (root) privileges.
# Post-process: $ grep -E "Command number:" comm_em* | cut -d ':' -f 3 | sort | uniq  # Print different command numbers received


# Receive arguments (1 or 2)
arg=0
if [ $# -gt 1 ];
then
    int=$2 # Which interface use to capture TCP traffic (by default wi-fi interface)
    denb=$1
    arg=1
elif [ $# -eq 0 ];
then
    echo "Please you have to specify the time (on seconds) to capture Monero traffic with tcpflow. \nYou can specify interface name as second argument."
    exit 1
else
    denb=$1
fi

ter=0

# Capturing traffic on port number: 18080 (tcpflow stores raw data communication on <srce@IP:Port>-<dest@IP:Port> format files)
if [ $arg -eq 1 ]; # interface specified
then
#    sudo tcpflow -a -i $int port 18080 &
    sudo tcpflow -a -i $int port 18080 &
    sleep 2

    ps "$!" > logps # check if tcpflow process is active: passing id of last process executed in background
    ter=$?  # returns 0 if tcpflow is active; else returns 1 and we exit because interface is not valid

    if [ $ter -gt 0 ];
    then
        echo "Please choose a valid interface:"
        ip address
        exit 1
    fi
else # default
    sudo tcpflow -a port 18080 &
fi

# Wait the time specified as first argument (s)
sleep $denb

# Then kill the tcpflow process (sending SIGKILL signal)
pid=$(ps -aux | grep "tcpflow" | grep -v "grep" | awk '{print $2}')
echo "Terminating tcpflow process" # $pid"
sudo kill -9 $pid

# name of files created by tcpflow
find -mtime -1 | grep -E '(^|[^0-9])[0-9]{3}.[0-9]{3}.[0-9]{3}.[0-9]{3}.[0-9]*-[0-9]{3}.[0-9]{3}.[0-9]{3}.[0-9]{3}' > conn_files
#ls | grep -E '(^|[^0-9])[0-9]{3}.[0-9]{3}.[0-9]{3}.[0-9]{3}.[0-9]*-[0-9]{3}.[0-9]{3}.[0-9]{3}.[0-9]{3}' > conn_files

iter=1
echo "Processing monero packets... Output: em<i> & comm_em<i> "
while read -r line
do
     # em$iter and comm_em$iter files: hex, ascii and Levin protocol's headers commented (command numbers...)
     sh process_levin.sh "$line" $iter
     iter=$((iter+1))
done < conn_files

# Extract the IP lists were received with Levin 1001 messages
echo "Extracting IPs from data... Output: ipak "
sh ip_conv.sh > ipak
cat ipak | sort | uniq > ipeak # Only store them once (OK or ..: ping response)

# Get geolocation information with geoip and other online tools
echo "Getting geolocation of IPs... Output: iplocs "
sh ipgeoloc.sh ipeak > iplocs

exit 0

