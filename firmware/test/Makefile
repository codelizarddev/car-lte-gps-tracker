# Makefile for host-side unit tests
# Usage:
#   make          — build and run all tests
#   make test     — same
#   make clean    — remove test binaries

CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -std=c11
LIBS    = -lm

TESTS   = test_gnss test_power test_relay test_mqtt_json

.PHONY: all test clean

all: test

test: $(TESTS)
	@echo ""
	@echo "========================================"
	@echo "Running all unit tests..."
	@echo "========================================"
	@PASS=0; FAIL=0; \
	for t in $(TESTS); do \
	    ./$$t; \
	    if [ $$? -eq 0 ]; then PASS=$$((PASS+1)); else FAIL=$$((FAIL+1)); fi; \
	done; \
	echo "========================================";\
	echo "Test suites: $$((PASS+FAIL)) total, $$PASS passed, $$FAIL failed"; \
	echo "========================================"; \
	[ $$FAIL -eq 0 ]

test_gnss: test_gnss.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

test_power: test_power.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

test_relay: test_relay.c
	$(CC) $(CFLAGS) -o $@ $<

test_mqtt_json: test_mqtt_json.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f $(TESTS)
