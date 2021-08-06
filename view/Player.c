#include <memory.h>
#include <stdbool.h>
#include <string.h>

#include "Game.h"
#include "Player.h"
#include "Places.h"

player_t *new_player(Player id, bool track_all_history) 
{
    player_t *player = malloc(sizeof(player_t));
    player->id = id;
    player->staycount = 0;
    if (id == PLAYER_DRACULA)
        player->health = GAME_START_BLOOD_POINTS;
    else
        player->health = GAME_START_HUNTER_LIFE_POINTS;
    player->location = NOWHERE;
    player->move = NOWHERE;
    player->trail = new_queue(TRAIL_SIZE);
    player->location_history = new_queue(TRAIL_SIZE);
    if (track_all_history) {
        player->all_location_history = malloc(MAX_ROUNDS * sizeof(PlaceId));
        player->all_move_history = malloc(MAX_ROUNDS * sizeof(PlaceId));
        memset(player->all_location_history, NOWHERE,
            MAX_ROUNDS * sizeof(PlaceId));  // -1
        memset(player->all_move_history, NOWHERE,
            MAX_ROUNDS * sizeof(PlaceId));  // -1
        player->all_history_size = 0;
    } else {
        player->all_history_size = -1;
    }
    player->neverdie = false;

    return player;
}

player_t *clone_player(player_t *p) 
{
    player_t *new = malloc(sizeof(player_t));
    new->id = p->id;
    new->health = p->health;
    new->staycount = p->staycount;
    new->neverdie = p->neverdie;
    new->location = p->location;
    new->move = p->move;
    new->trail = clone_queue(p->trail);
    new->location_history = clone_queue(p->location_history);
    new->all_history_size = p->all_history_size;

    return new;
}

void destroy_player(player_t *player) 
{
    destroy_queue(player->trail);
    destroy_queue(player->location_history);
    if (player->all_history_size >= 0) {
        free(player->all_location_history);
        free(player->all_move_history);
    }
    free(player);
}

int player_get_health(player_t *player) 
{ 
    return player->health; 
}

bool player_lose_health(player_t *player, int lose) 
{
    printf("Player %d with health %d lost health %d\n", player->id,
          player->health, lose);
    player->health -= lose;
    if (player->health <= 0) {
        player->health = 0;
        return true;
    }
    return false;
}

PlaceId player_get_location(player_t *player) 
{ 
    return player->location; 
}

void player_get_trail(player_t *player, PlaceId trail[TRAIL_SIZE]) 
{
    queue_to_array(player->trail, trail, true);
}

void player_get_location_history(player_t *player,
                                 PlaceId history[TRAIL_SIZE]) 
{
    queue_to_array(player->location_history, history, true);
}

PlaceId *player_get_all_location_history(player_t *player) 
{
    return player->all_location_history;
}

// NOTE: this doesn't take care of value check, losing blood, and spawning
// minions
void player_move_to(player_t *player, PlaceId location, PlaceId move) 
{
    if (location == player->location)
        player->staycount++;
    else {
        if (player->staycount >= 5) player->neverdie = true;
        player->staycount = 0;
    }
    player->location = location;
    player->move = move;
    queue_add(player->trail, move);
    queue_add(player->location_history, location);
    if (player->all_history_size >= 0) {
        player->all_location_history[player->all_history_size] = location;
        player->all_move_history[player->all_history_size++] = move;
    }
}

PlaceId player_resolve_move_location(player_t *player, PlaceId move) 
{
    if (move == HIDE) return player->location;
    if (move >= DOUBLE_BACK_1 && move <= DOUBLE_BACK_5)
        return queue_get_backwards(player->location_history,
                                            move - DOUBLE_BACK_1);
    if (move == TELEPORT) return CASTLE_DRACULA;
    return move;
}

Player player_id_from_char(char player) 
{
    switch (player) {
        case 'G':
            return PLAYER_LORD_GODALMING;
        case 'S':
            return PLAYER_DR_SEWARD;
        case 'H':
            return PLAYER_VAN_HELSING;
        case 'M':
            return PLAYER_MINA_HARKER;
        case 'D':
            return PLAYER_DRACULA;
        default:
        printf("Unknown player");
    }
    __builtin_unreachable();
}
