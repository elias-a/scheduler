CXX=g++
CXXFLAGS=-std=c++11

OBJECTS=main
OBJECTS_LINK=scheduler.cpp

all: $(OBJECTS)

$(OBJECTS): %: %.cpp
	$(CXX) $(CXXFLAGS) -o $@.o $< $(OBJECTS_LINK)

clean:
	rm $(OBJECTS).o