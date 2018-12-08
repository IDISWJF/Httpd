bin=HttpdServer
cc=g++
LDFLAGS=-lpthread 
.PHONY:all
all:$(bin) cgi Cal
$(bin):HttpdServer.cc
	$(cc) -o $@ $^ $(LDFLAGS) -std=c++11 
.PHONY:cgi
cgi:
	g++ -o TestCgi TestCgi.cc
.PHONY:clean
clean:
	rm -f $(bin) TestCgi
.PHONY:Cal
Cal:
	g++ -o Cal Cal.cc
