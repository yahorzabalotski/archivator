SRCDIR 		:= src
BUILDDIR 	:= build

SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRCS))

DEPFLAGS = -MT $@ -MMD -MP -MF $(BUILDDIR)/$*.Td
CFLAGS := -Wall
TARGET := huff 

default: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $^ -o $@

clean:
	rm -rf $(BUILDDIR)/*.o $(BUILDDIR)/*.d $(TARGET)

$(shell mkdir -p $(BUILDDIR) >/dev/null)
COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) -c
POSTCOMPILE = mv -f $(BUILDDIR)/$*.Td $(BUILDDIR)/$*.d

$(BUILDDIR)/%.o : $(SRCDIR)/%.c
$(BUILDDIR)/%.o : $(SRCDIR)/%.c $(BUILDDIR)/%.d
	$(COMPILE.c) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)


$(BUILDDIR)/%.d: ;
.PRECIOUS: $(BUILDDIR)/%.d

-include $(patsubst $(BUILDDIR)/%.o,$(BUILDDIR)/%.d, $(OBJS))

