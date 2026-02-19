all: cavein

cavein: cavein.cpp falrowhani.cpp image.cpp timers.cpp
	g++ cavein.cpp falrowhani.cpp image.cpp timers.cpp jgaribay.cpp \
		harinaga.cpp libggfonts.a -Wall -lX11 -lGL -ocavein

clean:
	rm -f cavein

