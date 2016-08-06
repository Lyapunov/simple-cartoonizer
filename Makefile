# Magic:
# g++ -std=c++11  extract_background.cpp -o app `pkg-config --cflags --libs opencv`

CC = g++
CFLAGS =-Wall -std=c++11
CVFLAGS=$(shell pkg-config --cflags --libs opencv)

CPP_FILES := $(wildcard src/*.cpp)
APP_FILES := $(addprefix app/,$(notdir $(CPP_FILES:.cpp=)))

all: app $(APP_FILES)

app:
	mkdir -p app

app/%: src/%.cpp
	$(CC) $(CFLAGS) $(CVFLAGS) -O3 -o $@ $<

clean:
	rm -rf app
