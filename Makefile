T=test1 test2

all: $(T)

%: %.cc
	g++ -std=c++14 -o $@ $^

clean:
	rm -fr $(T)
