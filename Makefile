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
	rm -rf $(bin) TestCgi output
.PHONY:Cal
Cal:
	g++ -o Cal Cal.cc
.PHONY:output
output:
	mkdir output
	cp $(bin) output
	cp -rf wwwroot output
	cp Cal output/wwwroot
	cp start.sh output
