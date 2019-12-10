# baic loop

locality=(1.0 0.0001)

for i in {0..3}
	  do 
		  var=$((2**(i+1))) 
		  for t in ${locality[@]} 
		  	do
		  sed "s/UGRAD_SPATIAL_LOCALITY/${var}/" input_template > input_sub 
		  sed "s/UGRAD_TEMPORAL_LOCALITY/${t}/" input_sub > input
		  gcc gen.pub.c -lm -w > /dev/null
		  ./a.out
		  gcc Apex.c -lm -w > /dev/null
		  sudo perf stat -B -d -d -d ./a.out > /dev/null
	  done
done
