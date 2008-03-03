#include "screen.h"
/* Simple screen "driver" that utilizes standard VGA text mode to output text
 * 
 * Assumes an 80x25 screen, and character wraps.
 * Has simple support for cursor position and stuff
 */


#define WIDTH 80
#define HEIGHT 25
static unsigned short *screen = (unsigned short*)0xB8000;
static int cursor_x = 0;
static int cursor_y = 0;
static int cursor_idx = 0;
static char color = 7;

void screen_set_base(void *address)
{
	screen = (unsigned short*)address;
}

/* Set the cursor to a new place */
void inline set_cursor(int x, int y)
{
	while(x >= WIDTH)
	{
		x -= WIDTH;
		y++;
	}
	while(y >= HEIGHT)
	{
		y -= HEIGHT;
	}
	cursor_x = x;
	cursor_y = y;
	cursor_idx = (y * WIDTH) + x;
}

/* Clear screen and reset cursor */
void cls()
{
	int i = 0;
	for(;i < (WIDTH*HEIGHT/2); i++)
		*(((long*)screen)+i) = 0;
	cursor_x = 0;
	cursor_y = 0;
	cursor_idx = 0;
}

/* Put a single character in the current color to the current screen position */
void inline putch(char chr)
{
	if(chr == '\n')
	{
		set_cursor(0, cursor_y+1);
		return;
	}
	screen[cursor_idx] = chr | color << 8;
	set_cursor(cursor_x+1, cursor_y);
	//cursor_idx++;
	//cursor_x++;
	//if(cursor_x == WIDTH)
	//{
		//set_cursor(0, cursor_y+1);
	//}
}

/* Put a full string and go to next line */
void puts(const char *string)
{
	while(*string)
	{
		putch(*string);
		string++;
	}
	set_cursor(0, cursor_y+1);
}

/* Helper method to get the hex equivalent of a 4bit number */
char get_hex(char num)
{
	if(num >= 0 && num <= 9)
		return '0' + num;
	else if(num >= 10 && num <= 16)
		return 'A' + (num-10);
	else
		return 'a' + (num-10);
}

/* Print a char as hex (with no prefix) */
void print_hex_char(char c)
{
	putch(get_hex((c>>4)&0xF));
	putch(get_hex(c&0xF));
}

/* Print a 32 bit number as hex (with 0x prefix) */
void print_hex(unsigned long num)
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

/* Set the color of the letters */
void set_color(char c)
{
	color = c;
}
