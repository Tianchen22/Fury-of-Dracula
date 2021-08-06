#include <stdbool.h>
#include <stdio.h>

#include "Game.h"
#include "Places.h"
#include "Queue.h"

#ifndef FOD__PLAYER_H_
#define FOD__PLAYER_H_

#define MAX_ROUNDS 366

typedef struct player_t {
    Player id;  // who is it
    int health;
    PlaceId location;
    PlaceId move;
    queue_t *trail;
    queue_t *location_history;
    Round all_history_size;
    PlaceId *all_location_history;
    PlaceId *all_move_history;
    int staycount;
    bool neverdie;
} player_t;

player_t *new_player(Player id, bool track_all_history);
player_t *clone_player(player_t *p);
void destroy_player(player_t *player);
int player_get_health(player_t *player);
bool player_lose_health(player_t *player, int lose);
PlaceId player_get_location(player_t *player);
void player_get_trail(player_t *player, PlaceId trail[TRAIL_SIZE]);
void player_get_location_history(player_t *player,
                                 PlaceId history[TRAIL_SIZE]);
PlaceId player_resolve_move_location(player_t *player, PlaceId move);
PlaceId *player_get_all_location_history(player_t *player);
void player_move_to(player_t *player, PlaceId location, PlaceId move);
Player player_id_from_char(char player);

#endif 
