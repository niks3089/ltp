if [ $# -ne 2 ]; then
    echo "sendto_infinite_tap.sh <Number of programs> <cpu_start>"
    exit 1
fi

curr_interface=0
cpu=$2
while [ $curr_interface -lt $1 ]
do
    curr_interface=`expr $curr_interface + 1`
    echo Starting program to send packets on tap10$curr_interface on cpu $cpu
	./sendtotap -c $cpu -l -1 -m $curr_interface &
    cpu=`expr $cpu + 1`
done
