CONTIKI_PROJECT = sink streetlight parking aqi moisture person car
all: $(CONTIKI_PROJECT)

TARGET = cooja
CONTIKI = ../..

# Cooja-specific optimizations
CFLAGS += -DCOOJA=1

include $(CONTIKI)/Makefile.include
