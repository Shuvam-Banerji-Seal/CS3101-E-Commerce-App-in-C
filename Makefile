
CC = gcc
CFLAGS = -Wall -Wextra -std=c11
GTK_CFLAGS = $(shell pkg-config --cflags gtk4 libadwaita-1)
GTK_LIBS = $(shell pkg-config --libs gtk4 libadwaita-1)
SQLITE_LIBS = -lsqlite3
MATH_LIBS = -lm

LDFLAGS += -L/usr/local/lib -lsqlite3 


all: masterchatbot main


masterchatbot: chatbot/chatbot-gtk-final.c
	$(CC) $(CFLAGS) $(GTK_CFLAGS) $< -o $@ $(GTK_LIBS) $(MATH_LIBS)

# Build main
main: main.c 
	$(CC) $(CFLAGS) $^ -o $@ $(MATH_LIBS) $(LDFLAGS)

# # Clean up
# clean:
# 	rm -f masterchatbot main
