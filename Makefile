CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall

hexapeek: hexapeek.cpp
	$(CXX) $(CXXFLAGS) hexapeek.cpp -o hexapeek

clean:
	rm -f hexapeek

.PHONY: clean