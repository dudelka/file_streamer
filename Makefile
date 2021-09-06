srcdir = src
outdir = build
client_outdir = $(outdir)/client
server_outdir = $(outdir)/server

mode_independed_src = file.cpp socket.cpp
mode_depended_src = commandline.cpp packet_manager.cpp receiver.cpp \
	sender.cpp utils.cpp

objects := $(patsubst %.cpp, $(outdir)/%.o, $(mode_independed_src))

client_objects := $(patsubst %.cpp, $(client_outdir)/%.o, $(mode_depended_src))
client_objects += $(client_outdir)/client.o

server_objects := $(patsubst %.cpp, $(server_outdir)/%.o, $(mode_depended_src))
server_objects += $(server_outdir)/server.o

CXX = g++
CXXFLAGS = -O3 -Wall -Wextra -Werror -pthread -std=c++17 -I.

$(shell mkdir -p $(outdir))
$(shell mkdir -p $(client_outdir))
$(shell mkdir -p $(server_outdir))

all: client server

client: $(objects) $(client_objects)
	$(CXX) $(CXXFLAGS) -I $(outdir) -I $(client_outdir) -DCLIENT_MODE \
		$(objects) $(client_objects) -o $@

server: $(objects) $(server_objects)
	$(CXX) $(CXXFLAGS) -I $(outdir) -I $(server_outdir) -DSERVER_MODE \
		$(objects) $(server_objects) -o $@

$(objects): $(outdir)/%.o: $(srcdir)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(client_objects): $(client_outdir)/%.o: $(srcdir)/%.cpp
	$(CXX) $(CXXFLAGS) -DCLIENT_MODE -c $< -o $@

$(server_objects): $(server_outdir)/%.o: $(srcdir)/%.cpp
	$(CXX) $(CXXFLAGS) -DSERVER_MODE -c $< -o $@

run_tests:
	cd tests && ./run_test.py

check:
	cppcheck --enable=all -DCLIENT_MODE *.cpp *.hpp
	cppcheck --enable=all -DSERVER_MODE *.cpp *.hpp

clean:
	rm -r $(outdir) client server

.PHONY: all run_tests check clean
