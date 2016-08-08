CXX = g++
INCLUDES = -Iinclude
LIBS = -lmsgpackc -lcrypto -lcrypto++ -lstdc++ -lntl -lgmp -lm -lgarble -lgarblec
CXXFLAGS = -g -std=c++11 -Wall -maes -msse4 -march=native $(INCLUDES) $(LIBS)
#CXXFLAGS = -g -std=c++11 -O3 -Wall -maes -msse4 -march=native $(INCLUDES) $(LIBS)

SRCDIR = src
SOURCES = $(wildcard $(SRCDIR)/*/*.cpp)


SRCOBJS = $(SOURCES:.cpp=.o)

all: $(SRCOBJS)
	$(CXX) $(SRCDIR)/main.cpp $(SRCOBJS) $(JUSTGARBLE) $(CPPFLAGS) $(CXXFLAGS)

#testing

GTEST_DIR = googletest/googletest
TEST_DIR = test

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include

# Flags passed to the C++ compiler.
CXXFLAGS += -Wextra -pthread

TESTS = $(wildcard $(TEST_DIR)/*.cpp)
TESTOBJS = $(TESTS:.cpp=.o)

# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
		$(GTEST_DIR)/include/gtest/internal/*.h

# Builds gtest.a and gtest_main.a.

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
	$(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
	$(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^


# Builds a sample test.  A test should link with either gtest.a or
# gtest_main.a, depending on whether it defines its own main()
# function.

tests : $(SRCOBJS) $(TESTOBJS) $(JUSTGARBLE) gtest_main.a
	$(CXX) $^ $(CPPFLAGS) $(CXXFLAGS) -lpthread -o $@

.PHONY: clean
clean:
	$(RM) $(SRCDIR)/*/*.o
	$(RM) a.out
#	$(RM) gtest.a gtest_main.a *.o
	$(RM) tests
	$(RM) test/*.o
	$(RM) test/tmp/*
