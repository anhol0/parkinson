BUILDDIR = ./build
SOURCEDIR = ./src

CXX=g++
LDFLAGS = -L$(BUILDDIR) -lparkinson
CXXGLAGS = -g -Wall -Wextra -std=c++20

LIBSRC = $(SOURCEDIR)/parkinson.cpp
LIBOBJ = $(BUILDDIR)/parkinson.o
LIBNAME = libparkinson.a

APPSRC = $(SOURCEDIR)/main.cpp
APPOBJ = $(BUILDDIR)/app.o
APPNAME = app

all: lib 

app: $(BUILDDIR)/$(APPNAME)

$(BUILDDIR): 
	mkdir -p $(BUILDDIR)

# -- Building library -- 

lib: $(BUILDDIR) $(LIBOBJ)
	ar rcs $(BUILDDIR)/$(LIBNAME) $(LIBOBJ)
	rm $(LIBOBJ)

$(LIBOBJ): $(LIBSRC)
	$(CXX) $(CXXGLAGS) -c $< -o $@

# -- Building test app --

$(BUILDDIR)/$(APPNAME): $(APPOBJ)
	$(CXX) $(APPOBJ) $(LDFLAGS) -o $@
	rm $(APPOBJ)

$(APPOBJ): $(APPSRC)
	$(CXX) $(CXXGLAGS) -c $< -o $@

clean:
	rm -r $(BUILDDIR)
