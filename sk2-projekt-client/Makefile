# Makefile for SK2-projekt-client

CC = clang++
CFLAGS = -std=c++20 -Wall -Wextra -pedantic -O0 -D_FORTIFY_SOURCE=2 -stdlib=libc++

SRCS = main.cpp

OBJS = $(SRCS:.cpp=.o)

SK2-projekt-client: $(OBJS)
	$(CC) $(CFLAGS) -o SK2-projekt-client $(OBJS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

run: SK2-projekt-client
	./SK2-projekt-client

clean:
	rm -f $(OBJS) SK2-projekt-client
