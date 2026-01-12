BUILDDIR = ./build
SOURCEDIR = ./src

CXX=g++
LDFLAGS = -L$(BUILDDIR) -lparkinson
CXXFLAGS = -g -Wall -Wextra -std=c++20

LIBSRC = $(SOURCEDIR)/parkinson.cpp $(SOURCEDIR)/object.cpp $(SOURCEDIR)/array.cpp $(SOURCEDIR)/output.cpp
LIBOBJ = $(patsubst $(SOURCEDIR)/%.cpp,$(BUILDDIR)/%.o,$(LIBSRC))
LIBNAME = libparkinson.a

APPSRC = $(SOURCEDIR)/demo.cpp
APPOBJ = $(BUILDDIR)/demo.o
APPNAME = demo

TESTSRC = $(SOURCEDIR)/test.cpp
TESTOBJ = $(BUILDDIR)/test.o
TESTNAME = test

all: lib 

demo: $(BUILDDIR)/$(APPNAME)

$(BUILDDIR): 
	mkdir -p $(BUILDDIR)

# -- Building library -- 

lib: $(BUILDDIR) $(LIBOBJ)
	ar rcs $(BUILDDIR)/$(LIBNAME) $(LIBOBJ)

$(BUILDDIR)/%.o: $(SOURCEDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# -- Building test app --

$(BUILDDIR)/$(APPNAME): $(APPOBJ)
	$(CXX) $(APPOBJ) $(LDFLAGS) -o $@

$(APPOBJ): $(APPSRC)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# -- Building and runnng tests --

$(TESTOBJ): $(TESTSRC) 
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/$(TESTNAME): $(TESTOBJ)
	$(CXX) $(TESTOBJ) $(LDFLAGS) -o $@
	rm -rf $(TESTOBJ)

test: $(BUILDDIR)/$(TESTNAME) $(BUILDDIR)/$(LIBNAME)
	$(BUILDDIR)/$(TESTNAME)
	rm -rf $(BUILDDIR)/$(TESTNAME)

clean:
	rm -r $(BUILDDIR)
