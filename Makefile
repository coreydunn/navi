CXXFLAGS=    $(shell pkg-config --cflags ncurses)
CXXFLAGS += -Wfatal-errors

LDFLAGS=     $(shell pkg-config --libs ncurses)
LDFLAGS +=   -s

OBJS=        navi

all: $(OBJS)
navi: main.cc
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDFLAGS)
clean:
	$(RM) $(OBJS)
