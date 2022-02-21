build:
	g++ main.cpp -o out

clean:
	rm out

_test:
	g++ test/random.cpp -o test/random
	./test/random
	rm test/random

