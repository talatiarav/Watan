CXX      = g++
CXXFLAGS = -std=c++14 -Wall -g -MMD -Werror=vla

EXEC    = watan
SOURCES = $(wildcard *.cc)
OBJECTS = $(SOURCES:.cc=.o)
DEPENDS = $(OBJECTS:.o=.d)

$(EXEC): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(EXEC)

-include $(DEPENDS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(DEPENDS) $(EXEC)

.PHONY: clean
