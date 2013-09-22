gc2a : gc2a.o gc_rel.o 
	g++ gc2a.o gc_rel.o -g3 -ggdb -o0 -Wall -Wextra -Wno-unused  -o gc2a  -lcryptopp -lpthread
gc2a.o: gc2a.cpp
	g++ -c gc2a.cpp -g3 -ggdb -o0 -Wall -Wextra -Wno-unused -o gc2a.o -lcryptopp -lpthread
gc_rel.o : gc_rel.cpp
	g++ -c gc_rel.cpp -g3 -ggdb -o0 -Wall -Wextra -Wno-unused -o gc_rel.o  -lcryptopp -lpthread

clean : 
	rm *test*
	rm *key*
	rm *.o
  
