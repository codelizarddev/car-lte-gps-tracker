# Makefile for host-side unit tests
# Usage:
#   make          — build and run all tests
#   make test     — same
#   make clean    — remove test binaries

CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -std=c11
LIBS    = -lm

TESTS   = test_gnss test_power test_relay test_mqtt_json

ifeq ($(OS),Windows_NT)
EXE_SUFFIX = .exe
TEST_BINS = $(addsuffix $(EXE_SUFFIX),$(TESTS))
RUN_ALL = powershell -NoProfile -Command "$$pass=0; $$fail=0; Write-Host ''; Write-Host '========================================'; Write-Host 'Running all unit tests...'; Write-Host '========================================'; foreach ($$t in '$(TEST_BINS)'.Split(' ')) { $$exe = '.\' + $$t; & $$exe; if ($$LASTEXITCODE -eq 0) { $$pass++ } else { $$fail++ } }; Write-Host '========================================'; Write-Host ('Test suites: ' + ($$pass + $$fail) + ' total, ' + $$pass + ' passed, ' + $$fail + ' failed'); Write-Host '========================================'; if ($$fail -ne 0) { exit 1 }"
RM_CMD = powershell -NoProfile -Command "$$paths = @('test_gnss','test_gnss.exe','test_power','test_power.exe','test_relay','test_relay.exe','test_mqtt_json','test_mqtt_json.exe'); Remove-Item -Path $$paths -Force -ErrorAction SilentlyContinue"
else
EXE_SUFFIX =
TEST_BINS = $(TESTS)
RUN_ALL = PASS=0; FAIL=0; \
	for t in $(TESTS); do \
	    ./$$t; \
	    if [ $$? -eq 0 ]; then PASS=$$((PASS+1)); else FAIL=$$((FAIL+1)); fi; \
	done; \
	echo "========================================";\
	echo "Test suites: $$((PASS+FAIL)) total, $$PASS passed, $$FAIL failed"; \
	echo "========================================"; \
	[ $$FAIL -eq 0 ]
RM_CMD = rm -f $(TESTS)
endif

.PHONY: all test clean

all: test

test: $(TEST_BINS)
	@$(RUN_ALL)

test_gnss$(EXE_SUFFIX): test_gnss.c ../components/gnss/gnss_pure.c
	$(CC) $(CFLAGS) -I../components/gnss -o $@ test_gnss.c ../components/gnss/gnss_pure.c $(LIBS)

test_power$(EXE_SUFFIX): test_power.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

test_relay$(EXE_SUFFIX): test_relay.c
	$(CC) $(CFLAGS) -o $@ $<

test_mqtt_json$(EXE_SUFFIX): test_mqtt_json.c ../components/mqtt/mqtt_payloads.c
	$(CC) $(CFLAGS) -I../components/mqtt -I../components/gnss -I../components/power -o $@ test_mqtt_json.c ../components/mqtt/mqtt_payloads.c $(LIBS)

clean:
	@$(RM_CMD)
