bin=HttpdServer
cc=g++
LDFLAGS=-lpthread 
.PHONY:all
all:$(bin) 
$(bin):HttpdServer.cc
	$(cc) -o $@ $^ $(LDFLAGS) -std=c++11 
.PHONY:clean
clean:
	rm -rf $(bin) TestCgi output
	rm -f *.o *.hpp.gch
.PHONY:output
output:
	mkdir output
	cp $(bin) output
	cp -rf wwwroot output
	cp Cal output/wwwroot
	cp start.sh output
