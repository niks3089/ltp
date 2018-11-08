if [ $# -ne 2 ]; then
    echo "sendto_infinite_raw.sh <Number of programs> <cpu_start>"
    exit 1
fi

curr_interface=0
cpu=$2
while [ $curr_interface -lt $1 ]
do
    curr_interface=`expr $curr_interface + 1`
    echo Starting program to send packets in netns $curr_interface on cpu $cpu
    sudo ip netns exec dummy_netns$curr_interface sudo ./sendtoraw -c $cpu -l -1 -m $curr_interface &
    cpu=`expr $cpu + 1`
done
