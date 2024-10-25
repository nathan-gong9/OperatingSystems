all: part1 part2 part3 part4 iobound cpubound

part1: part1.c
	gcc -o part1 part1.c -g

part2: part2.c
	gcc -o part2 part2.c -g

part3: part3.c
	gcc -o part3 part3.c -g

part4: part4.c
	gcc -o part4 part4.c -g
    
iobound: iobound.c
	gcc iobound.c -o iobound
	
cpubound: cpubound.c
	gcc cpubound.c -o cpubound

clean:
	rm -f part1 part2 part3 part4