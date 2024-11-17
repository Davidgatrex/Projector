SOURCES = $(shell find src/*.cpp)

code: $(SOURCES)
	g++ -o build/projector $(SOURCES)