if [ $# -lt 2 ]; then
    echo "sendto_infinite_tap.sh <Number of programs> <cpu_start> [optional: tap offset]"
    exit 1
fi

curr_interface=0

if [ $# -eq 3 ]; then
curr_interface=$3
fi

cpu=$2
end=`expr $1 + $curr_interface`
while [ $curr_interface -lt $end ]
do
    curr_interface=`expr $curr_interface + 1`
    echo Starting program to send packets on tap10$curr_interface on cpu $cpu
	./sendtotap -c $cpu -l -1 -m $curr_interface &
    cpu=`expr $cpu + 1`
done
