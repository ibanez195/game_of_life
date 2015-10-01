#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define KEY_ENTER 0x0d

void init_ncurses();
void update_board(int rows, int columns, bool cells[rows][columns]);
void draw_board(int rows, int columns, bool cells[rows][columns], bool paused, int speed, int generation);
int get_num_neighbors(int rows, int columns, bool cells[rows][columns], int r, int c);
void copy_array(int rows, int columns, bool toBeCopied[rows][columns], bool copy[rows][columns]);
void draw_pause_message();

// TODO: make a way to read text files as input patterns
int main(int argc, char *argv[])
{
	init_ncurses();

	// for framerate
	int delay = 50;
	int speed = 5;

	// generation counter
	int generation = 0;

	// get size of terminal window
	int columns = getmaxx(stdscr);
	int rows = getmaxy(stdscr);

	// create cell array
	bool cells[rows][columns];

	bool paused = true;

	// get middle of screen
	int middlec = columns/2;
	int middler = rows/2;

	// prepare for file input
	FILE *inputfile = NULL;
	bool usefile = false;

	// initialize array
	int r, c;
	for(r=0; r < rows; r++)
	{
		for(c=0; c < columns; c++)
		{
			cells[r][c] = false;
		}
	}



	//default color for cells
	init_pair(1, COLOR_WHITE, COLOR_WHITE);

	//for top bar
	init_pair(2, COLOR_BLACK, COLOR_GREEN);

	// handle flags
	if(argc > 1 && argv[1][0] == '-')
	{
		int i=1;
		while(i < argc)
		{
			switch(argv[i][1])
			{
				// use user defined color
				case 'c' :
					if(strcmp(argv[i+1], "black") == 0){init_pair(1, COLOR_BLACK, COLOR_BLACK);}
					else if(strcmp(argv[i+1], "red") == 0){init_pair(1, COLOR_RED, COLOR_RED);}
					else if(strcmp(argv[i+1], "green") == 0){init_pair(1, COLOR_GREEN, COLOR_GREEN);}
					else if(strcmp(argv[i+1], "yellow") == 0){init_pair(1, COLOR_YELLOW, COLOR_YELLOW);}
					else if(strcmp(argv[i+1], "blue") == 0){init_pair(1, COLOR_BLUE, COLOR_BLUE);}
					else if(strcmp(argv[i+1], "magenta") == 0){init_pair(1, COLOR_MAGENTA, COLOR_MAGENTA);}
					else if(strcmp(argv[i+1], "cyan") == 0){init_pair(1, COLOR_CYAN, COLOR_CYAN);}
					else if(strcmp(argv[i+1], "white") == 0){init_pair(1, COLOR_WHITE, COLOR_WHITE);}
					i+=2;
					break;
				// use user defined speed
				case 's' :
					speed = atoi(argv[i+1]);

					if(speed < 1)
					{
						speed = 1;
					}else if(speed > 10){
						speed = 10;
					}

					delay = 100 - (10 * (speed-1));
					i+=2;
					break;
				// use file as input for beginning state
				case 'f' :
					usefile = true;
					inputfile = fopen(argv[i+1], 'r');
					if(errno < 0)
					{
						mvprintw(LINES/2, (COLS/2)-12, "Error reading input file");
						nodelay(stdscr, false);
						getch();
						endwin();
						exit(1);
					}
					break;
			}
		}
	}

	// set up beginning state
	if(usefile)
	{
		int numlines = 0;
		char *line = malloc(20*sizeof(char));
		size_t *length;

		while(getline(&line, &length, inputfile) < 0)
		{
			numlines++;
		}
		mvprintw(0, 0, "number of line: %d", numlines);
		
	}else{
		cells[middler-1][middlec] = true;
		cells[middler-1][middlec-1] = true;
		cells[middler][middlec] = true;
		cells[middler][middlec+1] = true;
		cells[middler+1][middlec] = true;
	}



	draw_board(rows, columns, cells, paused, speed, generation);

	MEVENT event;

	// main logic loop
	while(true)
	{
		int cur_c = getcurx(stdscr);
		int cur_r = getcury(stdscr);
		if(!paused)
		{
			// pause on press of enter
			int key = getch();
			if(key == KEY_ENTER)
			{
				paused = true;
				draw_board(rows, columns, cells, paused, speed, generation);
				move(cur_r, cur_c);
			}else if(key == '-'){
				if(speed > 0)
				{
					speed -= 1;
				}
				delay = 100 - (10 * (speed-1));
			}else if(key == '='){
				if(speed < 10)
				{
					speed += 1;
				}
				delay = 100 - (10 * (speed-1));
			}else{
				update_board(rows, columns, cells);
				draw_board(rows, columns, cells, paused, speed, generation);
				move(cur_r, cur_c);
	
				generation++;

				usleep(delay * 1000);
			}

		}else{

			// show cursor
			curs_set(1);

			// wait for input with getch()
			nodelay(stdscr, false);

			switch(getch())
			{
				case KEY_MOUSE:
					if(getmouse(&event) == OK)
					{
						move(event.y, event.x);

						if(!cells[event.y][event.x])
						{
							cells[event.y][event.x] = true;
						}else{
							cells[event.y][event.x] = false;
						}

						draw_board(rows, columns, cells, paused, speed, generation);
						move(cur_r, cur_c);

					}
					break;
				case ' ' :
					// toggle state of cell
					if(!cells[cur_r][cur_c])
					{
						cells[cur_r][cur_c] = true;
					}else{
						cells[cur_r][cur_c] = false;
					}

					draw_board(rows, columns, cells, paused, speed, generation);
					move(cur_r, cur_c+1);
				case 'h' :
					cur_c = getcurx(stdscr);
					cur_r = getcury(stdscr);

					if(cur_c == 0)
					{
						move(cur_r, COLS-1);
					}else{
						move(cur_r, cur_c-1);
					}

					break;
				case 'j' :
					cur_c = getcurx(stdscr);
					cur_r = getcury(stdscr);

					if(cur_r == LINES-1)
					{
						move(0, cur_c);
					}else{
						move(cur_r+1, cur_c);
					}

					break;
				case 'k' :
					if(cur_r == 0)
					{
						move(LINES-1, cur_c);
					}else{
						move(cur_r-1, cur_c);
					}

					break;
				case 'l' :
					if(cur_c == COLS-1)
					{
						move(cur_r, 0);
					}else{
						move(cur_r, cur_c+1);
					}

					break;
				case '-' :
					if(speed > 0)
					{
						speed -= 1;
					}
					delay = 100 - (10 * (speed-1));
					draw_board(rows, columns, cells, paused, speed, generation);
					break;
				case '=' :
					if(speed < 10)
					{
						speed += 1;
					}
					delay = 100 - (10 * (speed-1));
					draw_board(rows, columns, cells, paused, speed, generation);
					break;
				case KEY_ENTER :
					paused = false;
					// hide cursor
					curs_set(0);
					// don't wait for input with getch()
					nodelay(stdscr, true);
					break;
			}// end switch
		}// end else
	}//end while(true)

	return 0;
}// end main

void init_ncurses()
{
	initscr();
	cbreak();
	noecho();
	nonl();
	nodelay(stdscr, true);
	keypad(stdscr, true);
	curs_set(0);

	mousemask(BUTTON1_CLICKED | REPORT_MOUSE_POSITION, NULL);

	start_color();
	use_default_colors();
}

void update_board(int rows, int columns, bool cells[rows][columns])
{
	// create copy of initial state
	bool initial_copy[rows][columns];
	copy_array(rows, columns, cells, initial_copy);

	int r, c;
	for(r=0; r < rows; r++)
	{
		for(c=0; c < columns; c++)
		{
			int neighbors = get_num_neighbors(rows, columns, initial_copy, r, c);

			if(initial_copy[r][c])
			{
				if(neighbors < 2 || neighbors > 3)
				{
					cells[r][c] = false;
				}
			}else{
				if(neighbors == 3)
				{
					cells[r][c] = true;
				}
			}

		}
	}
}

void draw_board(int rows, int columns, bool cells[rows][columns], bool paused, int speed, int generation)
{
	int r, c;
	for(r=0; r < rows; r++)
	{
		for(c=0; c < columns; c++)
		{
			if(cells[r][c])
			{
				// start white on white color
				attron(COLOR_PAIR(1));

				mvprintw(r, c, " ");

				// end color
				attroff(COLOR_PAIR(1));
			}else{
				mvprintw(r, c, " ");
			}
		}
	}

	mvprintw(0, 0, "Generation: %d", generation);
	mvprintw(0, COLS-10, "Speed: %d", speed);

	if(paused)
	{
		draw_pause_message();
	}

	refresh();
}

int get_num_neighbors(int rows, int columns, bool cells[rows][columns], int r, int c)
{
	int row_above, row_below;
	int column_left, column_right;

	// define neighboring rows and columns accounting for wrapping
	row_above    =  r == 0         ? rows-1    : r-1;
	row_below    =  r == rows-1    ? 0         : r+1;
	column_left  =  c == 0         ? columns-1 : c-1;
	column_right =  c == columns-1 ? 0         : c+1;

	int count = 0;

	if(cells[row_above][column_left]){count++;}
	if(cells[row_above][c]){count++;}
	if(cells[row_above][column_right]){count++;}

	if(cells[r][column_left]){count++;}
	if(cells[r][column_right]){count++;}

	if(cells[row_below][column_left]){count++;}
	if(cells[row_below][c]){count++;}
	if(cells[row_below][column_right]){count++;}

	return count;
}

void copy_array(int rows, int columns, bool toBeCopied[rows][columns], bool copy[rows][columns])
{
	int r, c;
	for(r=0; r < rows; r++)
	{
		for(c=0; c < columns; c++)
		{
			copy[r][c] = toBeCopied[r][c];
		}
	}
}

void draw_pause_message()
{
	attron(COLOR_PAIR(2));
	mvprintw(0, (COLS/2)-3, "Paused");
	attroff(COLOR_PAIR(2));
}
