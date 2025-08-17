compile:
#   only include necessary OpenCV libs
	g++ -std=c++20 -O3 -march=native -DNDEBUG -flto -o imgstego main.cpp ImageStego.cpp `pkg-config --cflags opencv4` -lopencv_core -lopencv_imgproc -lopencv_imgcodecs
compile-debug:
	g++ -std=c++20 -Wall -o imgstego main.cpp ImageStego.cpp `pkg-config --cflags opencv4` -lopencv_core -lopencv_imgproc -lopencv_imgcodecs
run:
	./imgstego $(ARGS)
clean:
	rm imgstego