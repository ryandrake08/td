LIBOUT = libpoker_tournament.a
LIBOBJS = datetime.o gameinfo.o json.o logger.o program.o scope_timer.o server.o socket.o tournament.o types.o cjson/cJSON.o
TESTOBJS = test.o tests.o
SERVEROBJS = main.o program.o bonjour.o

CFLAGS = -g
CXXFLAGS = -g -std=c++11 -Wall
LDFLAGS = -L/usr/local/lib

ifeq ($(shell uname -s),Darwin)
	LDFLAGS += -framework CoreFoundation -framework CFNetwork
endif

tournamentd: $(SERVEROBJS) $(LIBOUT)
	$(CXX) -o $@ $^ $(LDFLAGS)

test: $(TESTOBJS) $(LIBOUT)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(LIBOUT): $(LIBOBJS)
	$(AR) rcs $(LIBOUT) $(LIBOBJS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(LIBOBJS) $(SERVEROBJS) $(TESTOBJS) tournamentd test $(LIBOUT)
