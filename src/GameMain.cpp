//COS10009: Introduction to Programming
//Task 10HD: HD Custom code
//Project name: Feeding Frenzy: An aquatic game
//---------------------------------------------
//This file contains the C source code to remake
// the PopCqp Games called Feeding Frenzy.
//The code uses Swingame Library to a large extent.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "SwinGame.h"

#define REP(i,a,b) for ( int (i) = (a); (i) < (b); ++(i) )

#define N_LEVELS 2

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 720

#define TRACE_DISTANCE_SMALL 10000
#define TRACE_DISTANCE_MEDIUM 30000
#define TRACE_DISTANCE_BIG 60000

#define MENU_BUTTON_COLUMN (SCREEN_WIDTH - BUTTON_WIDTH) / 2

#define BUTTON_WIDTH 150
#define BUTTON_HEIGHT 50

#define LEVEL_BUTTON_HEIGHT 20
#define LEVEL_BUTTON_WIDTH 20

#define BACK_X_POSITION 0
#define BACK_Y_POSITION 0
#define BACK_WIDTH 50
#define BACK_HEIGHT 50

#define N_SMALL_FISH 10
#define N_MEDIUM_FISH 5
#define N_BIG_FISH 2
#define SUM_FISH (N_SMALL_FISH + N_MEDIUM_FISH + N_BIG_FISH)

#define LEVEL_WINNING 100

#define STAR_TIME 10000
#define LIFE_BONUS_TIME 60000

#define STAR_SCORE_BONUS 10
#define STAR_PROGRESS_BONUS 2
#define STAR_SPEED 10

enum SIZE 	{SMALL = 0,
			MEDIUM,
			BIG
			};

enum SPEC 	{SUCK,
			POISONATE
			};

enum DIRECTION {F_LEFT,
				F_RIGHT
				};

enum STATUS {
			MENU,
			LEVEL_MAP,
			INGAME,
			LEVEL_WON,
			LEVEL_FAILED,
			HIGH_SCORE,
			EXIT
			};


typedef struct {
	char name[50]; 				//fish name
	SIZE size;					//fish size
	float x;					//fish x position
	float y;					//fish position
	DIRECTION d;				//direction of fish
	int speed;					//constant speed of fish
	float speed_x, speed_y;		//speed of fish according to x and y cordinates
								//a number indicates the number of remaining lives
	bool isdead;
	timer time;
} FISH;

typedef struct {
	float x;					//star x position
	float y;					//star y position
	timer time;					//a timer to check whether the star should appear on the screen
} STAR;							//if player fish eats star, player earns some bonus score points and progress points

typedef struct {
	int score;
	int progress;
	int highest_level;
	int life;
	bool isdead;
} PLAYER_DATA;

typedef struct {
	FISH player_fish;

	FISH small_fish;
	FISH medium_fish;
	FISH big_fish;

	FISH huge_fish;

	char bgr[50];
	int milestone[2];
} LEVEL_DATA;

typedef struct {
	PLAYER_DATA p1, p2;
	int high_score[10];
	LEVEL_DATA level_data[N_LEVELS];
	int n;
} GAME_DATA;

//
// check if button clicked
//
bool button_clicked(float button_X, float button_Y, float button_width, float button_height)
{
  float x,y;
  x = mouse_x();
  y = mouse_y();
  if (mouse_clicked(LEFT_BUTTON))
    if (x > button_X && x < button_X + button_width && y > button_Y && y < button_Y + button_height)
      return true;
  return false;
}
//
// function to calculate the bonus score point when player fish eats another fish
//
int score_bonus(SIZE size)
{
	if (size == SMALL) return 2;
	if (size == MEDIUM) return 5;
	if (size == BIG) return 10;
}
//
// function to calculate the bonus progress point when player fish eats another fish
//
int progress_bonus(SIZE size)
{
	if (size == SMALL) return 1;
	if (size == MEDIUM) return 2;
	if (size == BIG) return 3;
}
//
//Load necessary resources containing in the resource folder
//
void load_resources()
{
	load_sound_effect_named("bite1", "bite1.wav");
	//main images
	load_bitmap_named("bgr_menu", "bgr_menu.jpg");
	load_bitmap_named("bgr_level", "bgr_level.jpg");

	load_bitmap_named("bgr_lv1", "bgr_lv1.jpg");
	load_bitmap_named("bgr_lv2", "bgr_lv2.jpg");
	load_bitmap_named("bgr_lv3", "bgr_lv3.png");

	load_bitmap_named("star", "star.png");
	//images for level 1
	load_bitmap_named("angel_left_small", "angel_left_small.png");
	load_bitmap_named("angel_right_small", "angel_right_small.png");
	load_bitmap_named("angel_left_medium", "angel_left_medium.png");
	load_bitmap_named("angel_right_medium", "angel_right_medium.png");
	load_bitmap_named("angel_left_big", "angel_left_big.png");
	load_bitmap_named("angel_right_big", "angel_right_big.png");

	load_bitmap_named("tuna_left", "tuna_left.png");
	load_bitmap_named("tuna_right", "tuna_right.png");

	load_bitmap_named("shark_left", "shark_left.png");
	load_bitmap_named("shark_right", "shark_right.png");
	load_bitmap_named("dory_left", "dory_left.png");
	load_bitmap_named("dory_right", "dory_right.png");
	//image for level 2
	load_bitmap_named("koi_left_small", "koi_left_small.png");
	load_bitmap_named("koi_right_small", "koi_right_small.png");
	load_bitmap_named("koi_left_medium", "koi_left_medium.png");
	load_bitmap_named("koi_right_medium", "koi_right_medium.png");
	load_bitmap_named("koi_left_big", "koi_left_big.png");
	load_bitmap_named("koi_right_big", "koi_right_big.png");

	load_bitmap_named("bluefish_left", "bluefish_left.png");
	load_bitmap_named("bluefish_right", "bluefish_right.png");
	load_bitmap_named("brownfish_left", "brownfish_left.png");
	load_bitmap_named("brownfish_right", "brownfish_right.png");
	load_bitmap_named("marlin_left", "marlin_left.png");
	load_bitmap_named("marlin_right", "marlin_right.png");

}

GAME_DATA load_game(void)
{
	GAME_DATA result;
	load_resources();
	LEVEL_DATA level_data;

	FILE *level_file_ptr = NULL;
	FILE *player_file_ptr = NULL;
	FILE *high_score_file_ptr = NULL;
	// load level
	char level_index_string[3];
	int i;
	for (i = 0; i < N_LEVELS; ++i)
	{
		char level_name[] = "level";
		itoa(i+1, level_index_string, 10);

		strcat(level_name, level_index_string);
		strcat(level_name, ".dat");
		printf("%s\n", level_name);
		if ((level_file_ptr = fopen(level_name, "r")) != NULL) {
			printf("Find the file %s\n", level_name);
			fread(&result.level_data[i], sizeof(LEVEL_DATA), 1, level_file_ptr);
		}
		else printf("can\'t find file %s\n", level_name);

		fclose(level_file_ptr);
	}
	// load player data
	if ((player_file_ptr = fopen("player.dat", "r")) != NULL)
		fread(&(result.p1), sizeof(PLAYER_DATA), 1, player_file_ptr);
	else printf("can\'t find player data\n");

	if (&(result.p1) == NULL)
		printf("Error loading player data\n");
	fclose(player_file_ptr);
	// load high score data
	if ((high_score_file_ptr = fopen("highscore.dat", "r")) != NULL) {
		fscanf(high_score_file_ptr, "%d", &result.n);
		REP(i, 0, result.n) fscanf(high_score_file_ptr, "%d", &result.high_score[i]);
	} else printf("can\'t find high score data\n");

	fclose(high_score_file_ptr);

	return result;
}


FISH init_player_data(LEVEL_DATA level_data, SIZE size)
{
	FISH result;

	result.x = mouse_x();
	result.y = mouse_y();
	strcpy(result.name, level_data.player_fish.name);
	result.size = size;
	result.d = F_LEFT;
	result.isdead = false;
	return result;
}

void init_single_fish(FISH *fish, LEVEL_DATA level_data, SIZE size)
{
	fish->x = (2*rnd() - 1 >= 0) ? (1.4 * SCREEN_WIDTH) : -0.4 * SCREEN_WIDTH;
	fish->y = rnd()*SCREEN_HEIGHT;

	if (size == SMALL) {
		fish->size = SMALL;
		strcpy(fish->name, level_data.small_fish.name);
		fish->speed  = level_data.small_fish.speed;
	}
	else if (size == MEDIUM) {
		fish->size = MEDIUM;
		strcpy(fish->name, level_data.medium_fish.name);
		fish->speed = level_data.medium_fish.speed;
	}
	else if (size == BIG) {
		fish->size = BIG;
		strcpy(fish->name, level_data.big_fish.name);
		fish->speed = level_data.big_fish.speed;
	}

	if (rnd(2)%2) fish->d = F_LEFT;
	else fish->d = F_RIGHT;

	fish->isdead = false;

	fish->time = create_timer();
	start_timer(fish->time);
}

void init_star(STAR * star)
{
	star->x = rnd()*SCREEN_WIDTH;
	star->y = SCREEN_HEIGHT*3/2;
	star->time = create_timer();
	start_timer(star->time);
}

//
// The function will update the game status.
//
STATUS game_status_updated(STATUS status, int life, int progress)
{
	switch(status) {
		case MENU:
			if (button_clicked(MENU_BUTTON_COLUMN, 		SCREEN_HEIGHT / 4, BUTTON_WIDTH, BUTTON_HEIGHT)) return LEVEL_MAP;
			if (button_clicked(MENU_BUTTON_COLUMN, 2*	SCREEN_HEIGHT / 4, BUTTON_WIDTH, BUTTON_HEIGHT)) return HIGH_SCORE;
			if (button_clicked(MENU_BUTTON_COLUMN, 3*	SCREEN_HEIGHT / 4, BUTTON_WIDTH, BUTTON_HEIGHT)) return EXIT;
			break;

		case LEVEL_MAP:
			if (button_clicked(BACK_X_POSITION, BACK_Y_POSITION, BACK_WIDTH, BACK_HEIGHT)) return MENU;
			break;

		case INGAME:
			if (button_clicked(BACK_X_POSITION, BACK_Y_POSITION, BACK_WIDTH, BACK_HEIGHT)) return MENU;
			if (life < 0) return LEVEL_FAILED;
			if (progress >= LEVEL_WINNING) return LEVEL_WON;
			break;

		case LEVEL_FAILED:
			if (button_clicked(BACK_X_POSITION, BACK_Y_POSITION, BACK_WIDTH, BACK_HEIGHT)) return MENU;
			break;

		case LEVEL_WON:
			if (button_clicked(BACK_X_POSITION, BACK_Y_POSITION, BACK_WIDTH, BACK_HEIGHT)) return LEVEL_MAP;
			break;

		default:
			if (button_clicked(BACK_X_POSITION, BACK_Y_POSITION, BACK_WIDTH, BACK_HEIGHT)) return MENU;
			break;
	}
	return status;
}

void check_fish_bitten(FISH * player_fish, FISH  fish[], PLAYER_DATA * player_data)
{
	char player_fish_name[50];
	strcpy(player_fish_name, player_fish->name);
	strcat(player_fish_name, (player_fish->d == F_LEFT) ? "_left" : "_right");

	switch(player_fish->size) {
	case SMALL:
		strcat(player_fish_name, "_small");
		break;
	case MEDIUM:
		strcat(player_fish_name, "_medium");
		break;
	case BIG:
		strcat(player_fish_name, "_big");
		break;
	}
	printf("player_fish_name: %s\n\n", player_fish_name);
	REP(i, 0, SUM_FISH) {
		int j = 0;
		char name_i[50];
		strcpy(name_i, fish[i].name);
		strcat(name_i, (fish[i].d == F_LEFT) ? "_left" : "_right");

		while (fish[j].size < fish[i].size) {
			char name_j[50];
			strcpy(name_j, fish[j].name);
			strcat(name_j, (fish[j].d == F_LEFT) ? "_left" : "_right");
			if (bitmap_collision(bitmap_named(name_i),fish[i].x, fish[i].y, bitmap_named(name_j), fish[j].x, fish[j].y))
				fish[j].isdead = true;
			j++;
		}

		if (bitmap_collision(bitmap_named(name_i),fish[i].x, fish[i].y, bitmap_named(player_fish_name), player_fish->x, player_fish->y)) {
			if (player_fish->size >= fish[i].size) {
				player_data->score	+= score_bonus(fish[i].size);
				player_data->progress += progress_bonus(fish[i].size);
				fish[i].isdead = true;
			}
			else {
				player_fish->isdead = true;
			}
		}
	}
}
//
//update in-game fish position
//
void update_fish_position(FISH * fish, FISH player_fish)
{
	//Fish change their direction after a certain amount of time
	if (timer_ticks(fish->time) > 3000) {
		fish->speed_x = 2*rnd() - 1;
		fish->speed_y = 0.5*(2*rnd() - 1);
		reset_timer(fish->time);
		start_timer(fish->time);
	}

	double dist_square = (player_fish.x - fish->x)*(player_fish.x - fish->x) + (player_fish.y - fish->y)*(player_fish.y - fish->y);
	double trace_distance = (fish->size == SMALL) ? TRACE_DISTANCE_SMALL : ((fish->size == MEDIUM) ? TRACE_DISTANCE_MEDIUM : TRACE_DISTANCE_BIG);

	//Fish trace player fish if player fish is smaller than the fish and vice versa, fish run away from player fish if player fish is bigger than fish.

	if (dist_square < trace_distance) {
		if (fish->size > player_fish.size) {
			fish->speed_x = player_fish.x - fish->x >= 0 ? 1 : -1;
			fish->speed_y = player_fish.y - fish->y >= 0 ? 1 : -1;
		}
		else {
			fish->speed_x = player_fish.x - fish->x >= 0 ? -1 : 1;
			fish->speed_y = player_fish.y - fish->y >= 0 ? -1 : 1;
		}
		reset_timer(fish->time);
		start_timer(fish->time);
	}

	fish->d = (fish->speed_x < 0) ? F_LEFT: F_RIGHT;
	fish->x += fish->speed_x * fish->speed; //update x position
	fish->y += fish->speed_y * fish->speed; //update y position

	//If fish go outside the screeen region, they will appear on the opposite side of the screen

	if (fish->x > SCREEN_WIDTH * 3/2) fish->x = -1/2 * SCREEN_WIDTH;
	if (fish->x < SCREEN_WIDTH * -1/2) fish->x = 3/2 * SCREEN_WIDTH;
	if (fish->y > SCREEN_HEIGHT * 3/2) fish->y = -1/2 * SCREEN_HEIGHT;
	if (fish->y < SCREEN_HEIGHT * -1/2) fish->y = 3/2 * SCREEN_HEIGHT;
}
//
// procedure to update fish life if they are dead
//
void update_fish_life(FISH * fish, LEVEL_DATA level_data)
{
	if (fish->isdead == true)
		init_single_fish(fish, level_data, fish->size);
}
//
// procedure to update the position of star
//
void update_star(STAR * star)
{
	if (timer_ticks(star->time) > STAR_TIME)
	{
		star->y -= STAR_SPEED;
	}
	if (star->y == 0)
		init_star(star);
}

void handle_player_input(FISH *player_fish)
{
	player_fish->x = mouse_x();
	player_fish->y = mouse_y();
	_vector v = mouse_movement();
	player_fish->d = (v.x < 0) ? F_LEFT : F_RIGHT;
}
//
//procedure to update all information about fish and player fish when playing game
//
void update_ingame(FISH *player_fish, FISH fish[], STAR * star, PLAYER_DATA * player_data, LEVEL_DATA level_data)
{
	if (player_fish->isdead) {
		show_mouse();
		delay(5000);
		hide_mouse();
		player_data->life--;
		if (player_data->life >= 0) {
			*player_fish = init_player_data(level_data, player_fish->size);
			switch(player_fish->size) {
				case SMALL:
					player_data->progress = 0;
					break;
				case MEDIUM:
					player_data->progress = level_data.milestone[0];
					break;
				case BIG:
					player_data->progress = level_data.milestone[1];
					break;
			}
		}
	}
	char player_fish_name[50];
	strcpy(player_fish_name, player_fish->name);
	strcat(player_fish_name, (player_fish->d == F_LEFT) ? "_left" : "_right");

	switch(player_fish->size) {
	case SMALL:
		strcat(player_fish_name, "_small");
		break;
	case MEDIUM:
		strcat(player_fish_name, "_medium");
		break;
	case BIG:
		strcat(player_fish_name, "_big");
		break;
	}

	handle_player_input(player_fish);
	REP(i, 0, SUM_FISH) {
		update_fish_position(&fish[i], *player_fish);
		update_fish_life(&fish[i], level_data);
	}
	update_star(star);
	check_fish_bitten(player_fish, fish, player_data);

	if (bitmap_collision(bitmap_named("star"), star->x, star->y, bitmap_named(player_fish_name), player_fish->x, player_fish->y)) {
		player_data->score	+= STAR_SCORE_BONUS;
		player_data->progress += STAR_PROGRESS_BONUS;
		init_star(star);
	}
	if (player_data->progress < level_data.milestone[0])
		player_fish->size = SMALL;
	else if (player_data->progress < level_data.milestone[1])
		player_fish->size = MEDIUM;
	else
		player_fish->size = BIG;
}
//
// initialize the fish data
//
void init_ingame(FISH * player_fish, FISH fish[], STAR * star, PLAYER_DATA * player_data, LEVEL_DATA level_data)
{
	*player_fish = init_player_data(level_data, SMALL);
	REP(i, 0, N_SMALL_FISH)
		init_single_fish(&fish[i], level_data, SMALL);
	REP(i, N_SMALL_FISH, N_SMALL_FISH + N_MEDIUM_FISH)
		init_single_fish(&fish[i], level_data, MEDIUM);
	REP(i, N_SMALL_FISH + N_MEDIUM_FISH, SUM_FISH)
		init_single_fish(&fish[i], level_data, BIG);
	player_data->progress = 0;
	if (player_data->life < 0) player_data->life = 3;
	init_star(star);
}
//
//procedure add a new high score
//
void add_new_high_score(int new_score, int score_array[], int *n)
{
	if (*n < 10) {
		score_array[*n] = new_score;
		*n += 1;
	} else {
		if (score_array[*n-1] >= new_score) return;
		else score_array[*n-1] = new_score;
	}
	int i = *n - 1;
	while (score_array[i-1] < score_array[i] && i > 0)
	{
		int t = score_array[i-1];
		score_array[i-1] = score_array[i];
		score_array[i] = t;
		i--;
	}
}
//
//draw back button
//
void draw_back_button(float x, float y, float w, float h)
{
	fill_rectangle(COLOR_SALMON, x, y, w, h);
	draw_text("BACK", COLOR_BLACK, "arial.ttf", 12, x + 3, y + 3);
}
//
//procedure to draw menu
//
void draw_menu()
{
	draw_bitmap("bgr_menu", 0, 0);
	fill_rectangle(COLOR_RED, MENU_BUTTON_COLUMN, 		1 * SCREEN_HEIGHT / 4, BUTTON_WIDTH, BUTTON_HEIGHT);
	fill_rectangle(COLOR_RED, MENU_BUTTON_COLUMN, 		2 * SCREEN_HEIGHT / 4, BUTTON_WIDTH, BUTTON_HEIGHT);
	fill_rectangle(COLOR_RED, MENU_BUTTON_COLUMN, 		3 * SCREEN_HEIGHT / 4, BUTTON_WIDTH, BUTTON_HEIGHT);

	draw_text("LET'S PLAY", 	COLOR_WHITE, "arial.ttf", 18, MENU_BUTTON_COLUMN + 10, 		1 * SCREEN_HEIGHT / 4 + 10);
	draw_text("HIGH_SCORE", 	COLOR_WHITE, "arial.ttf", 18, MENU_BUTTON_COLUMN + 10, 		2 * SCREEN_HEIGHT / 4 + 10);
	draw_text("EXIT GAME", 		COLOR_WHITE, "arial.ttf", 18, MENU_BUTTON_COLUMN + 10, 		3 * SCREEN_HEIGHT / 4 + 10);
}

void draw_level_map(int finished_level, int total_level)
{
	draw_bitmap("bgr_level", 0, 0);
	REP(i, 0, total_level)
		fill_circle((i <= finished_level) ? COLOR_RED : COLOR_WHITE, SCREEN_WIDTH/(N_LEVELS+1)*(i+1), SCREEN_HEIGHT / 2, LEVEL_BUTTON_HEIGHT);
	draw_back_button(BACK_X_POSITION, BACK_Y_POSITION, BACK_WIDTH, BACK_HEIGHT);
}

void draw_player_fish(float x, float y, char fish_name[], SIZE size, DIRECTION d)
{
	char name[50];
	strcpy(name, fish_name);
	strcat(name, (d == F_LEFT) ? "_left" : "_right");
	switch(size) {
		case SMALL:
			strcat(name, "_small");
			break;
		case MEDIUM:
			strcat(name, "_medium");
			break;
		case BIG:
			strcat(name, "_big");
			break;
		}
	draw_bitmap(name, x, y);
}
void draw_single_fish(float x, float y, char fish_name[], DIRECTION d)
{
	char name[50];
	strcpy(name, fish_name);
	strcat(name, (d == F_LEFT) ? "_left" : "_right");
	draw_bitmap(name, x, y);
}

void draw_fish(FISH player_fish, FISH fish[])
{
	draw_player_fish(player_fish.x, player_fish.y, player_fish.name, player_fish.size, player_fish.d);
	REP(i , 0, SUM_FISH)
		draw_single_fish(fish[i].x, fish[i].y, fish[i].name, fish[i].d);
}

void draw_star(STAR star)
{
	draw_bitmap("star", star.x, star.y);
}

void draw_player_data(int score, int progress, int life)
{
	char score_string[10], life_string[10];

	itoa(score, score_string, 10);
	itoa(life, 	life_string, 10);

	draw_text("SCORE: ", COLOR_WHITE, "arial.ttf", 14, SCREEN_WIDTH - 100, 10);
	draw_text(score_string, COLOR_WHITE, "arial.ttf", 14, SCREEN_WIDTH - 50, 10);

	draw_text("LIVES: ", COLOR_WHITE, "arial.ttf", 14, SCREEN_WIDTH - 100, 30);
	draw_text(life_string, COLOR_WHITE, "arial.ttf", 14, SCREEN_WIDTH - 50, 30);

	draw_text("PROGRESS: ", COLOR_WHITE, "arial.ttf", 30, 10);
	draw_rectangle(COLOR_WHITE, 100, 0, 300, 10);
	fill_rectangle(COLOR_SALMON, 100, 0, 300 / LEVEL_WINNING * progress, 10);
}

void draw_high_score(int h[], int n)
{
	REP(i, 0, n) {
		char index_string[10];
		itoa(i + 1, index_string, 10);
		strcat(index_string, ". ");
		char score_string[10];
		itoa(h[i], score_string, 10);
		strcat(index_string, score_string);
		draw_text(index_string, COLOR_WHITE, "arial.ttf", 18, 300, SCREEN_HEIGHT*(i+1)/(n+1));
	}
}

void draw_level_failed()
{
	fill_rectangle(COLOR_GREEN, 400, 300, 400, 200);
	draw_text("YOU FAILED !", COLOR_WHITE, "arial.ttf", 18, 500, 300);
}

void draw_level_won()
{
	fill_rectangle(COLOR_RED, 400, 300, 400, 200);
	draw_text("YOU WON !", COLOR_WHITE, "arial.ttf", 18, 500, 300);
	draw_bitmap("star", 100, SCREEN_HEIGHT/2);
}
//
//save player data to an external file
//
void draw_lose_message()
{
	fill_rectangle(COLOR_RED, SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2, 500, 100);
	draw_text("you lost your life", COLOR_WHITE, "arial.ttf", 18, SCREEN_WIDTH/2 - 250 + 10, SCREEN_HEIGHT/2 + 10);
	draw_text("Please move the mouse far away your enemies", COLOR_WHITE, "arial.ttf", 14, SCREEN_WIDTH/2 - 250 + 10, SCREEN_HEIGHT/2 + 30);

}
void save_player_data(PLAYER_DATA player_data)
{
	FILE * player_file_ptr = NULL;

	if ( (player_file_ptr = fopen("player.dat", "w")) != NULL )
		fwrite(&player_data, sizeof(player_data), 1, player_file_ptr);
	else printf("Error opening files");
	fclose(player_file_ptr);
}
//
//save high score data to an external file
//
void save_high_score_data(int n, int h[])
{
	FILE * high_score_file_ptr = NULL;

	if ( (high_score_file_ptr = fopen("highscore.dat", "w")) != NULL ) {
		fprintf(high_score_file_ptr, "%d ", n);
		REP(i, 0, n)
			fprintf(high_score_file_ptr, "%d ", h[i]);
	} else printf("Error opening files");
	fclose(high_score_file_ptr);
}

int main()
{
	STATUS current_status = MENU;
	int current_level = -1;
	FISH fish[SUM_FISH];
	FISH player_fish;
	bool is_ingame_init = false;
	STAR star;
	GAME_DATA game_data =  load_game();
	open_graphics_window("FEEDING FRENZY", SCREEN_WIDTH, SCREEN_HEIGHT);

	do {
		process_events();
		current_status = game_status_updated(current_status, game_data.p1.life, game_data.p1.progress);
		clear_screen(COLOR_BLACK);
		if (current_status == MENU) {
			show_mouse();
			draw_menu();
		}
		if (current_status == LEVEL_MAP) {
			show_mouse();
			draw_level_map(game_data.p1.highest_level, N_LEVELS);
			REP(i, 0, game_data.p1.highest_level + 1)
				if (button_clicked(SCREEN_WIDTH/(N_LEVELS+1)*(i+1) - LEVEL_BUTTON_WIDTH, SCREEN_HEIGHT / 2 - LEVEL_BUTTON_HEIGHT, LEVEL_BUTTON_WIDTH*2, LEVEL_BUTTON_HEIGHT*2)) {
					current_level = i;
					current_status = INGAME;
				}
			if (current_level != -1) current_status = INGAME;
		}

		if (current_status == INGAME) {
			//first check whether in_game data has been initialised already
			if (is_ingame_init == false) {
				init_ingame(&player_fish, fish, &star, &game_data.p1, game_data.level_data[current_level]);
				is_ingame_init = true;
			}

			if (is_ingame_init == true)
				update_ingame(&player_fish, fish, &star, &game_data.p1, game_data.level_data[current_level]);

			//draw all elements in-game
			draw_bitmap(game_data.level_data[current_level].bgr, 0, 0); 						//draw in-game background
			draw_fish(player_fish, fish);																						//draw fish
			draw_star(star);																	//draw star
			draw_player_data(game_data.p1.score, game_data.p1.progress, game_data.p1.life);		//draw player data: score, progress and remaining lives
			draw_back_button(BACK_X_POSITION, BACK_Y_POSITION, BACK_WIDTH, BACK_HEIGHT);		//draw button
			if (player_fish.isdead) draw_lose_message();
		}

		if (current_status == LEVEL_FAILED) {
			show_mouse();
			is_ingame_init = false;
			game_data.p1.score = 0;
			current_level = -1;
			draw_level_failed();
			draw_back_button(BACK_X_POSITION, BACK_Y_POSITION, BACK_WIDTH, BACK_HEIGHT);
			add_new_high_score(game_data.p1.score, game_data.high_score, &game_data.n);
		}

		if (current_status == LEVEL_WON) {
			show_mouse();
			draw_level_won();
			if (++current_level > game_data.p1.highest_level) (game_data.p1.highest_level)++	;
			is_ingame_init = false;
			current_level = -1;
			draw_back_button(BACK_X_POSITION, BACK_Y_POSITION, BACK_WIDTH, BACK_HEIGHT);
		}

		if (current_status == HIGH_SCORE) {
			show_mouse();
			draw_high_score(game_data.high_score, game_data.n);
			draw_back_button(BACK_X_POSITION, BACK_Y_POSITION, BACK_WIDTH, BACK_HEIGHT);
		}
			;
		//printf("current level: %d\nhighest level %d\n", current_level, game_data.p1.highest_level);
		refresh_screen(60);
	}
	while (! window_close_requested() && current_status != EXIT);

	save_player_data(game_data.p1);
	save_high_score_data(game_data.n, game_data.high_score);
	release_all_resources();

	return 0;
}
