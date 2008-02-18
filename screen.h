#ifndef KERN_SCREEN_H
#define KERN_SCREEN_H 1
void inline set_cursor(int x, int y);
void cls();
void inline putch(char chr);
void puts(const char *string);
void print_hex_char(char c);
void print_hex(unsigned int num);
void set_color(char c);
#endif
