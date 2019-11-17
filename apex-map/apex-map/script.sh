# baic loop

for i in {0..1}
	  do 
		  gcc gen.pub.c -lm
		  ./a.out
		  gcc Apex.c -lm
		  sudo perf stat -B -d -d -d ./a.out || rawResults.txt

	      done
