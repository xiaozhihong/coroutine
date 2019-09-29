# =========================================================
INCLUDE_DIR    += -I.. -I./include/ -I./3rd/include

LIB_DIR        +=
# ==========================================================
CC             = gcc
CXX 		   = g++
USER_MACRO     = -DDEBUG_
GNU_FLAGS      = -g -W -O1 -fstrict-aliasing  -fno-omit-frame-pointer $(USER_MACRO)
CFLAGS         = $(GNU_FLAGS)
CXXFLAGS       = $(GNU_FLAGS) -std=c++0x

# ==========================================================
SOURCES += $(wildcard ./*.cpp)
SOURCES += $(wildcard ../*.cpp)
SOURCES += $(wildcard ../*.S)
OBJECTS += $(patsubst %.cpp,%.o, $(patsubst %.c,%.o, $(patsubst %.S,%.o, $(SOURCES))))
# ==========================================================
ALL_OBJECTS = $(OBJECTS)
# ==========================================================
DEP_FILE += $(foreach obj, $(ALL_OBJECTS), $(dir $(obj)).$(basename $(notdir $(obj))).d)
# ==========================================================
TARGET = main
# ==========================================================

all: $(TARGET)

-include $(DEP_FILE)

.%.d: %.cpp
	@echo "update $@ ..."; \
    echo -n $< | sed s/\.cpp/\.o:/ > $@; \
    $(CXX) $(INCLUDE_DIR) $(CXXFLAGS)  -MM $< | sed '1s/.*.://' >> $@;

%.o: %.cpp
	$(CXX) $(INCLUDE_DIR) $(CXXFLAGS) -o $@ -c $<

.%.d: %.c
	@echo "update $@ ..."; \
    echo -n $< | sed s/\.c/\.o:/ > $@; \
    $(CC) $(INCLUDE_DIR) $(CFLAGS)  -MM $< | sed '1s/.*.://' >> $@;

%.o: %.c
	$(CC) $(INCLUDE_DIR) $(CFLAGS) -o $@ -c $<

%.o: %.S
	$(CC) $(INCLUDE_DIR) $(CFLAGS) -o $@ -c $<

$(TARGET): $(OBJECTS)
	$(CXX) $(INCLUDE_DIR) $(CXXFLAGS) $(OBJECTS) $(LIB_DIR) -o $@

clean:
	rm -f $(DEP_FILE) $(OBJECTS) $(TARGET) *.o
