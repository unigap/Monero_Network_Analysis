#!/bin/sh
# $ sh terminate.sh
# Easy to use script to send kill signal to the main program: SIGINT (Keyboard Interrupt: -2), else: SIGTERM (Terminate: -15)
# Main program writes logbst file (information of all nodes) and sends (prints) 'q' method to the map, due to exit that program writing map.svg (image) and mapinfo (information of the nodes on the map)
# You can also specify arguments of the execution that you want to terminate (e.g. '212.83.175.67 18080 10' to terminate './main 212.83.175.67 18080 10' process)
# Example with argument filter: $ sh terminate 212.83.175.67 18080 10

if [ $1 ];
then
    if [ $2 ];
    then
        if [ $3 ];
        then
            pidn=$(ps -aux | grep " ./main $1 $2 $3" | grep -v "grep" | awk '{print $2}')
        else
            pidn=$(ps -aux | grep " ./main $1 $2" | grep -v "grep" | awk '{print $2}')
        fi
    else
        pidn=$(ps -aux | grep " ./main $1" | grep -v "grep" | awk '{print $2}')
    fi
else
    pidn=$(ps -aux | grep " ./main " | grep -v "grep" | awk '{print $2}')
fi

#echo $pidn
if [ $pidn ];
then
    kill -2 $pidn || kill $pidn
else
    echo "The 'main' program is not running.\nCannot find ./main $1 $2 $3"
fi

