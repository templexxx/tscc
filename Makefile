
CURDIR := $(shell pwd)

perf: build run_perf

build:
	$(CURDIR)/build.sh

run_perf:
	$(CURDIR)/tsc_perf
