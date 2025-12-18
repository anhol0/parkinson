BUILDDIR = ./build
LDFLAGS = 
CFLAGS = -g -Wall -Wextra
SOURCEDIR = ./src
SOURCES = $(SOURCEDIR)/main.cpp $(SOURCEDIR)/parser.cpp
PROJNAME = parkinson
CC=gcc
CXX=g++

all: $(SOURCES)
	mkdir -p $(BUILDDIR)
	$(CXX) -o $(BUILDDIR)/$(PROJNAME) $(CFLAGS) $(SOURCES)

run:
	$(BUILDDIR)/$(PROJNAME) test1.json

clean:
	rm -r $(BUILDDIR)
