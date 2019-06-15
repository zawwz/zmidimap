IDIR=include
SRCDIR=src
ODIR=obj
BINDIR=bin

NAME = zmidimap

CC=g++
CXXFLAGS= -I$(IDIR) -Wall -pedantic -std=c++17
ifeq	 ($(RELEASE),true)
	BINDIR=.
else
	CXXFLAGS += -g
endif

LDFLAGS = -lpthread

# automatically finds .hpp
DEPS = $(shell if [ -n "$(ld $(IDIR))" ] ; then ls $(IDIR)/*.hpp ; fi)
# automatically finds .cpp and makes the corresponding .o rule
OBJ = $(shell ls $(SRCDIR)/*.cpp | sed 's/.cpp/.o/g;s|$(SRCDIR)/|$(ODIR)/|g')

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
