# Makefile for SK2-projekt

CC = clang++
CFLAGS = -std=c++20 -Wall -Wextra -pedantic -O0 -D_FORTIFY_SOURCE=2 -stdlib=libc++

SRCS = main.cpp \
       Server.cpp \
       Client.cpp \
       Channel.cpp

HDRS = server.h \
       Client.h \
       Channel.h \
       InvalidRequest.h

OBJS = $(SRCS:.cpp=.o)

SK2-projekt: $(OBJS)
	clang++ $(CFLAGS) -o SK2-projekt $(OBJS)

%.o: %.cpp $(HDRS)
	clang++ $(CFLAGS) -c $< -o $@

run: SK2-projekt
	./SK2-projekt

clean:
	rm -f $(OBJS) SK2-projekt
