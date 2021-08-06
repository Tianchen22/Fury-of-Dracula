////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// DraculaView.h: the DraculaView ADT
//
// You may augment this with as many of your own functions as you like,
// but do NOT remove or modify any of the existing function signatures,
// otherwise, your code will not be able to compile with our tests.
// 
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-11-30	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#ifndef FOD__DRACULA_VIEW_H_
#define FOD__DRACULA_VIEW_H_

#include <stdbool.h>

#include "Game.h"
#include "GameView.h"
#include "Places.h"


typedef struct draculaView *DraculaView;

///////////////////////////////////////////////////////////////////////
// Constructor/Destructor

/**
 * Creates a new view to summarise the current state of the game.
 *
 * @param pastPlays - A string of all the plays made in the game so far.
 *                    See the spec for details of the format. The string
 *                    can be assumed to be valid.
 *
 * @param messages - An  array containing a message for each play in the
 *                   game so far. It will have exactly the  same  number
 *                   of  elements as there are plays in `pastPlays`. The
 *                   message from the first play will be at index 0, and
 *                   so on. The contents of each message will be exactly
 *                   as provided by the player.
 *
 * The "Message" type is defined in Game.h.
 * You are free to ignore messages if you wish.
 */
DraculaView DvNew(char *pastPlays, Message messages[]);

/**
 * Frees all memory allocated for `dv`.
 * After this has been called, `dv` should not be accessed.
 */
void DvFree(DraculaView dv);

////////////////////////////////////////////////////////////////////////
// Game State Information

/**
 * Get the current round number
 */
Round DvGetRound(DraculaView dv);

/**
 * Gets the current game score; a positive integer between 0 and 366.
 */
int DvGetScore(DraculaView dv);

/**
 * Gets the current health points for the given player; an value between
 * 0 and 9 for a hunter, or >= 0 for Dracula.
 */
int DvGetHealth(DraculaView dv, Player player);

/**
 * Gets the current location of a given player.
 *
 * This function should return:
 * - NOWHERE if the given player has not had a turn yet.
 * - Otherwise,  a  PlaceId corresponding to a real location, as Dracula
 *   has full knowledge of all moves.
 */
PlaceId DvGetPlayerLocation(DraculaView dv, Player player);


/**
 * Gets the location of the sleeping immature vampire.
 *
 * This function should return:
 * - NOWHERE  if the immature vampire does not exist (i.e., if it hasn't
 *   been spawned, or if it has already matured or been vanquished).
 * - Otherwise,  a  PlaceId corresponding to a real location, as Dracula
 *   has full knowledge of all moves.
 */
PlaceId DvGetVampireLocation(DraculaView dv);

/**
 * Gets the locations of all active traps.
 *
 * This  function should return the locations in a dynamically allocated
 * array, and set *numTraps to the number of  traps.  The  array  should
 * contain  multiple  copies  of the same location if there are multiple
 * traps in that location. The locations can be in any order.
 */
PlaceId *DvGetTrapLocations(DraculaView dv, int *numTraps);


////////////////////////////////////////////////////////////////////////
// Making a Move

/**
 * Gets all the moves that Dracula can validly make this turn.
 *
 * This  function  should  return  the  moves in a dynamically allocated
 * array, and set *numReturnedMoves to the number of moves returned. The
 * array can be in any order but must contain unique entries.
 *
 * If  Dracula  has  no valid moves (other than TELEPORT), this function
 * should set *numReturnedMoves to 0 and return  an  empty  array  (NULL
 * will suffice).
 *
 * If  Dracula  hasn't  made  a move yet, set *numReturnedMoves to 0 and
 * return NULL.
 */
PlaceId *DvGetValidMoves(DraculaView dv, int *numReturnedMoves);

/**
 * Gets all the locations that Dracula can move to this turn.
 *
 * This  function should return the locations in a dynamically allocated
 * array, and set *numReturnedLocs to the number of locations  returned.
 * The array can be in any order but must contain unique entries.
 *
 * The  returned  locations should be consistent with Dracula's movement
 * rules (see the spec for details).  If  Dracula  has  no  valid  moves
 * (other than TELEPORT), this function should set *numReturnedLocs to 0
 * and return an empty array (NULL will suffice).
 *
 * If  Dracula  hasn't  made  a move yet, set *numReturnedMoves to 0 and
 * return NULL.
 */
PlaceId *DvWhereCanIGo(DraculaView dv, int *numReturnedLocs);

/**
 * Similar  to  DvWhereCanIGo, but the caller can restrict the transport
 * types to be considered. For example, if road is  true,  but  boat  is
 * false, boat connections will be ignored.
 */
PlaceId *DvWhereCanIGoByType(DraculaView dv, bool road, bool boat,
                             int *numReturnedLocs);

/**
 * Gets  all  the  locations  that the given player can move to in their
 * next move (for Dracula that is this turn).
 *
 * This  function  should return the locations in a dynamically alloated
 * array, and set *numReturnedLocs to the number of locations  returned. 
 * The  array can be in any order but must contain unique entries.
 *
 * If  the given player is a hunter, the function must take into account
 * how far the player can travel by rail in their next move.
 *
 * If  the  given  player  is  Dracula, the returned locations should be
 * consistent with the rules on Dracula's movement  (see  the  spec  for
 * details).
 *
 * If the given player hasn't made a move yet, set *numReturnedLocs to 0
 * and return NULL.
 */
PlaceId *DvWhereCanTheyGo(DraculaView dv, Player player,
                          int *numReturnedLocs);

/**
 * Similar to DvWhereCanTheyGo but the caller can restrict the transport
 * types  to  be considered. For example, if road and rail are true, but
 * boat is false, boat connections will be ignored.
 */
PlaceId *DvWhereCanTheyGoByType(DraculaView dv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs);							 

/**
 * Get the most recent move of a given player, returning the start and
 * end location of that move.
 *
 * Since Dracula moves last, the end location for other players is
 * always known, but the start location may be `UNKNOWN_LOCATION`
 * (for a hunter's first move).
 */
void DvGetPlayerMove  (
	DraculaView dv, Player player,
	PlaceId *start, PlaceId *end);

/**
 * Find out what minions I (Dracula) have placed at the specified
 * location -- minions are traps and immature vampires -- and returns
 * the counts in the variables referenced by `n_traps` and `n_vamps`.
 *
 * If `where` is not a place where minions can be left
 * (e.g. at sea, or NOWHERE), then set both counts to zero.
 */
void DvGetLocaleInfo (
	DraculaView dv, PlaceId where, int *n_traps, int *n_vamps);

/**
 * Fills the trail array with the locations of the last 6 turns for the
 * given player, including Dracula (if he asks about his own trail).
 * If the move does not exist yet (i.e., the start of the game),
 * the value should be UNKNOWN_LOCATION (-1).
 *
 * For example after 2 turns the array may have the contents
 *
 *     {29, 12, -1, -1, -1, -1}
 *
 * This would mean in the first move the player started on location 12
 * then moved to the current location of 29.
 *
 * If Dracula asks about his own trail, he should get real locations he
 * has previously been, not double-backs, etc.
 */
void DvGetTrail (
	DraculaView dv, Player player, PlaceId trail[TRAIL_SIZE]);

/**
 * Return an array of `PlaceId`s giving all locations that Dracula
 * could reach in his next move.
 *
 * The array can be in any order but must contain unique entries.
 * The array size is stored at the variable referenced by `n_locations`.
 * The current location should be included in the array.
 *
 * Road or sea connections should only be considered
 * if the `road` or `sea` parameters are true, respectively.
 *
 * The array should not include the hospital (where Dracula cannot go),
 * nor any locations only reachable by rail (which Dracula cannot use).
 * The set of possible locations must be consistent with the rules on
 * Dracula's movement (e.g. can't move to a location currently in his
 * trail).
 */
PlaceId *DvGetDests (DraculaView dv, int *n_locations, bool road,
                         bool sea);
/**
 * Return an array of `PlaceId`s giving all of the locations that the
 * given `player` could reach from their current location.
 *
 * The array can be in any order but must contain unique entries.
 * The array size is stored at the variable referenced by `n_locations`.
 * The player's current location should be included in the array.
 *
 * `road`, `rail`, and `sea` connections should only be considered
 * if the `road`, `rail`, `sea` parameters are true, respectively.
 *
 * The function must take into account the current round for determining
 * how far `player` can travel by rail.
 *
 * If `player` is Dracula, calls DvGetDests ().
 */
PlaceId *DvGetDestsPlayer (
	DraculaView dv, int *n_locations,
	Player player, bool road, bool rail, bool sea);


#endif // !defined(FOD__DRACULA_VIEW_H_)
