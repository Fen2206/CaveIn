all: play

play: falrowhani.cpp asteroids.cpp log.cpp image.cpp timers.cpp\
	falrowhani.h jgaribay.h harinaga.h image.h log.h
	g++ asteroids.cpp falrowhani.cpp log.cpp image.cpp timers.cpp \
		jgaribay.cpp harinaga.cpp \
		libggfonts.a \
		-Wall -lX11 -lGL -o play

clean:
	rm -f play
