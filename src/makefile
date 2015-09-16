CXXFLAGS=-Wall -export-dynamic -c `pkg-config --cflags gtk+-3.0`
DEBUG=-D DEBUG_MODE
OBJ=main.o handlers.o time_module.o
APP_NAME=CStudy

CStudy: $(OBJ)
	g++ -o $(APP_NAME) $^ `pkg-config --libs gtk+-3.0` -export-dynamic

CStudy_DEBUG: CXXFLAGS+=$(DEBUG)
CStudy_DEBUG: CStudy

-include dependencies

.PHONY: depend_debug depend clean cleanall

depend_debug:
	g++ -MM -D DEBUG_MODE *.cpp > dependencies

depend:
	g++ -MM *.cpp > dependencies
	
clean:
	rm *.o
	
cleanall:
	rm *.o $(APP_NAME)

