IDIR=include
SRCDIR=src
ODIR=obj
BINDIR=.

NAME = zmidimap

LDFLAGS = -lpthread

CC=g++
CXXFLAGS= -I$(IDIR) -Wall -pedantic -std=c++17
ifeq ($(DEBUG),true)
	CXXFLAGS += -g
endif

$(shell mkdir -p $(ODIR))
$(shell mkdir -p $(BINDIR))

# automatically finds .h and .hpp
DEPS = $(shell if [ -n "$(ld $(IDIR))" ] ; then ls $(IDIR)/*.hpp $(IDIR)/*.h 2>/dev/null ; fi)
# automatically finds .c and .cpp and makes the corresponding .o rule
OBJ = $(shell ls $(SRCDIR)/*.cpp $(SRCDIR)/*.c 2>/dev/null | sed 's|\.cpp|.o|g;s|\.c|.o|g;s|$(SRCDIR)/|$(ODIR)/|g')

$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(ODIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(BINDIR)/$(NAME): $(OBJ) $(DEPS)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

test: $(BINDIR)/$(NAME)
	$(BINDIR)/$(NAME)

clean:
	rm $(ODIR)/*.o

clear:
	rm $(BINDIR)/$(NAME)

install:
	mv $(BINDIR)/$(NAME) /usr/local/bin
