shell379: shell379.o validateCmd.o helper.o buildInCmd.o
	gcc -Wall -std=c99 -o shell379 shell379.o helper.o buildInCmd.o validateCmd.o -lm

shell379.o: shell379.c shell379.h helper.h buildInCmd.h validateCmd.h
	gcc -Wall -std=c99 -c shell379.c

buildInCmd.o: buildInCmd.c helper.h shell379.h
	gcc -Wall -std=c99 -c buildInCmd.c

validateCmd.o: validateCmd.c shell379.h
	gcc -Wall -std=c99 -c validateCmd.c

helper.o: helper.c shell379.h 
	gcc -Wall -std=c99 -c helper.c

clean:
	rm -f shell379 *.o