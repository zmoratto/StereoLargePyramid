VWDIR = $(HOME)/projects/VisionWorkbench/build/i386_darwin_gcc4.2

CXXFLAGS += -arch i386 -g -O0 -fno-strict-aliasing -I/opt/local/include -I$(VWDIR)/include -fno-strict-aliasing
LDFLAGS += -L/opt/local/lib -lboost_system-mt -lboost_thread-mt -L$(VWDIR)/lib -lvwCore -lvwImage -lvwMath -lvwFileIO -lvwStereo -L$(HOME)/packages/i386_darwin_gcc4.2/local/lib -lprofiler -lboost_filesystem-mt

%.o : %.cc
	$(CXX) -c -o $@ $(CXXFLAGS) $^

ross_stereo: ross_stereo.o
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $^

clean:
	rm -f *.o *~ ross_stereo *disp*.tif L?.tif R?.tif D?.tif D?-{H,V}.tif ZONE?.txt