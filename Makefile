exec: *.cpp
	g++ -o exec *.cpp -std=c++17 -Wall -Werror -pthread

clean: 
	rm exec
