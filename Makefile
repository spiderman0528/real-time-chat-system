all:
	g++ server.cpp -o server -pthread
	g++ client.cpp -o client -pthread

server:
	g++ server.cpp -o server -pthread

client:
	g++ client.cpp -o client -pthread

clean:
	rm -f server client