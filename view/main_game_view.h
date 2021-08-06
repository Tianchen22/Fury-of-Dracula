#ifndef FOD__MAIN_GAME_VIEW_H_
#define FOD__MAIN_GAME_VIEW_H_

#include <stdbool.h>

#include "Game.h"
#include "Places.h"
#include "Player.h"

//#define NUM_TRAILS 5
typedef struct main_game_view* MainGameView;

typedef struct main_game_view {
	Round round;
  	Player current_player;
  	int score;
	player_t *players[NUM_PLAYERS];
	queue_t *traps[CITY_UNKNOWN+1];
	PlaceId vampire;
	bool track_minions; // ?
  	PlaceId last_loc;
  	Message msg[NUM_PLAYERS]; 
} main_game_view;

/**
 * Creates a new view to summarise the current state of the game.
 *
 * @param past_plays is a string of all plays made in the game so far by
 *    all players (including Dracula) from earliest to most recent.
 *
 * @param messages is an array containing a `Message` for each
 *    play in the game so far.  It will have exactly the same number of
 *    elements as there are plays in `past_plays`.  The message from the
 *    first play will be at index 0, and so on.  The contents of each
 *    `Message` will be exactly as provided by the player.
 *
 * The "Message" type is defined in game.h.
 * You are free to ignore messages if you wish.
 */
MainGameView main_gv_new (char *past_plays, Message messages[],
                            bool track_minions);

/**
 * Frees all resources allocated for `gv`.
 * After this has been called, `gv` should not be accessed.
 */
void main_gv_drop (MainGameView gv);

/**
 * Get the current round
 */
Round main_gv_get_round (MainGameView gv);

/**
 * Get the current player; effectively, whose turn is it?
 */
Player main_gv_get_player (MainGameView gv);

/**
 * Get the current score, a positive integer between 0 and 366.
 */
int main_gv_get_score (MainGameView gv);

/**
 * Get the current health points for a given player.
 * @param player specifies which players's life/blood points to return;
 * @returns a value between 0..9 for Hunters, or >0 for Dracula
 */
int main_gv_get_health (MainGameView gv, Player player);

/**
 * Get the current location of a given player.
 *
 * May be `UNKNOWN_LOCATION` if the player has not had a turn yet
 * (i.e., at the beginning of the game when the round is 0)
 *
 * Possible values include:
 * - in the interval 0..70, if the player was (known to be) in a
 *   particular city or on a particular sea;
 * - `CITY_UNKNOWN`, if Dracula was known to be in a city;
 * - `SEA_UNKNOWN`, if Dracula was known to be at sea;
 * - `HIDE`, if Dracula was known to have made a hide move;
 * - `DOUBLE_BACK_n`, where n is [1...5], if Dracula was known to have
 *   made a double back move _n_ positions back in the trail; e.g.,
 *   `DOUBLE_BACK_1` is the last place place he visited; or
 * - `TELEPORT`, if Dracula apparated back to Castle Dracula.
 */
PlaceId main_gv_get_location (MainGameView gv, Player player);

/**
 * Fills the trail array with the locations of the last 6 turns for the
 * given player.
 *
 * If the move does not exist yet (i.e., the start of the game),
 * the value should be UNKNOWN_LOCATION (-1).
 *
 * For example after 2 turns the array may have the contents
 *
 *     {29, 12, -1, -1, -1, -1}
 *
 * This would mean in the first move the player started on location 12
 * then moved to the current location of 29.
 */
void main_gv_get_history (
	MainGameView gv, Player player, PlaceId trail[TRAIL_SIZE]);

/**
 * Return an array of `PlaceId`s giving all of the locations that the
 * given `player` could reach from their current location, assuming it's
 * currently `round`.
 *
 * The array can be in any order but must contain unique entries.
 * The array size is stored at the variable referenced by `n_locations`.
 * The player's current location should be included in the array.
 *
 * `road`, `rail`, and `sea` connections should only be considered
 * if the `road`, `rail`, `sea` parameters are true, respectively.
 *
 * The function must take into account the current round and player for
 * determining how far `player` can travel by rail.
 *
 * When `player` is `PLAYER_DRACULA`, the function must take into
 * account (many of) the rules around Dracula's movements, such as that
 * Dracula may not go to the hospital, and may not travel by rail.
 * It need not take into account the trail restriction.
 */
void main_gv_get_connections_rec(int *n_locations, bool *can_go,
                                    PlaceId from, Player player,
                                    Round round, bool road, bool rail,
                                    bool sea, int max_rail_dist);

PlaceId *main_gv_player_get_connections(player_t *pobj, int *n_locations,
                                    PlaceId from, Player player,
                                    Round round, bool road, bool rail,
                                    bool sea, bool trail, bool stay, bool hide);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void main_gv_get_location_history(MainGameView gv, Player player,
                              PlaceId trail[TRAIL_SIZE]);

char* main_gv_get_msg(MainGameView gv, Player player); // may need
                            

char *parse_hunter_move(char *move, MainGameView gv,
                                      Player pid, PlaceId old_loc,
                                      PlaceId real_loc); // may need

void parse_hunter_encounter(MainGameView gv, Player pid,
                            PlaceId real_loc, char encounter);

char *parse_move(char *move, MainGameView gv);

player_t *main_gv_get_player_class(MainGameView gv, Player player);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PlaceId main_gv_get_real_location(MainGameView gv, Player player);// do we need this?

void main_gv_get_move_history(MainGameView gv, Player player,
                                PlaceId trail[TRAIL_SIZE]);// do we need this?

void main_gv_get_locale_info(MainGameView gv, PlaceId where, int *n_traps,
                                int *n_vamps); // do we need this?

void populate_messages(MainGameView gv,
                                    Message messages[]);
                                    
int get_rail_travel_dist(Round round, Player player);
player_t *_gv_get_player_class(MainGameView gv, Player player);

void parse_dracula_minion_placement(MainGameView gv, PlaceId real_loc,
                                    char minion); 

void parse_dracula_minion_left_trail(MainGameView gv, char left);

void parse_hunter_encounter(MainGameView gv, Player pid,
                            PlaceId real_loc, char encounter);

// Returns the locations of all active traps.
PlaceId *main_gv_get_trap_locations(MainGameView gv, int *numTraps);

// Returns the location of the sleeping immature vampire.
PlaceId main_gv_get_vampire_location(MainGameView gv);

// Returns all the moves that Dracula can validly make this turn. 
// Including HIDE, DOUBLEBACK 1-5
PlaceId *main_gv_get_valid_move(MainGameView gv, int *numReturnedMoves, PlaceId *place) ;

// Return the total locations of the player,
// numAllLocs returns the number of total locations 
PlaceId *main_gv_get_all_location(MainGameView gv, Player player, int *numAllLocs);

// Return the total moves of the player,
// numAllLocs returns the number of total moves 
PlaceId *main_gv_get_all_move(MainGameView gv, Player player, int *numAllLocs);

// Return the last numLocs' locations of the player, 
// numReturnedLocs returns the length of locations in case the total moves is smaller than numReturnedLocs
PlaceId *main_gv_get_last_location(MainGameView gv, Player player, int numLocs, int *numReturnedLocs);

// Return the last numMoves' moves of the player, 
// numReturnedMoves returns the length of moves in case the total moves is smaller than numMoves
PlaceId *main_gv_get_last_move(MainGameView gv, Player player, int numMoves, int *numReturnedMoves);

// Return the minimal distance from two distance. 
PlaceId minDistance(PlaceId *dist, PlaceId *vSet);

// Return the hunter's shortest path; pathLength will return the shortest path length 
PlaceId *main_gv_hunter_shortest_path(
					MainGameView gv, Player hunter, 
					PlaceId dest, int *pathLength);

PlaceId main_gv_last_known_dracula_loc(MainGameView gv, Round *round);

void hunter_lose_health(MainGameView gv, Player player,
                                      int lose);

char *parse_dracula_move(char *move, MainGameView gv,
                                       Player pid, bool is_sea,
                                       PlaceId real_loc);                                      
#endif // !defined (FOD__GAME_VIEW_H_)
