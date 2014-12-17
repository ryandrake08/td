LIBOUT = libpoker_tournament.a
LIBOBJS = datetime.o json.o logger.o process.o socket.o cjson/cJSON.o
TESTOBJS = test.o tests.o
SERVEROBJS = main.o program.o

CFLAGS = -g
CXXFLAGS = -g -std=c++11
LDFLAGS = -L/usr/local/lib

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
