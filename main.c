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

void _main(void *mb_data, unsigned int mb_magic)
{
	cls();
	puts("Hello, world!");
	puts("v0.0.0.1a");
}
