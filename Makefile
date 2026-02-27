all: cavein

cavein: cavein.cpp falrowhani.cpp jgaribay.cpp harinaga.cpp game.cpp image.cpp timers.cpp
	g++ cavein.cpp falrowhani.cpp jgaribay.cpp harinaga.cpp game.cpp image.cpp timers.cpp \
		libggfonts.a -Wall -lX11 -lGL -lm -ocavein

clean:
	rm -f cavein