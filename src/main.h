void init_ncurses();
void update_board(int rows, int columns, bool cells[rows][columns]);
void draw_board(int rows, int columns, bool cells[rows][columns], bool paused, int speed, int generation, int population);
int get_num_neighbors(int rows, int columns, bool cells[rows][columns], int r, int c);
int get_population(int rows, int columns, bool cells[rows][columns]);
void copy_array(int rows, int columns, bool toBeCopied[rows][columns], bool copy[rows][columns]);
void draw_pause_message();
void export_to_file(int rows, int columns, bool cells[rows][columns]);
