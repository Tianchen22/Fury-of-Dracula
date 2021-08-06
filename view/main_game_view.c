// COMP2521 20T2 ... the Fury of Dracula
// main_game_view.c: GameView ADT implementation

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <err.h>
#include <math.h>
#include <memory.h>
#include <stdbool.h>
#include <string.h>
#include <sysexits.h>

#include "Game.h"
#include "GameView.h"
#include "Map.h"
#include "main_game_view.h"
#include "Places.h"

#define min(a,b) (a) < (b) ? (a) : (b)
#define max(a,b) (a) > (b) ? (a) : (b)

MainGameView main_gv_new (char *past_plays, Message messages[],
                            bool track_minions) 
{
	// log print
	printf("NEW GAME\n============\n");
  	printf("PAST PLAYER: %s\n", past_plays);

  	main_game_view *gv = malloc(sizeof *gv);
  	gv->round = 0;
  	gv->current_player = 0;
  	gv->score = GAME_START_SCORE;
  	gv->vampire = NOWHERE;
  	gv->track_minions = track_minions;
  	gv->last_loc = NOWHERE;

  	for (int i = 0; i < NUM_PLAYERS; i++) 
	  	gv->msg[i][0] = '\0';

  	for (int i = MIN_REAL_PLACE; i <= CITY_UNKNOWN; i++)
    	gv->traps[i] = new_queue(4);

  	for (int i = 0; i < NUM_PLAYERS; i++) 
	  	gv->players[i] = new_player((Player)i, true); //  new_player to be done

  	while (*past_plays != '\0') 
	  	past_plays = parse_move(past_plays, gv); // process_move to be done

  	populate_messages(gv, messages); // populate_messages to be done

  	return gv;
}

void main_gv_drop (MainGameView gv)
{
	for (int i = 0; i < NUM_PLAYERS; i++) 
		destroy_player(gv->players[i]);

  	for (int i = MIN_REAL_PLACE; i <= CITY_UNKNOWN; i++)
    	destroy_queue(gv->traps[i]);

  	free(gv);
}

Round main_gv_get_round(MainGameView gv) 
{ 
	return gv->round;
}

Player main_gv_get_player (MainGameView gv) 
{
	return gv->current_player;
}

int main_gv_get_score(MainGameView gv) 
{
	return gv->score;
}

int main_gv_get_health (MainGameView gv, Player player) 
{
	return gv->players[player]->health;
}

PlaceId main_gv_get_location (MainGameView gv, Player player) 
{
	PlaceId last_move = queue_last(gv->players[player]->trail);
  	if ((last_move >= MIN_REAL_PLACE && last_move <= MAX_REAL_PLACE)
		|| (last_move >= HIDE && last_move <= TELEPORT)){
    	return gv->players[player]->location;
	}

  	return last_move;
}

void main_gv_get_move_history(MainGameView gv, Player player,
                          PlaceId trail[TRAIL_SIZE]) 
{
  	player_get_trail(gv->players[player], trail);
}

void main_gv_get_location_history(MainGameView gv, Player player,
                              PlaceId trail[TRAIL_SIZE]) 
{
	player_get_location_history(gv->players[player], trail);
}

void main_gv_get_connections_rec(int *n_locations, bool *can_go,
                                    PlaceId from, Player player,
                                    Round round, bool road, bool rail,
                                    bool sea, int max_rail_dist) 
{
  	if ((!road) && ((!rail) || max_rail_dist <= 0) && (!sea)) return;

  	struct adj_connection *conns = get_connections(from);

  	for (int i = 0; i < ADJLIST_COUNT[from]; i++) {
    	if (player == PLAYER_DRACULA && conns[i].v == HOSPITAL_PLACE) continue;
    	if (conns[i].type == ROAD && road) {
      		if (!can_go[conns[i].v]) {
        		can_go[conns[i].v] = true;
        		(*n_locations)++;
      		}
    	} else if (conns[i].type == BOAT && sea) {
      		if (!can_go[conns[i].v]) {
	        	can_go[conns[i].v] = true;
	        	(*n_locations)++;
      		}
   		} else if (conns[i].type == RAIL && rail && max_rail_dist > 0 &&
            conns[i].v != from) {
	      	if (!can_go[conns[i].v]) {
	        	can_go[conns[i].v] = true;
	        	(*n_locations)++;
      		}

      		main_gv_get_connections_rec(n_locations, can_go, conns[i].v, player, round,
                              false, rail, false, max_rail_dist - 1);
    	}
  	}			
}

PlaceId *main_gv_player_get_connections(player_t *pobj, int *n_locations,
                                    PlaceId from, Player player,
                                    Round round, bool road, bool rail,
                                    bool sea, bool trail, bool stay, bool hide) 
{
	printf("invoking main_gv_player_get_connections with settings: road %d rail %d sea %d trail %d stay %d hide %d\n",
        road, rail, sea, trail, stay, hide);

  	if (from < MIN_REAL_PLACE ||
      	from > MAX_REAL_PLACE) {  // don't know exact loc
    	*n_locations = 0;
    	return NULL;
  	}

  	if (player == PLAYER_DRACULA) rail = false;

  	bool can_go[NUM_REAL_PLACES];
  	bool doublebacks[5];
 	memset(can_go, 0, NUM_REAL_PLACES);
  	memset(doublebacks, 0, 5);
  	*n_locations = 0;
  	int dbs = 0;
  	int max_rail_dist = 0;
			  				
  	if (rail) {
    	max_rail_dist = get_rail_travel_dist(round, player);
	}

	main_gv_get_connections_rec(n_locations, can_go, from, player, round, road, rail,
                          sea, max_rail_dist);

  	bool candb = hide && player == PLAYER_DRACULA;
  	bool canhide = candb && placeIdToType(from) != SEA;
  	if (trail) {
    	PlaceId hist[TRAIL_SIZE];
    	PlaceId tr[TRAIL_SIZE];
    	player_get_location_history(pobj, hist);
    	player_get_trail(pobj, tr);
    	for (int i = TRAIL_SIZE - 2; i >= 0; i--) {
      		if (hist[i] >= MIN_REAL_PLACE && hist[i] <= MAX_REAL_PLACE) {
        		if (candb &&
					(from == hist[i] ||
					is_connected_via(hist[i], from, 5))) {  // 101: boat and road
						doublebacks[i] = true;
						dbs++;
					}
      		}

      		if (tr[i] >= MIN_REAL_PLACE && tr[i] <= MAX_REAL_PLACE) {
				if (can_go[tr[i]]) {
				can_go[tr[i]] = false;
				(*n_locations)--;
				}
			} else if (tr[i] == HIDE) {
				canhide = false;
			} else if (tr[i] >= DOUBLE_BACK_1 && tr[i] <= DOUBLE_BACK_5) {
				candb = false;			
			}
		}
  	}

  	if (canhide) (*n_locations)++;

  	if (stay && (!can_go[from])) {
    	can_go[from] = true;
    	(*n_locations)++;
  	}

  	if (candb) *n_locations += dbs;
  	if (hide && (*n_locations) == 0) *n_locations = 1;

  	PlaceId *valid_conns = malloc(sizeof(PlaceId) * (size_t)(*n_locations));
                    
  	int j = 0;
  	for (int i = 0; i < NUM_REAL_PLACES; i++) {
    	if (can_go[i]) valid_conns[j++] = i;
	}
	if (canhide) valid_conns[j++] = HIDE;
  	if (candb) {
    	for (int i = 0, k = DOUBLE_BACK_1; i < 5; i++, k++) {
	      	if (doublebacks[i]) {
	        	valid_conns[j++] = k;
      		}
    	}
  	}
  	if (hide && j == 0) 
    	valid_conns[0] = TELEPORT;

	return valid_conns;
}

char* main_gv_get_msg(MainGameView gv, Player player) 
{
	return gv->msg[player];
}

void hunter_lose_health(MainGameView gv, Player player,
                                      int lose) 
{
	if (player_lose_health(gv->players[player], lose)) {
		gv->players[player]->location = HOSPITAL_PLACE;
		gv->score -= SCORE_LOSS_HUNTER_HOSPITAL;
	}
}

char *parse_hunter_move(char *move,  MainGameView gv,
                                      Player pid, PlaceId old_loc,
                                      PlaceId real_loc) 
{
	for (int i = 0; i < 4; i++)
    	parse_hunter_encounter(gv, pid, real_loc, *(move++));

  	if (old_loc == gv->players[pid]->location && gv->players[pid]->health > 0) {
    	gv->players[pid]->health += LIFE_GAIN_REST;
    	printf("player %d gain health %d to %d\n", pid, LIFE_GAIN_REST,
           gv->players[pid]->health);
    	if (gv->players[pid]->health > GAME_START_HUNTER_LIFE_POINTS)
      		gv->players[pid]->health = GAME_START_HUNTER_LIFE_POINTS;

  	}

  	gv->current_player++;

  	return move;	
}

char *parse_dracula_move(char *move, MainGameView gv,
                                       Player pid, bool is_sea,
                                       PlaceId real_loc) 
{
	if (is_sea) player_lose_health(gv->players[pid], LIFE_LOSS_SEA);
	if (real_loc == CASTLE_DRACULA)
		gv->players[pid]->health += LIFE_GAIN_CASTLE_DRACULA;

	// parse minion
	parse_dracula_minion_placement(gv, real_loc, *(move++));
	parse_dracula_minion_placement(gv, real_loc, *(move++));

	// parse left trail
	parse_dracula_minion_left_trail(gv, *move);
	move += 2;

	gv->round++;
	gv->current_player = 0;
	gv->score -= SCORE_LOSS_DRACULA_TURN;

	return move;
}

void parse_dracula_minion_left_trail(MainGameView gv, char left) 
{
	if (left == 'M') {
		printf("trap invalidates\n");
		if (gv->track_minions && gv->round >= TRAIL_SIZE) {
			queue_remove_first(gv->traps[gv->last_loc]);
			printf("at %s(%s), there are %d left\n",
				placeIdToAbbrev(gv->last_loc),
				placeIdToName(gv->last_loc),
				queue_size(gv->traps[gv->last_loc]));
		}
	} else if (left == 'V') {
		printf("vampire matures\n");
		if (gv->track_minions) gv->vampire = NOWHERE;
		gv->score -= SCORE_LOSS_VAMPIRE_MATURES;
	}
}

void parse_dracula_minion_placement(MainGameView gv, PlaceId real_loc,
                                    char minion) 
{
  if (minion == 'T') {
    printf("placed trap at %s(%s)\n", placeIdToAbbrev(real_loc),
           placeIdToName(real_loc));
    if (gv->track_minions) {
      	queue_add(gv->traps[real_loc], main_gv_get_round(gv));
      	printf("there are %d traps there\n",
            queue_size(gv->traps[real_loc]));
    }
  } else if (minion == 'V') {
    printf("placed vampire as %s(%s)\n",
           placeIdToAbbrev(real_loc), placeIdToName(real_loc));
    if (gv->track_minions) gv->vampire = real_loc;
  }
}


void parse_hunter_encounter(MainGameView gv, Player pid,
                            PlaceId real_loc, char encounter) 
{
	switch (encounter) {
		case 'T':
		printf("encounter trap\n");
		if (gv->track_minions)
			queue_remove_first(gv->traps[real_loc]);
		hunter_lose_health(gv, pid, LIFE_LOSS_TRAP_ENCOUNTER);
		break;
		case 'V':
		printf("encounter immature vampire\n");
		if (gv->track_minions) gv->vampire = NOWHERE;
		break;
		case 'D':
		printf("encounter dracula");
		player_lose_health(gv->players[PLAYER_DRACULA],
							LIFE_LOSS_HUNTER_ENCOUNTER);
		hunter_lose_health(gv, pid, LIFE_LOSS_DRACULA_ENCOUNTER);
		break;
		default:
		// encountered nothing
		break;
	}
}

PlaceId location_find_by_abbrev (char *abbrev)
{
    // an attempt to optimise a linear search
    struct place *first = &PLACES[MIN_REAL_PLACE];
    struct place *last = &PLACES[MAX_REAL_PLACE];

    for (struct place *p = first; p <= last; p++) {
        if (
            p->abbrev[0] == abbrev[0] &&
            p->abbrev[1] == abbrev[1] &&
            p->abbrev[2] == '\0'
        )
            return p->id;
	}
	
	if (abbrev[0] == 'C' && abbrev[1] == '?') {
        return CITY_UNKNOWN;
    } else if (abbrev[0] == 'S' && abbrev[1] == '?') {
        return SEA_UNKNOWN;
    } else if (abbrev[0] == 'H' && abbrev[1] == 'I') {
        return HIDE;
    } else if (abbrev[0] == 'D' && abbrev[1] == '1') {
        return DOUBLE_BACK_1;
    } else if (abbrev[0] == 'D' && abbrev[1] == '2') {
        return DOUBLE_BACK_2;
    } else if (abbrev[0] == 'D' && abbrev[1] == '3') {
        return DOUBLE_BACK_3;
    } else if (abbrev[0] == 'D' && abbrev[1] == '4') {
        return DOUBLE_BACK_4;
    } else if (abbrev[0] == 'D' && abbrev[1] == '5') {
        return DOUBLE_BACK_5;
    } else if (abbrev[0] == 'T' && abbrev[1] == 'P') {
        return TELEPORT;
    }

    return NOWHERE;
}

char *parse_move(char *move, MainGameView gv) 
{
  // move format (hunter):
  // Player[1] Location[2] Encounter[4]
  // move format (dracula):
  // Player[1] Location[2] Minion[2] LeftTrail[1] .
	if (*move == ' ') move++;
	
  // parse player
  	Player pid = player_id_from_char(*move);
  	move++;

  // parse location
  	PlaceId loc = NOWHERE;
  	bool is_sea = false;
  	if (pid == PLAYER_DRACULA) {
    	// loc = placeAbbrevToId(Move);
		loc = location_find_by_abbrev (move);
    	if (loc == SEA_UNKNOWN || placeIdToType(loc) == SEA) is_sea = true;
  	} else if (gv->players[pid]->health <= 0) {
    	gv->players[pid]->health = GAME_START_HUNTER_LIFE_POINTS;
  	}

  	if (loc == NOWHERE) {
    	loc = location_find_by_abbrev (move);
    	is_sea = (placeIdToType(loc) == SEA);
  	}

  	PlaceId real_loc = loc;

  	if ((loc >= DOUBLE_BACK_1 && loc <= DOUBLE_BACK_5) || loc == HIDE) {
    	int b = (loc == HIDE) ? 0 : loc - DOUBLE_BACK_1;  // 1: stay at current
    	real_loc =
        	queue_get_backwards(gv->players[pid]->location_history, b);
    	is_sea = (real_loc == SEA_UNKNOWN) ||
            (real_loc >= MIN_REAL_PLACE && real_loc <= MAX_REAL_PLACE &&
           	placeIdToType(real_loc) == SEA);
  	} else if (loc == TELEPORT) {
    	real_loc = CASTLE_DRACULA;
  	}

  	PlaceId old_loc = player_get_location(gv->players[pid]);

  	if (gv->round >= TRAIL_SIZE) {
    	gv->last_loc =
        	queue_get(gv->players[pid]->location_history, 0);
  	}

  	printf("player %d at %s (%s) makes move %s (%s) to %s (%s)\n", pid,
        placeIdToAbbrev(old_loc), placeIdToName(old_loc),
        placeIdToAbbrev(loc), placeIdToName(loc),
        placeIdToAbbrev(real_loc), placeIdToName(real_loc));
  	player_move_to(gv->players[pid], real_loc, loc);
	
  	move += 2;

  	if (pid == PLAYER_DRACULA)
    	return parse_dracula_move(move, gv, pid, is_sea, real_loc);
  	return parse_hunter_move(move, gv, pid, old_loc, real_loc);
}

void populate_messages(MainGameView gv, Message messages[]) 
{
  	if (messages == NULL) return;
 	if (gv->current_player == 0) return;
  	int message_counter = gv->round * NUM_PLAYERS + (int)gv->current_player - 1;
  	printf("round %d %d message counter %d\n", gv->round,
        gv->current_player, message_counter);
  	for (int player_counter = (int)gv->current_player - 1; player_counter > -1;
       	player_counter--) {
    	printf("message counter %d\n", message_counter);
    	strncpy(gv->msg[player_counter], messages[message_counter],
            MESSAGE_SIZE);
    	gv->msg[player_counter][MESSAGE_SIZE - 1] = '\0';
    	printf("received message for player %d: %s\n", player_counter,
           	gv->msg[player_counter]);
    	message_counter--;
  	}	  
}

int get_rail_travel_dist(Round round, Player player) 
{
	if (player == PLAYER_DRACULA) return 0;
	return (round + (int)player) % 4;
}

void main_gv_get_locale_info(MainGameView gv, PlaceId where, int *n_traps,
                                int *n_vamps)
{
	assert(gv->track_minions);
  	*n_vamps = (gv->vampire == where);
  	*n_traps = queue_size(gv->traps[where]);								
}

player_t *main_gv_get_player_class(MainGameView gv, Player player) 
{
	return gv->players[player];
}

PlaceId *main_gv_get_trap_locations(MainGameView gv, int *numTraps)
{
	PlaceId *place = malloc(sizeof(PlaceId));
	*numTraps = 0;
	for (int i = MIN_REAL_PLACE; i <= MAX_REAL_PLACE; i++) { 
		int size = queue_size(gv->traps[i]);
		*numTraps += size;
		place = realloc(place, (size_t)(*numTraps + 1) * sizeof(PlaceId));
		for (int j = 0; j < size; j++) {
			place[*numTraps - size + j] = i; // PLACES[i].id
		}		
	}

	return place;
}

PlaceId main_gv_get_vampire_location(MainGameView gv)
{
	return gv->vampire;
}

PlaceId *main_gv_get_last_move(MainGameView gv, Player player, int numMoves, int *numReturnedMoves) 
{
	int size = min((int)gv->players[player]->all_history_size, numMoves);
	*numReturnedMoves = size;
	PlaceId *place = malloc(min((size_t)size,1) * sizeof(PlaceId));
	for (int i = 1; i <= size; i++) {
		place[size - i] = gv->players[player]->all_move_history[(int)gv->players[player]->all_history_size - i];
	}

	return place;
}

PlaceId *main_gv_get_last_location(MainGameView gv, Player player, int numLocs, int *numReturnedLocs) 
{
	int size = min((int)gv->players[player]->all_history_size,numLocs);
	PlaceId *place = malloc((size_t)size * sizeof(PlaceId));
	for (int i = 1; i <= size; i++) {
		place[size - i] = gv->players[player]->all_location_history[(int)gv->players[player]->all_history_size - i];
	}
	*numReturnedLocs = size;

	return place;
}

PlaceId *main_gv_get_all_move(MainGameView gv, Player player, int *numAllLocs) {
	*numAllLocs = gv->players[player]->all_history_size;

	return gv->players[player]->all_move_history;
}

PlaceId *main_gv_get_all_location(MainGameView gv, Player player, int *numAllLocs) {
	*numAllLocs = gv->players[player]->all_history_size;

	return gv->players[player]->all_location_history;
}

// helper
PlaceId minDistance(PlaceId *dist, PlaceId *vSet) 
{ 
    // Initialize min value 
    int min = 500;
    int min_index = 0; 
  
    for (int i = 0; i < NUM_REAL_PLACES; i++){
        if (vSet[i] == 0 && dist[i] <= min){
            min = dist[i];
            min_index = i; 
        }
    }

    return min_index; 
} 

PlaceId *main_gv_hunter_shortest_path(
					MainGameView gv, Player hunter, 
					PlaceId dest, int *pathLength)
{
	//(Graph g, Vertex src, Vertex dest, int max, int *path)
	PlaceId src = main_gv_get_location(gv, hunter);
	PlaceId *dist = malloc(sizeof(PlaceId) * NUM_REAL_PLACES);
	PlaceId *pred = malloc(sizeof(PlaceId) * NUM_REAL_PLACES);
	PlaceId *vSet = malloc(sizeof(PlaceId) * NUM_REAL_PLACES);
	PlaceId *path = malloc(sizeof(PlaceId) * NUM_REAL_PLACES);
	for (int i = 0; i < NUM_REAL_PLACES; i++){
	    dist[i] = 50;               //dictance (prevent overflow)
	    pred[i] = -1;               //pred
	    vSet[i] = 0;                //checked
		path[i] = -1;
	}
    dist[src] = 0;
    pred[src] = src;
    
    for(int check = 0; check < (NUM_REAL_PLACES-1); check++){
        //find minimum dist
        PlaceId curr_src = minDistance(dist, vSet);
        vSet[curr_src] = 1;
        // find where i can go in current round
        Player curr_player = gv->players[hunter]->id;
		// PlaceId *nearBy = malloc(sizeof(PlaceId) * NUM_REAL_PLACES)
		int numReturnedLocs = -1;
		//round fix based on if this player has moved in current round
		int round_fix;
		if(main_gv_get_round(gv) == gv->players[curr_player]->all_history_size){
			round_fix = main_gv_get_round(gv) + dist[curr_src];
		} else {
			round_fix = main_gv_get_round(gv) + dist[curr_src] + 1;
		}
		PlaceId *nearBy = main_gv_player_get_connections(
						gv->players[curr_player], &numReturnedLocs,
						curr_src, curr_player,
						round_fix, true, true, 
						true, true, true, false);
		

		//RELAX
		for(int i = 0; i < numReturnedLocs; i++){
			if(vSet[nearBy[i]] == 0
				&& (dist[curr_src] + 1) < dist[nearBy[i]])
			{
				dist[nearBy[i]] = dist[curr_src] + 1; //g->edges[v][w]);
				pred[nearBy[i]] = curr_src;
			}
		}
		free(nearBy);
    }

	assert(pred[dest] != -1);
    
	int length = 0;
	PlaceId temp = dest;
	while(temp != src){
	    //find pred
	    length++;
	    for(int i = length; i >= 0; i--){
	        // pushes *path
	        path[i+1] = path[i];
	    }
	    path[0] = temp;
	    temp = pred[temp];
	}
	assert(temp == src);
	free(dist);
	free(pred);
	free(vSet);
	*pathLength = length;

	return path;
}

PlaceId main_gv_last_known_dracula_loc(MainGameView gv, Round *round)
{
	PlaceId pid = NOWHERE;
	*round = -1;
	//find the last common city (same/consequetive rounds) in hunter & dracula trail history
	PlaceId *dracula_trail = player_get_all_location_history(gv->players[PLAYER_DRACULA]);
	// find the latest round which hunter meets dracula
	// upon find, update 
	int j = 0;
	while(j <= main_gv_get_round(gv)){
		if (dracula_trail[j] <= MAX_REAL_PLACE 
			&& dracula_trail[j] >= MIN_REAL_PLACE
			&& *round < j)
		{
			// as dracula move later than hunters
			// hunter_hist[n+1] is compared with drac_hist[n]
			*round = j;
			pid = dracula_trail[j];
		}
		j++;
	}
	
	return pid;
} 
