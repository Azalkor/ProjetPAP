for i in `seq 0 7`; 
do
	for ((j=256 ; $j<=4096; j*=4))
    do 
		for ((k=100 ; $k<=10000; k*=10))
		do 
			echo "$i - $j - $k"
			./prog -i $k -s $j -n -v $i 
		done
	done
done

