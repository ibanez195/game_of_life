#include <ncurses.h>
#include <stdlib.h>

#define KEY_ENTER 0x0d

void init_ncurses();
void update_board(int rows, int columns, bool cells[rows][columns]);
void draw_board(int rows, int columns, bool cells[rows][columns]);
int get_num_neighbors(int rows, int columns, bool cells[rows][columns], int r, int c);
void copy_array(int rows, int columns, bool toBeCopied[rows][columns], bool copy[rows][columns]);

// TODO: Add flag to change color
int main()
{
	init_ncurses();

	bool paused = true;

	// for framerate
	int delay = 50;

	//get size of terminal window
	int columns = getmaxx(stdscr);
	int rows = getmaxy(stdscr);

	bool cells[rows][columns];

	// initialize array
	int r, c;
	for(r=0; r < rows; r++)
	{
		for(c=0; c < columns; c++)
		{
			cells[r][c] = false;
		}
	}

	int middlec = columns/2;
	int middler = rows/2;

	// set up beginning state
	cells[middler-1][middlec] = true;
	cells[middler-1][middlec-1] = true;
	cells[middler][middlec] = true;
	cells[middler][middlec+1] = true;
	cells[middler+1][middlec] = true;

	draw_board(rows, columns, cells);

	// main logic loop
	while(true)
	{
		if(!paused)
		{
			// pause on press of enter
			if(getch() == KEY_ENTER)
			{
				paused = true;
			}

			update_board(rows, columns, cells);
			draw_board(rows, columns, cells);

			usleep(delay * 1000);
		}else{
			// show cursor
			curs_set(1);
			// wait for input with getch()
			nodelay(stdscr, false);

			int cur_c, cur_r;
			switch(getch())
			{
				case ' ' :
					// current cursor row and column
					cur_c = getcurx(stdscr);
					cur_r = getcury(stdscr);

					// toggle state of cell
					if(!cells[cur_r][cur_c])
					{
						cells[cur_r][cur_c] = true;
					}else{
						cells[cur_r][cur_c] = false;
					}

					draw_board(rows, columns, cells);
					move(cur_r, cur_c+1);
				case 'h' :
					cur_c = getcurx(stdscr);
					cur_r = getcury(stdscr);
					move(cur_r, cur_c-1);
					break;
				case 'j' :
					cur_c = getcurx(stdscr);
					cur_r = getcury(stdscr);
					move(cur_r+1, cur_c);
					break;
				case 'k' :
					cur_c = getcurx(stdscr);
					cur_r = getcury(stdscr);
					move(cur_r-1, cur_c);
					break;
				case 'l' :
					cur_c = getcurx(stdscr);
					cur_r = getcury(stdscr);
					move(cur_r, cur_c+1);
					break;
				case KEY_ENTER :
					paused = false;
					// hide cursor
					curs_set(0);
					// don't wait for input with getch()
					nodelay(stdscr, true);
					break;
			}
		}
	}

	return 0;
}// end main

void init_ncurses()
{
	initscr();
	cbreak();
	noecho();
	nonl();
	nodelay(stdscr, true);
	curs_set(0);

	start_color();
	use_default_colors();
	init_pair(1, COLOR_WHITE, COLOR_WHITE);
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

void draw_board(int rows, int columns, bool cells[rows][columns])
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

				mvprintw(r, c, "#");

				// end color
				attroff(COLOR_PAIR(1));
			}else{
				mvprintw(r, c, " ");
			}
		}
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
