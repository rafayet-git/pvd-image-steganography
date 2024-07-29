compile:
	g++ -o imgstego main.cpp image.cpp -lfreeimage
clean:
	rm imgstego
