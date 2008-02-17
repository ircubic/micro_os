#include "screen.h"
#define WIDTH 80
#define HEIGHT 25
static unsigned short *screen = (unsigned short*)0xB8000;
static int cursor_x = 0;
static int cursor_y = 0;
static int cursor_idx = 0;
static char color = 7;

void inline set_cursor(int x, int y)
{
	cursor_x = x;
	cursor_y = y;
	cursor_idx = (y * WIDTH) + x;
}

void cls()
{
	int i = 0;
	for(;i < (WIDTH*HEIGHT/2); i++)
		*(((long*)screen)+i) = 0;
	cursor_x = 0;
	cursor_y = 0;
	cursor_idx = 0;
}

void inline putch(char chr)
{
	screen[cursor_idx] = chr | color << 8;
	cursor_idx++;
	cursor_x++;
	if(cursor_x == WIDTH)
	{
		set_cursor(0, cursor_y+1);
	}
}

void puts(const char *string)
{
	while(*string)
	{
		putch(*string);
		string++;
	}
	set_cursor(0, cursor_y+1);
}

char get_hex(char num)
{
	if(num >= 0 && num <= 9)
		return '0' + num;
	else if(num >= 10 && num <= 16)
		return 'A' + (num-10);
	else
		return '?';
}

void print_hex_char(char c)
{
	putch(get_hex(c>>4));
	putch(get_hex(c&0xF));
}

void print_hex(unsigned int num)
{
	putch('0');
	putch('x');
	const char* ptr = (const char*)&num;
	int i = 0;
	for(; i < 4; i++)
	{
		print_hex_char(ptr[3-i]);
	}
}
