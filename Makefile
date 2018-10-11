PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

OBJS = hvacmgr.o Frame.o HVAC.o MQTT.o

ifeq ($(BUILD_MODE),run)
	CPPFLAGS += -O2 -std=c++1z -DBOOST_LOG_DYN_LINK 
else
	CPPFLAGS += -g -std=c++1z -Wall -DBOOST_LOG_DYN_LINK 
endif

ifeq (($realpath g++-8),)
	CXX=g++	
else
	CXX=g++-8
endif

LDFLAGS=-lboost_system -lboost_log -lpthread -lboost_thread  -lboost_log_setup -lmosquittopp

all:	hvacmgr

hvacmgr:	$(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

%.o:	$(PROJECT_ROOT)%.cpp
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

%.o:	$(PROJECT_ROOT)%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

clean:
	rm -fr hvacmgr $(OBJS)
