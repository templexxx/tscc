#!/usr/bin/env bash

build() {
  enabled=1
  freq=$(echo $TSC_FREQ_X | bc -l)

  if [ -z "$freq" ]; then
    enabled=0
    gcc -O3 -o tsc_perf tsc_perf.c
  else
    gcc -O3 -DTSC_FREQ="$(echo $freq)" -DTSC_ENABLED="$(echo $enabled)" -o tsc_perf tsc_perf.c
  fi

  echo "freq is: ""$freq"
  echo "tsc enabled: "$enabled

  return 0
}

build
