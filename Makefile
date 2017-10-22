all: simple_example cbor_to_jsonesque dump_scsp_events

simple_example: simple_example.c scsp.o

cbor_to_jsonesque: cbor_to_jsonesque.o scsp.o

dump_scsp_events: dump_scsp_events.o scsp.o
