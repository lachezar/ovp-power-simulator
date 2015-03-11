WORKDIR=.
EXECUTABLE=test_instructions_analyser.elf
OBJDIR=obj
CFLAGS=-DTESTING -I../src
CC=cc

all: $(WORKDIR)/$(EXECUTABLE)

$(WORKDIR)/$(EXECUTABLE): $(OBJDIR)/test_instructions_analyser.o $(OBJDIR)/instructions_analyser.o
	@ strace $(CC) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/test_%.o: test_%.c
	@ mkdir -p $(OBJDIR)
	@ strace $(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR)/%.o: ../src/%.c
	@ mkdir -p $(OBJDIR)
	@ strace $(CC) -c -o $@ $< $(CFLAGS)

clean:
	- rm -rf $(OBJDIR) $(EXECUTABLE)
