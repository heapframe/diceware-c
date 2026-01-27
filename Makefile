CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Os -s
SRCS = main.c
OBJS = $(SRCS:.c=.o)
TARGET = diceware

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

iwyu:
	include-what-you-use -Xiwyu \
		--mapping_file=/usr/share/include-what-you-use/iwyu.gcc.imp \
		$(SRCS)

valgrind:
	valgrind --tool=massif $(TARGET)

upx:
	upx --best $(TARGET)

.PHONY: all clean iwyu valgrind