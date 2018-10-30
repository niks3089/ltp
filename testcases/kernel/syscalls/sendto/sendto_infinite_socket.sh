if [ $# -ne 3 ]; then
    echo "sendto_infinite_tap.sh <Number of programs> <cpu_start> <tcp/udp>"
    exit 1
fi

curr_interface=0
cpu=$2
trans=$3
while [ $curr_interface -lt $1 ]
do
    curr_interface=`expr $curr_interface + 1`
    echo Starting program to send packets via $trans as server ip: $((10 + $curr_interface)).0.0.100 on cpu $cpu
    timeout 60 ./sendto$trans -c $cpu -l -1 -m $((10 + $curr_interface)).0.0.100 &
    cpu=`expr $cpu + 1`
done
