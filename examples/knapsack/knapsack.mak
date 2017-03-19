

UNAME := $(shell uname)

ifeq ($(shell uname -o 2>/dev/null),Msys)
	OS := MINGW
endif

ifneq ("$(OS)","")
	EZ_PATH=../../
endif

knapsackLIB_PATH=$(EZ_PATH)/libeasea/

CXXFLAGS =  -fopenmp	-O2 -g -Wall -fmessage-length=0 -I$(knapsackLIB_PATH)include -I$(EZ_PATH)boost

OBJS = knapsack.o knapsackIndividual.o 

LIBS = -lpthread -fopenmp 
ifneq ("$(OS)","")
	LIBS += -lws2_32 -lwinmm -L"C:\MinGW\lib"
endif

#USER MAKEFILE OPTIONS :

#END OF USER MAKEFILE OPTIONS

TARGET =	knapsack

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LDFLAGS) -g $(knapsackLIB_PATH)/libeasea.a $(EZ_PATH)boost/program_options.a $(LIBS)

	
#%.o:%.cpp
#	$(CXX) -c $(CXXFLAGS) $^

all:	$(TARGET)
clean:
ifneq ("$(OS)","")
	-del $(OBJS) $(TARGET).exe
else
	rm -f $(OBJS) $(TARGET)
endif
easeaclean:
ifneq ("$(OS)","")
	-del $(TARGET).exe *.o *.cpp *.hpp knapsack.png knapsack.dat knapsack.prm knapsack.mak Makefile knapsack.vcproj knapsack.csv knapsack.r knapsack.plot knapsack.pop
else
	rm -f $(TARGET) *.o *.cpp *.hpp knapsack.png knapsack.dat knapsack.prm knapsack.mak Makefile knapsack.vcproj knapsack.csv knapsack.r knapsack.plot knapsack.pop
endif

