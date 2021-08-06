////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// hunter.c: your "Fury of Dracula" hunter AI.
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "Game.h"
#include "hunter.h"
#include "HunterView.h"
#include "main_game_view.h"
#include "Map.h"
#include "Mapdata.h"
#include "Player.h"
#include "Places.h"

// #define DEBUG_AS_ERROR

#ifdef HUNTER_SEARCH_FAST_MODE  // for testing
#define MAX_SCENARIOS 10000
#define CHECK_TIME_INTERVAL 255
#define SEARCH_ALLOWED_TIME 100000  // usec - 0.1s
#else                               // prod settings
#define MAX_SCENARIOS 500000
#define CHECK_TIME_INTERVAL 32767
#define SEARCH_ALLOWED_TIME 900000  // usec - 0.9s
#endif

#ifdef DEBUG_AS_ERROR
#define HUNT_LOG AC_LOG_ERROR
#else
#define HUNT_LOG AC_LOG_INFO
#endif

typedef struct move_place {
  player_t *player;
  struct move_place *prev;
  struct move_place *next;
} move_place;

typedef struct hunterView
{
    MainGameView gv;
} hunterView;

static inline bool isValidLoc(PlaceId loc) {
  return loc >= MIN_REAL_PLACE && loc <= MAX_REAL_PLACE;
}

static inline int randint(int max) {
  return rand() % max;
}

static inline char *
str_ncpy(char *dest, const char *src, int n)
{
    int i;

    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for ( ; i < n; i++)
        dest[i] = '\0';

   return dest;
}

static int probs[NUM_REAL_PLACES];

static struct timeval start_time;

static PlaceId resolve_loc_backwards(player_t *player,
                                        const PlaceId *hist, Round round,
                                        PlaceId move) {
  PlaceId ret = player_resolve_move_location(player, move);
  if (ret != NOWHERE) return ret;
  while (move == HIDE || (move >= DOUBLE_BACK_1 && move <= DOUBLE_BACK_5)) {
    round -= (move == HIDE) ? 1 : (move - DOUBLE_BACK_1 + 1);
    move = hist[round];
  }
  if (move == TELEPORT) return CASTLE_DRACULA;
  return move;
}

bool getPossibleDraculaLocations(player_t *players[], Round round) {
  memset(probs, 0, sizeof(probs));
  Round last_known_round;
  PlaceId last_known_location = NOWHERE;
  for (last_known_round = round - 1;
       (!isValidLoc(last_known_location)) && last_known_round > -1;
       last_known_round--) {
    last_known_location =
        players[PLAYER_DRACULA]->all_location_history[last_known_round];
    if (last_known_location == TELEPORT) last_known_location = CASTLE_DRACULA;
  }
  printf("last_known_round %d last_known_location %s\n",
         last_known_round, placeIdToAbbrev(last_known_location));
  
  if (!isValidLoc(last_known_location)) return false;
  move_place *start = malloc(sizeof(move_place));
  start->player = new_player(PLAYER_DRACULA, false);
  player_move_to(start->player, last_known_location, last_known_location);
  start->prev = start->next = NULL;
  move_place *end = start;
  int MoveCount = 1;
  int iterations = 1;
  for (last_known_round += 2; last_known_round < round; last_known_round++) {
    PlaceId loc =
        players[PLAYER_DRACULA]->all_location_history[last_known_round];
    // move_place **l = &start;
    bool cont = true;
    if (MoveCount == 0) {
      return false;
    }

    for (move_place *oend = end, *i = start; cont && i != oend->next;
         iterations++) {
      // NOTES: CHECK_TIME_INTERVAL must be power of 2 - 1
      if (!(iterations & CHECK_TIME_INTERVAL)) {
        static struct timeval currentTime;
        gettimeofday(&currentTime, NULL);
        // NOTES: won't overflow since sec difference is at most 1 or 2.
        int time_elapsed = (currentTime.tv_sec - start_time.tv_sec) * 1000000 +
                           currentTime.tv_usec - start_time.tv_usec;

        if (time_elapsed >= SEARCH_ALLOWED_TIME) {
          for (move_place *td = start; td != NULL; td = td->next) {
            if (td->prev != NULL) {
              destroy_player(td->prev->player);
              free(td->prev);
            }
          }
          destroy_player(end->player);
          free(end);
          printf("reached SEARCH_ALLOWED_TIME\n");
          return false;
        }
      }
      int plays = 0;
      PlaceId firstPlay;
      if (loc == HIDE || (loc >= DOUBLE_BACK_1 && loc <= DOUBLE_BACK_5)) {
        PlaceId resolveLoc = resolve_loc_backwards(
            i->player, players[PLAYER_DRACULA]->all_location_history,
            last_known_round, loc);

        if (resolveLoc >= MIN_REAL_PLACE && resolveLoc <= MAX_REAL_PLACE) {
          if ((loc == HIDE && placeIdToType(resolveLoc) == LAND) ||
              (loc > HIDE &&
               (is_connected_via(i->player->location, resolveLoc, 5) ||
                resolveLoc == i->player->location))) {  // 101: boat and road
            firstPlay = loc;
            plays++;
          }
        } else {
          loc = resolveLoc;
        }
      }
      if (loc == SEA_UNKNOWN || loc == CITY_UNKNOWN) {
        int n_locations = 0;
        PlaceId *moves;
        
        if (loc == SEA_UNKNOWN) {
          moves = main_gv_player_get_connections(
              i->player, &n_locations, i->player->location, PLAYER_DRACULA,
              last_known_round, false, false, true, true, false, false);
        } else {
          // NOTES: boat is set to true (go to city from sea via boat)
          moves = main_gv_player_get_connections(
              i->player, &n_locations, i->player->location, PLAYER_DRACULA,
              last_known_round, true, false, true, true, false, false);
        }
        printf("debug:get_connections ret: %d\n", n_locations);
        for (int j = 0; j < n_locations; j++) {
          printf("debug:Consider %s", placeIdToAbbrev(moves[j]));
          if ((placeIdToType(moves[j]) == LAND &&
               (loc == SEA_UNKNOWN ||
                (moves[j] ==
                 players[0]->all_location_history[last_known_round]) ||
                (moves[j] ==
                 players[1]->all_location_history[last_known_round]) ||
                (moves[j] ==
                 players[2]->all_location_history[last_known_round]) ||
                (moves[j] ==
                 players[3]->all_location_history[last_known_round]) ||
                (moves[j] ==
                 players[0]->all_location_history[last_known_round + 1]) ||
                (moves[j] ==
                 players[1]->all_location_history[last_known_round + 1]) ||
                (moves[j] ==
                 players[2]->all_location_history[last_known_round + 1]) ||
                (moves[j] ==
                 players[3]->all_location_history[last_known_round + 1]))) ||
              (loc == CITY_UNKNOWN && placeIdToType(moves[j]) == SEA))
            continue;
          if (plays == 0) {
            printf("debug:record firstPlay=%s",
                   placeIdToAbbrev(moves[j]));
            firstPlay = moves[j];
          } else {
            move_place *s = malloc(sizeof(move_place));
            s->player = clone_player(i->player);
            player_move_to(s->player, moves[j], moves[j]);
            s->next = NULL;
            s->prev = end;
            end->next = s;
            end = s;
            MoveCount++;
            printf("debug:clone move_place %p to %s", s,
                   placeIdToAbbrev(moves[j]));
          }
          plays++;
          if (MoveCount > MAX_SCENARIOS) {
            for (move_place *td = start; td != NULL; td = td->next) {
              if (td->prev != NULL) {
                destroy_player(td->prev->player);
                free(td->prev);
              }
            }
            destroy_player(end->player);
            free(end);
            free(moves);
            printf("error:reached MAX_SCENARIOS\n");
            return false;
          }
        }
        free(moves);
      }
      if (plays == 0) {
        printf("debug:destroy move_place %p\n", i);
        printf("debug:move_place count: %d\n", MoveCount);
        move_place *nxt = i->next;
        if (i->prev != NULL)
          i->prev->next = i->next;
        else
          start = i->next;
        if (i->next != NULL)
          i->next->prev = i->prev;
        else
          end = i->prev;
        if (i == oend) {
          printf("debug:nocont!\n");
          cont = false;
        }
        destroy_player(i->player);
        free(i);
        i = nxt;
        MoveCount--;
      } else {
        printf("debug:play move_place %p to %s\n", i,
               placeIdToAbbrev(firstPlay));
        player_move_to(
            i->player,
            resolve_loc_backwards(i->player,
                                  players[PLAYER_DRACULA]->all_location_history,
                                  last_known_round, firstPlay),
            firstPlay);
        printf("debug:i bef %p\n", i);
        i = i->next;
        printf("debug:i aft %p\n", i);
      }
    }
  }
  if (start == NULL) return false;
  if (MoveCount == 0) return false;
  move_place *n;
  for (; start != NULL; start = n) {
    n = start->next;
    probs[start->player->location]++;
    destroy_player(start->player);
    free(start);
  }
  return true;
}

static PlaceId sp_go_to(player_t *p, PlaceId dest, int round) {
  printf("debug:sp_go_to %s\n", placeIdToAbbrev(dest));
  if (p->location == dest) return dest;
  int n_locations = 0;
  int count = 1;
  PlaceId moves[NUM_REAL_PLACES];
  PlaceId loc[NUM_REAL_PLACES];
  Round rounds[NUM_REAL_PLACES];
  bool seen[NUM_REAL_PLACES];
  memset(seen, 0, sizeof(seen));
  moves[0] = NOWHERE;
  loc[0] = p->location;
  rounds[0] = round;
  seen[p->location] = true;

  int i;

  for (i = 0; loc[i] != dest; i++) {
    PlaceId *ds =
        main_gv_player_get_connections(p, &n_locations, loc[i], p->id, rounds[i], true,
                               true, true, false, false, false);
    for (int j = 0; j < n_locations; j++) {
      if (seen[ds[j]]) continue;
      seen[ds[j]] = true;
      if (moves[i] == NOWHERE)
        moves[count] = ds[j];
      else
        moves[count] = moves[i];
      loc[count] = ds[j];
      rounds[count] = rounds[i] + 1;
      count++;
    }
    free(ds);
  }
  return moves[i];
}

static inline double weighted_spdist(int spdist) {
  if (spdist == 1) return 1.9;
  if (spdist == 2) return 1.6;
  if (spdist == 3) return 1.3;
  return 1;
}

PlaceId decode_location_from_msg(const char *msg, Round round,
                                    enum player player) {
  int ret = 0;
  int i = 0;
  for (; msg[i] != '\0'; i += 3, ret++) {
    if (msg[i] == 'C' && msg[i + 1] == 'D') {
      i += 3;
      break;
    }
  }
  for (; msg[i] != '\0'; i += 3, ret += 10) {
    if (msg[i] == 'C' && msg[i + 1] == 'D') {
      i += 3;
      break;
    }
  }
  for (; msg[i] != '\0'; i += 3, ret += 100) continue;
  printf("decoded location - %s (%d/%d)\n",
         placeIdToAbbrev(ret - round - player), round, player);
  
  return ret - round - player;
}

void encode_msg_from_location(char *msg, PlaceId loc, Round round,
                              enum player player) {
  printf("encoding location %s (%d/%d)\n", placeIdToAbbrev(loc),
         round, player);
  int l = loc + round + player;
  int idx = 0;
  for (int i = l % 10; i > 0; i--) {
    PlaceId loc = randint(NUM_REAL_PLACES);
    while (loc == CASTLE_DRACULA) loc = randint(NUM_REAL_PLACES);
    str_ncpy(msg + idx, placeIdToAbbrev(loc), 3);
    msg[idx + 2] = ' ';
    idx += 3;
  }
  msg[idx++] = 'C';
  msg[idx++] = 'D';
  msg[idx++] = ' ';
  l /= 10;
  for (int i = l % 10; i > 0; i--) {
    PlaceId loc = randint(NUM_REAL_PLACES);
    while (loc == CASTLE_DRACULA) loc = randint(NUM_REAL_PLACES);
    str_ncpy(msg + idx, placeIdToAbbrev(loc), 3);
    msg[idx + 2] = ' ';
    idx += 3;
  }
  msg[idx++] = 'C';
  msg[idx++] = 'D';
  msg[idx++] = ' ';
  l /= 10;
  for (int i = l % 10; i > 0; i--) {
    PlaceId loc = randint(NUM_REAL_PLACES);
    while (loc == CASTLE_DRACULA) loc = randint(NUM_REAL_PLACES);
    str_ncpy(msg + idx, placeIdToAbbrev(loc), 3);
    msg[idx + 2] = ' ';
    idx += 3;
  }
}

void decideHunterMove(HunterView state) {
  gettimeofday(&start_time, NULL);
  unsigned int ts = (start_time.tv_usec % 5000) * (start_time.tv_sec % 5000);
  srand(ts);
  
  Round round = HvGetRound(state);
  PlaceId ret;
  enum player CurrentPlayer = HvGetPlayer(state);
  printf("PLAYER %d\n", CurrentPlayer);
  player_t *players[NUM_PLAYERS];
  char msg[100];
  memset(msg, 0, sizeof(msg));
  

  if (round == 0) {
    ret = randint(NUM_REAL_PLACES);
  } else {
    for (int i = 0; i < NUM_PLAYERS; i++) {
      players[i] = main_gv_get_player_class(state->gv, i);
    }

    bool guessDracula = getPossibleDraculaLocations(players, round);
    printf("getprob: %d\n", guessDracula);
    double maxprob = 0;
    int actionSpaceSize = 0;
    PlaceId maxprobl = NOWHERE;
    for (int i = MIN_REAL_PLACE; i <= MAX_REAL_PLACE; i++) {
      if (probs[i] > 0) {
        actionSpaceSize += probs[i];

        double prob = weighted_spdist(SPDIST[i][players[CurrentPlayer]->location]) *
                      probs[i];
      
        printf("%s: %d -> %lf\n", placeIdToAbbrev(i),
               probs[i], prob);
        if (prob > maxprob) {
          maxprob = prob;
          maxprobl = i;
        }
      }
    }
    printf("maxprob %lf actionSpace %d\n", maxprob,
           actionSpaceSize);

    printf("%s %s\n",
           placeIdToAbbrev(HvGetPlayerLocation(state, PLAYER_DRACULA)),
           placeIdToAbbrev(HvGetPlayerLocation(state, CurrentPlayer)));
    printf("health %d\n", HvGetHealth(state, CurrentPlayer));
    printf("%d\n", main_gv_get_health(state->gv, PLAYER_DRACULA));
    if (HvGetPlayerLocation(state, PLAYER_DRACULA) == HvGetPlayerLocation(state, CurrentPlayer) ||
        HvGetHealth(state, CurrentPlayer) <= 2 || (!guessDracula)) {
      if ((!guessDracula) && round < 6) {
        int num = 0;
        PlaceId *possible = HvGetDests(state, &num, true, true, true);
        ret = possible[randint(num)];
        free(possible);
      } else {
        ret = HvGetPlayerLocation(state, CurrentPlayer);
        if (ret == HOSPITAL_PLACE) {
          int num = 0;
          PlaceId *possible = HvGetDests(state, &num, true, true, true);
          ret = possible[randint(num)];
          free(possible);
        }
      }
    } else {
      ret = sp_go_to(players[CurrentPlayer], maxprobl, round);
      encode_msg_from_location(msg, maxprobl, round, CurrentPlayer);
    }
  }
  char name[3];
  str_ncpy(name, placeIdToAbbrev(ret), 3);
  if (msg[0] == '\0') encode_msg_from_location(msg, ret, round, CurrentPlayer);
  registerBestPlay(name, msg);
}

