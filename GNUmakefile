CXXFLAGS += -I. $(shell root-config --cflags) -g
LDFLAGS += $(shell root-config --libs) -lPhysics -lMatrix -g

CLASSES = kdtree
PROGRAMS = TrackMaker PurityStudy

all:	clean $(CLASSES) $(PROGRAMS) clean2

$(CLASSES):
	@echo '<<building' $@' object file>>'
	@$(CXX) -c $@.cpp -o $@.o $(CXXFLAGS) $(LDFLAGS)
	@rm -rf *.dSYM
$(PROGRAMS):
	@echo '<<compiling' $@'>>'
	@$(CXX) $@.cpp *.o -o $@ $(CXXFLAGS) $(LDFLAGS)
	@rm -rf *.dSYM
clean:	
	rm -f $(PROGRAMS)
clean2:
	rm -f *.o
