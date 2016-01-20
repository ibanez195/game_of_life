#include <ncurses.h>
#include <form.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

#define ENTER 0x0d

int main(int argc, char *argv[])
{
	init_ncurses();

	// for framerate
	int delay = 50;
	int speed = 5;

	// stats
	int generation = 0;
	int population = 0;

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
					inputfile = fopen(argv[i+1], "r");
					if(errno < 0)
					{
						mvprintw(LINES/2, (COLS/2)-12, "Error reading input file");
						nodelay(stdscr, false);
						getch();
						endwin();
						exit(1);
					}
					i += 2;
					break;
			}
		}
	}

	// set up beginning state
	if(usefile)
	{
		int length;
		int numlines = 0;
		char testline[500];

		// use testline to scan for line length
		fscanf(inputfile, "%s%n", testline, &length);

		// allocate proper memory for line
		char* line = malloc(length*sizeof(char));

		if(line == NULL)
		{
			clear();
			mvprintw(LINES/2, (COLS/2)-15, "Memory allocation error");
			refresh();
			nodelay(stdscr, true);
			getch();
			endwin();
			exit(1);
		}

		rewind(inputfile);

		// count number of lines in file
		while(fscanf(inputfile, " %s", line) > 0)
		{
			numlines++;
		}
		
		rewind(inputfile);

		for(int r = 0; r < numlines; r++)
		{
			fscanf(inputfile, " %s", line);
			for(int c = 0; c < length; c++)
			{
				if(line[c] == '1')
				{
					cells[(rows-numlines)/2+r][(columns-length)/2+c] = true;
				}
			}
		}

	}else{
		cells[middler-1][middlec] = true;
		cells[middler-1][middlec-1] = true;
		cells[middler][middlec] = true;
		cells[middler][middlec+1] = true;
		cells[middler+1][middlec] = true;
	}

	population = get_population(rows, columns, cells);


	draw_board(rows, columns, cells, paused, speed, generation, population);

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
			if(key == ENTER)
			{
				paused = true;
				draw_board(rows, columns, cells, paused, speed, generation, population);
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
				move(cur_r, cur_c);
	
				generation++;
				population = get_population(rows, columns, cells);

				draw_board(rows, columns, cells, paused, speed, generation, population);

				usleep(delay * 1000);
			}

		}else{

			// show cursor
			curs_set(1);

			// wait for input with getch()
			nodelay(stdscr, false);

			switch(getch())
			{
				// toggle state of clicked cell
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

						draw_board(rows, columns, cells, paused, speed, generation, population);
						move(cur_r, cur_c);

					}
					break;
				// toggle state of highlighted cell
				case ' ' :
					if(!cells[cur_r][cur_c])
					{
						cells[cur_r][cur_c] = true;
					}else{
						cells[cur_r][cur_c] = false;
					}

					draw_board(rows, columns, cells, paused, speed, generation, population);
					move(cur_r, cur_c+1);
				// move cursor
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
				// adjust speed
				case '-' :
					if(speed > 0)
					{
						speed -= 1;
					}
					delay = 100 - (10 * (speed-1));
					draw_board(rows, columns, cells, paused, speed, generation, population);
					break;
				case '=' :
					if(speed < 10)
					{
						speed += 1;
					}
					delay = 100 - (10 * (speed-1));
					draw_board(rows, columns, cells, paused, speed, generation, population);
					break;

				// step one frame
				case 's' :
					update_board(rows, columns, cells);
					draw_board(rows, columns, cells, paused, speed, generation, population);
					move(cur_r, cur_c);
					generation++;
					break;

				// export current state to file
				case 'e' :
					export_to_file(rows, columns, cells);
					draw_board(rows, columns, cells, paused, speed, generation, population);
					break;

				// unpause simulation
				case ENTER :
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

	for(int r=0; r < rows; r++)
	{
		for(int c=0; c < columns; c++)
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

void draw_board(int rows, int columns, bool cells[rows][columns], bool paused, int speed, int generation, int population)
{
	for(int r=0; r < rows; r++)
	{
		for(int c=0; c < columns; c++)
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
	mvprintw(1, 0, "Population: %d", population);
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

	// row above current cell
	if(cells[row_above][column_left]){count++;}
	if(cells[row_above][c]){count++;}
	if(cells[row_above][column_right]){count++;}

	// row of current cell
	if(cells[r][column_left]){count++;}
	if(cells[r][column_right]){count++;}

	// row below current cell
	if(cells[row_below][column_left]){count++;}
	if(cells[row_below][c]){count++;}
	if(cells[row_below][column_right]){count++;}

	return count;
}

int get_population(int rows, int columns, bool cells[rows][columns])
{
	int population = 0;

	for(int r = 0; r < rows; r++)
	{
		for(int c = 0; c < columns; c++)
		{
			if(cells[r][c])
			{
				population++;
			}
		}
	}

	return population;
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

void export_to_file(int rows, int columns, bool cells[rows][columns])
{
	FILE *output;
	char *filenamebuffer;
	char *filename;
	FORM *nameform;
	FIELD *fields[2];
	WINDOW *formwin;
	int ch;

	output = NULL;

	// initialize fields
	fields[0] = new_field(1, 20, 1, 0, 0, 0);
	fields[1] = NULL;

	// set field properties
	set_field_back(fields[0], A_UNDERLINE);
	field_opts_off(fields[0], O_AUTOSKIP);

	// initialize form with fields
	nameform = new_form(fields);

	// initialize form window
	formwin = newwin(7, 35, rows/2-2, columns/2-12);
	set_form_win(nameform, formwin);
	set_form_sub(nameform, derwin(formwin, 3, 20, 2, 12));

	box(formwin, 0, 0);
	mvwprintw(formwin, 1, 9, "Export to file");
	mvwprintw(formwin, 3, 2, "filename: ");

	// print form to screen
	post_form(nameform);
	wrefresh(formwin);

	// handle input to form
	while((ch = wgetch(formwin)) != ENTER)
	{
		switch(ch)
		{
			case KEY_BACKSPACE:
				form_driver(nameform, REQ_DEL_PREV);
				break;
			default:
				form_driver(nameform, ch);
				break;
		}
	}

	// extract field buffer
	form_driver(nameform, REQ_VALIDATION);
	filenamebuffer = field_buffer(fields[0], 0);

	// delete trailing whitespace on filename
	int i = 0;
	while(filenamebuffer[i] != ' ')
	{
		i++;
	}
	filename = malloc(i*sizeof(char));
	memcpy(filename, filenamebuffer, i);
	

	// open file for writing
	output = fopen(filename, "w+");

	// clean up form
	unpost_form(nameform);
	free_form(nameform);
	free_field(fields[0]);
	free_field(fields[1]);

	int topr = rows;
	int bottomr = 0;
	int leftc = columns;
	int rightc = 0;

	// find first live cell column and row
	for(int r = 0; r < rows; r++)
	{
		for(int c = 0; c < columns; c++)
		{
			if(cells[r][c])
			{
				if(r < topr){topr = r;}
				if(r > bottomr){bottomr = r;}
				if(c < leftc){leftc = c;}
				if(c > rightc){rightc = c;}
			}
		}
	}

	// use found positions to export to file
	for(int r = topr; r < bottomr; r++)
	{
		for(int c = leftc; c < rightc; c++)
		{
			if(cells[r][c])
			{
				fprintf(output, "1");
			}else{
				fprintf(output, "0");
			}
		}
		fprintf(output, "\n");
	}

	fclose(output);
}

void draw_pause_message()
{
	attron(COLOR_PAIR(2));
	mvprintw(0, (COLS/2)-3, "Paused");
	attroff(COLOR_PAIR(2));
}
