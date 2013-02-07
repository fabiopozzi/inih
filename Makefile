CFLAGS=-g -O2 -Wall -Wextra -Isrc -rdynamic -DNDEBUG $(OPTFLAGS)
LIBS=$(OPTLIBS)
PREFIX?=/usr/local

#SOURCES=$(wildcard src/*.c)
#OBJECTS=$(patsubst %.c,%.o,$(SOURCES))

TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

# The target build
#all: $(TARGET) tests
all: build 
	$(CC) -o bin/ini_demo src/ini.c

dev: CFLAGS=-g -Wall -Isrc -Wall -Wextra $(OPTFLAGS)
dev: all

$(TARGET): CFLAGS += -fPIC
$(TARGET): build $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

build:
	@mkdir -p bin

# The unit tests
.PHONY: tests
tests: CFLAGS += $(TARGET)
#tests: $(TESTS)
	#sh ./tests/runtests.sh

clean:
	rm -i bin/ini_demo
	rm -f tests/tests.log
	
