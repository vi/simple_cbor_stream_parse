CFLAGS=-Wall -g3
CXXFLAGS=-Wall -g3

all: simple_example cbor_to_jsonesque dump_scsp_events extract_strings roundtrip

simple_example: simple_example.c scsp.o

cbor_to_jsonesque: cbor_to_jsonesque.o scsp.o

dump_scsp_events: dump_scsp_events.o scsp.o

extract_strings: extract_strings.o scsp_cpp.o scsp.o
	${CXX} $^ -o $@

roundtrip: roundtrip.o scsp_cpp.o scsp.o
	${CXX} $^ -o $@
