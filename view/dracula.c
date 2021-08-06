////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// dracula.c: your "Fury of Dracula" Dracula AI
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dracula.h"
#include "Mapdata.h"
#include "main_game_view.h"

#define NEXT_Q_GAMMA 0.3
#define DB_HIDE_WEIGHT 0.6
#define NORMAL_SEA_WEIGHT 0.4
#define CRASH_CHECK_THRESHOLD 5


typedef struct draculaView {
    MainGameView gv;
} draculaView;


static double action_q[NUM_REAL_PLACES], next_q[NUM_REAL_PLACES];

static inline int weighted_dist(int spdist) {
  if (spdist == 0) return -100;
  if (spdist == 1) return -16;
  if (spdist == 2) return -5;
  if (spdist == 3) return -1;
  if (spdist > 6) return 6;
  return spdist;
}

//only used as weighted_cddist(distToCD, health, hunterDist2CD_WeightedTotal)
static inline double weighted_cddist(int spdist, int health, int hDistToCP) {
  double weight = 1;
  // determine base weight based on drac_health
  if (health < 13)
    weight = 10;
  else if (health < 25)
    weight = 1;
  else if (health >= 60)
    weight = 0.1;
  else if (health >= 45)
    weight = 0.3;
  else if (health >= 38)
    weight = 0.7;

  // amplify weight based on hunterDist2CD_WeightedTotal
  if (hDistToCP <= -180)
    weight = 0;
  else if (hDistToCP < 0)
    weight = -1.2;  //  *= 0.01;
  else if (hDistToCP <= 12)
    weight = -0.8;  //  *= 0.6;
  else if (hDistToCP > 16)
    weight *= 1.1;
  else if (hDistToCP > 30)
    weight *= 2;

  // amplify weight based on cityDistToCD (distToCD) (spdist)
  if (spdist == 0) return 9 * weight;
  if (spdist == 1) return 3 * weight;
  if (spdist == 2) return 2 * weight;
  if (spdist == 3) return 1 * weight;
  return -(spdist - 3) * weight / 2;
}

static inline double apply_weight(double x, double weight) {
  if (x > 0) return (x * weight);
  if (weight <= 0) return (-x * weight);
  return (x / weight); // x < 0 || weight > 0
}

static inline void addQ(PlaceId loc, double action, double next) {
  action_q[loc] += action;
  next_q[loc] += next;
}

static inline void setQ(PlaceId loc, double action, double next) {
  action_q[loc] = action;
  next_q[loc] = next;
}

static inline void applyWeightQ(PlaceId loc, double action, double next) {
  action_q[loc] = apply_weight(action_q[loc], action);
  next_q[loc] = apply_weight(next_q[loc], next);
}

void decide_dracula_move(DraculaView dv) {
  Round round = DvGetRound(dv);
  PlaceId decidedMove = NOWHERE;
  size_t num = 0;
  PlaceId *possible;
  possible = DvGetValidMoves(dv, (int *)&num);
  bool canGo[NUM_REAL_PLACES];
  bool crashed[PLAYER_DRACULA];
  memset(action_q, 0, sizeof(action_q));
  memset(next_q, 0, sizeof(next_q));
  memset(canGo, 0, sizeof(canGo));
  memset(crashed, 0, sizeof(crashed));
  // loc2moveArray => stores all moves at location_index; rev => loc2moveArray
  PlaceId loc2moveArray[110];
  memset(loc2moveArray, -1, sizeof(loc2moveArray));
  player_t *dracPlayer = main_gv_get_player_class(dv->gv, PLAYER_DRACULA);
  int health = DvGetHealth(dv, PLAYER_DRACULA);
  // identify if a play's ai is crashed;
  // based on:
  //          if the player stay at one place for too long (>= 5 round and still not moving)
  for (int i = 0; i < PLAYER_DRACULA; i++) {
    crashed[i] = 
        (!main_gv_get_player_class(dv->gv, (Player)i)->neverdie) &&
        (main_gv_get_player_class(dv->gv, (Player)i)->staycount >= CRASH_CHECK_THRESHOLD);
    if (crashed[i]) printf("Player %d seems to have crashed", i);
  }
  // loop through all possible moves:
  //         num => number of possible moves
  // resolve => decode MOVE to LOCATION (for HIDE & DOUBLE BACK)
  for (size_t i = 0; i < num; i++) {
    if (possible[i] >= HIDE) {
      PlaceId resolve = player_resolve_move_location(dracPlayer, possible[i]);
      if (possible[i] == HIDE && canGo[resolve] &&
          loc2moveArray[resolve] >= DOUBLE_BACK_1)  // prefer DB to HIDE
        continue;
      // store location_id or move_id (>100) at location_index
      loc2moveArray[resolve] = possible[i];
      // then update possble_move into possible_location
      possible[i] = resolve;
    } else { // while no move2loc translation needed
      // save possible move to loc2moveArray[]
      loc2moveArray[possible[i]] = possible[i];
      // mark locations not HI nor DB as canGo
      canGo[possible[i]] = true;
    }
  }

  // evaluate the tendency of moving towards CD (to obtain health)
  // hunterDistanceSpecial based on following
  int hunterDist2CD_WeightedTotal = 0;
  for (int i = 0; i < PLAYER_DRACULA; i++) {
    // for each hunter, obtain corresponding info,
    // sum up to the 
    PlaceId hunterLocation = DvGetPlayerLocation(dv, i);
    if (crashed[i]) {
      // if crashed ai (not moving) identified
      if (hunterLocation == CASTLE_DRACULA)
        // if hunter stay at CD forever
        // += -100; highly refuse to go CD
        hunterDist2CD_WeightedTotal += weighted_dist(0);
      else
        // otherwise
        // += 6; highly recommend to go CD
        hunterDist2CD_WeightedTotal += weighted_dist(10);
      continue;
    }

    // use calculated SPROUD map to find shortest distance from
    // hunterLocation to CD; to calculate tendency of "moving to CD"
    int hunterDist2CD = NUM_REAL_PLACES;
    for (int j = 0; j < 4; j++) {
      // for different round number (train move ability);
      // find the shortest
      if (SPROUND[hunterLocation][CASTLE_DRACULA][j] < hunterDist2CD)
        hunterDist2CD = SPROUND[hunterLocation][CASTLE_DRACULA][j];
    }
    // sum up all hunter's weighted distance
    hunterDist2CD_WeightedTotal += weighted_dist(hunterDist2CD);
  }

  
  for (int i = MIN_REAL_PLACE; i <= MAX_REAL_PLACE; i++) {
    // if (!canGo[i]) continue;
    int minDist = NUM_REAL_PLACES; // 
    int distToCD = SPDIST[i][CASTLE_DRACULA]; // dracula's distance to CD
    for (int j = 0; j < PLAYER_DRACULA; j++) {
      PlaceId hunterLocation = DvGetPlayerLocation(dv, j);
      if (crashed[j]) { // if hunter afk
        if (hunterLocation == i) // for occupied city, set MOVE preference to minimal
          addQ(i, weighted_dist(0), weighted_dist(0)); // encounter crashed hunter
        else if (SPDIST[hunterLocation][i] == 1) // otherwise, set adjacent area MOVE to -1
          addQ(i, weighted_dist(3), weighted_dist(3)); // away from crashed hunter 
        continue;
      }
      // otherwise (not afk)
      // min_round_dist = dist_i2h + dist_CD2h - dist_i2CD;
      // intend to find i which:
      //        1. near to CD
      //        2. far from hunter
      //        3. obtain minDist for later use
      int dst = SPDIST[i][hunterLocation] + SPDIST[CASTLE_DRACULA][hunterLocation] - distToCD;
      if (dst < minDist) minDist = dst;
      int hunter_max_trail = (round + 1 + j) % 4;
      // sum up all weighted_dist from 4 hunters
      // set MOVE from hunterLocation to i:
      //        1. current round: not approaching hunter
      //        2. next round: not approaching area adjacent to hunter
      // difference from crashed hunter:
      //        Treat area_dist == 1 as good or not;
      addQ(i, weighted_dist(SPROUND[hunterLocation][i][hunter_max_trail]),
           weighted_dist((int)fmax(SPROUND[hunterLocation][i][hunter_max_trail] - 1, 0))); 
           // distance to hunter
    }

    // for special cases
    // if a place is too far from CD & close to hunter
    // re-weighted based on health, distToCD and hunterDist2CD_WeightedTotal
    if (minDist == 0) {
      addQ(i, apply_weight(weighted_cddist(distToCD, health, hunterDist2CD_WeightedTotal), 0.4),
           apply_weight(weighted_cddist(distToCD, health, hunterDist2CD_WeightedTotal), 0.4)); 
           // how good to go to CD (*0.4)
    } else if (minDist == 1) {
      addQ(i, apply_weight(weighted_cddist(distToCD, health, hunterDist2CD_WeightedTotal), 0.6),
           apply_weight(weighted_cddist(distToCD, health, hunterDist2CD_WeightedTotal), 0.6)); 
           // how good to go to CD (*0.6)
    } else if (minDist == 2) {
      addQ(i, apply_weight(weighted_cddist(distToCD, health, hunterDist2CD_WeightedTotal), 0.8),
           apply_weight(weighted_cddist(distToCD, health, hunterDist2CD_WeightedTotal), 0.8)); 
           // how good to go to CD (*0.8)
    } else {
      addQ(i, weighted_cddist(distToCD, health, hunterDist2CD_WeightedTotal),
           weighted_cddist(distToCD, health, hunterDist2CD_WeightedTotal)); 
           // how good to go to CD (*1.0)
    }
    // if a place is sea_type (which -2 health for drac)
    if (placeIdToType(i) == SEA) {
      if (action_q[i] < 0) { // if not favored already
        if (health <= 13)
          // little tendency of trading round with health, to get rid of hunter
          applyWeightQ(i, 1.0 / 0.9, 1.0 / 0.9);
        else if (health > 25)
          // moderate tendency of trading round with health, to get rid of hunter
          applyWeightQ(i, 1.0 / 0.1, 1.0 / 0.1);
        else
          // higher tendency of trading round with health, to get rid of hunter
          applyWeightQ(i, 1.0 / 0.35, 1.0 / 0.35);
        // action_q[i] -= 3;
      } else if (round % 13 == 0 || round % 13 == 12) {
        // prioritize vampire placement, try to leave sea
        applyWeightQ(i, 0.25, 0.25);
      } else if (health <= 10) {
        // prioritize survival, leave sea, punish sea move heavily
        applyWeightQ(i, -NORMAL_SEA_WEIGHT, -NORMAL_SEA_WEIGHT);
      } else {
        // have enough health to spare, underrate sea move normally
        applyWeightQ(i, NORMAL_SEA_WEIGHT, NORMAL_SEA_WEIGHT);
      }
      // :) the worst stratagy is to die on sea, punish sea move to maximum
      if (health <= 2) setQ(i, -100, -100);
    }
    if (canGo[i] && loc2moveArray[i] != i)  // DB or HIDE
      applyWeightQ(i, DB_HIDE_WEIGHT, 1); // underrate DB/HIDE
  }

  // for all reachable locations; do test:
  // further adjust weight based on future possible moves
  for (int i = MIN_REAL_PLACE; i <= MAX_REAL_PLACE; i++) {
    if (!canGo[i]) continue;// for all reachable locations
    player_t *tempPlayer = clone_player(dracPlayer);
    // simulate dracular move & evaluate result
    tempPlayer->all_history_size = -1;
    player_move_to(tempPlayer, i, loc2moveArray[i]);
    size_t tempNLocations = 0;
    PlaceId *tempLocationArr =
        main_gv_player_get_connections(tempPlayer, 
                                (int *)&tempNLocations, i, 
                                PLAYER_DRACULA, round + 1, true,
                                false, true, true, false, true);
    double bestNextMove = -10000;
    double bestNextHideDbMove = -10000;
    size_t nextLand = 0;
    size_t nextSea = 0;
    size_t nextHides = 0;
    size_t nextDbs = 0;
    // split MOVEs to catagories
    for (int j = 0; j < tempNLocations; j++) {
      if (tempLocationArr[j] == HIDE || tempLocationArr[j] == DOUBLE_BACK_1) {
        nextHides++;
      } else if (tempLocationArr[j] >= DOUBLE_BACK_2 && tempLocationArr[j] <= DOUBLE_BACK_5) {
        nextDbs++;
      } else if (tempLocationArr[j] != TELEPORT && placeIdToType(tempLocationArr[j]) == SEA) {
        nextSea++;
      } else {
        nextLand++;
      }
    }
    // traverse all next MOVEs and obtain key value:
    // bestNextHideDbMove && bestNextMove
    for (int j = 0; j < tempNLocations; j++) {
      PlaceId loc = tempLocationArr[j];
      double w8 = 1;
      // underrate skill MOve (DB & HI)
      if (loc >= HIDE && loc <= DOUBLE_BACK_5) {
        loc = player_resolve_move_location(tempPlayer, loc);
        w8 = DB_HIDE_WEIGHT;
        // in real play, TELEPORT only shows individually, 
        // which weight doesn't matter, as it will be the only legit MOVE
        if (next_q[CASTLE_DRACULA] < 0) {
          w8 = -1;  // don't teleport 
        }
      } else if (loc == TELEPORT) {
        // in real play, TELEPORT only shows individually, 
        // which weight doesn't matter, as it will be the only legit MOVE
        loc = CASTLE_DRACULA;
        if (i == CASTLE_DRACULA) w8 = -3;  // we don't want to be locked in at CD
      }
      // start analyzing next possible move
      // apply w8 on calculated weight, amplify reward | punishment
      w8 = apply_weight(next_q[loc], w8);
      printf("%s %lf\n", placeIdToAbbrev(loc), w8); // final loc rewards
      if (tempLocationArr[j] >= HIDE && tempLocationArr[j] <= DOUBLE_BACK_5) {
        if (w8 > bestNextHideDbMove) bestNextHideDbMove = w8;
      } else {
        if (w8 > bestNextMove) bestNextMove = w8;
      }
    }
    // punish the weight to next MOVE if normal MOVE not exist
    // e.g. if after moving to ZA, drac can only HI or DB, then ZA is a bad MOVE
    if (bestNextHideDbMove > bestNextMove) {
      if (nextLand == 0 && nextSea == 0) {
        if (nextDbs == 0 && next_q[CASTLE_DRACULA] < 0) {
          bestNextMove = apply_weight(bestNextHideDbMove, -2.5);
        } else { // HIDE || D1
          bestNextMove = bestNextHideDbMove;
        }
      }
    }
    // 
    printf("%s bestNextMove %lf\n", placeIdToAbbrev(i), bestNextMove);
    free(tempLocationArr);
    destroy_player(tempPlayer);
    addQ(i, NEXT_Q_GAMMA * (bestNextMove - action_q[i]), 0); // ????
    // further adjust weight based on types of future connections
    if (action_q[i] < 0) { // if not favored possitively
      addQ(i, 0.72 * nextLand, 0); //  0.75
      addQ(i, 0.35 * nextSea, 0); //   0.30
    } else if (nextLand >= 4) { // if next city connects to >=4 cities
      addQ(i, 0.1 * (nextLand - 4) + 1, 0); // 
    } else if (nextLand == 0) { // if only sea move possible
      applyWeightQ(i, 0.5, 0.5); //   
    }
  }

  // compare all weight of MOVEs and find the heightest weight MOVE
  double maxWeightedDist = -10000;
  for (int i = MIN_REAL_PLACE ; i <= MAX_REAL_PLACE; i++) {
    if (i == CASTLE_DRACULA && canGo[i] && action_q[i] < -5)
      applyWeightQ(i, 0.5, 1); // run away from CD when about to get caught
    if (canGo[i])
      printf("%s: %lf", placeIdToAbbrev(i), action_q[i]);
    if (canGo[i] && action_q[i] > maxWeightedDist) {
      maxWeightedDist = action_q[i];
      decidedMove = i;
    }
  }
  // if all MOVE has weight < -10000, then rand()
  if (decidedMove == NOWHERE) {
    printf("Debug: random MOVE: %s\n", placeIdToAbbrev(decidedMove));
    srand(0xD2AC); // DRAC => D2AC
    decidedMove = possible[rand() % num];
  }
  free(possible);
  // strncpy not working well in gcc 9.0
  // implement manual strncpy() limited ver.:
  // also, use 'decidedMove' to find correspoding
  // move in loc2moveArray[], including TP, HI, DB
  char name[3] = {placeIdToAbbrev(loc2moveArray[decidedMove])[0], 
                  placeIdToAbbrev(loc2moveArray[decidedMove])[1],
                  placeIdToAbbrev(loc2moveArray[decidedMove])[2]};
  registerBestPlay(name, "MUDA MUDA MUDA MUDA MUDA! WRYYYYYYYY!");
}
