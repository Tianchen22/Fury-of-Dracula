////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// testDraculaView.c: test the DraculaView ADT
//
// As supplied, these are very simple tests.  You should write more!
// Don't forget to be rigorous and thorough while writing tests.
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-02	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DraculaView.h"
#include "Game.h"
#include "Places.h"
#include "testUtils.h"

int main(void)
{
	{///////////////////////////////////////////////////////////////////
	
		printf("Test for basic functions, "
			   "just before Dracula's first move\n");

		char *trail =
			"GST.... SAO.... HZU.... MBB....";
		
		Message messages[] = {
			"Hello", "Goodbye", "Stuff", "..."
		};
		
		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 0);
		assert(DvGetScore(dv) == GAME_START_SCORE);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == GAME_START_BLOOD_POINTS);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == STRASBOURG);
		assert(DvGetPlayerLocation(dv, PLAYER_DR_SEWARD) == ATLANTIC_OCEAN);
		assert(DvGetPlayerLocation(dv, PLAYER_VAN_HELSING) == ZURICH);
		assert(DvGetPlayerLocation(dv, PLAYER_MINA_HARKER) == BAY_OF_BISCAY);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == NOWHERE);
		assert(DvGetVampireLocation(dv) == NOWHERE);
		int numTraps = -1;
		PlaceId *traps = DvGetTrapLocations(dv, &numTraps);
		assert(numTraps == 0);
		free(traps);

		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("Test for encountering Dracula\n");

		char *trail =
			"GST.... SAO.... HCD.... MAO.... DGE.V.. "
			"GGEVD.. SAO.... HCD.... MAO....";
		
		Message messages[] = {
			"Hello", "Goodbye", "Stuff", "...", "Mwahahah",
			"Aha!", "", "", ""
		};
		
		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 1);
		assert(DvGetScore(dv) == GAME_START_SCORE - SCORE_LOSS_DRACULA_TURN);
		assert(DvGetHealth(dv, PLAYER_LORD_GODALMING) == 5);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == 30);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == GENEVA);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == GENEVA);
		assert(DvGetVampireLocation(dv) == NOWHERE);

		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("Test for Dracula leaving minions 1\n");

		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DED.V.. "
			"GST.... SST.... HST.... MST.... DMNT... "
			"GST.... SST.... HST.... MST.... DLOT... "
			"GST.... SST.... HST.... MST.... DHIT... "
			"GST.... SST.... HST.... MST....";
		
		Message messages[24] = {};
		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 4);
		assert(DvGetVampireLocation(dv) == EDINBURGH);
		int numTraps = -1;
		PlaceId *traps = DvGetTrapLocations(dv, &numTraps);
		assert(numTraps == 3);
		sortPlaces(traps, numTraps);
		assert(traps[0] == LONDON);
		assert(traps[1] == LONDON);
		assert(traps[2] == MANCHESTER);
		free(traps);
		
		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("Test for Dracula's valid moves 1\n");
		
		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DCD.V.. "
			"GGE.... SGE.... HGE.... MGE....";
		
		Message messages[9] = {};
		DraculaView dv = DvNew(trail, messages);
		
		int numMoves = -1;
		PlaceId *moves = DvGetValidMoves(dv, &numMoves);
		assert(numMoves == 4);
		sortPlaces(moves, numMoves);
		assert(moves[0] == GALATZ);
		assert(moves[1] == KLAUSENBURG);
		assert(moves[2] == HIDE);
		assert(moves[3] == DOUBLE_BACK_1);
		free(moves);
		
		printf("Test passed!\n");
		DvFree(dv);
	}
	


	{///////////////////////////////////////////////////////////////////
	
		printf("Test for DvWhereCanIGo 1\n");
		
		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DKL.V.. "
			"GGE.... SGE.... HGE.... MGE.... DD1T... "
			"GGE.... SGE.... HGE.... MGE.... DBCT... "
			"GGE.... SGE.... HGE.... MGE.... DHIT... "
			"GGE.... SGE.... HGE.... MGE....";
		
		Message messages[24] = {};
		DraculaView dv = DvNew(trail, messages);
		
		int numLocs = -1;
		PlaceId *locs = DvWhereCanIGo(dv, &numLocs);
		assert(numLocs == 4);
		sortPlaces(locs, numLocs);
		assert(locs[0] == BELGRADE);
		assert(locs[1] == CONSTANTA);
		assert(locs[2] == GALATZ);
		assert(locs[3] == SOFIA);
		free(locs);
		
		printf("Test passed!\n");
		DvFree(dv);
	}
	
	{///////////////////////////////////////////////////////////////////
	
		printf("Checking where is and lastmove for players sent to the hospital and for dracula hiding\n");

		char *trail =
			"GLS.... SMR.... HCD.... MAM.... DSN.V.. "
        	"GMA.... SCF.... HGA.... MCO.... DSRT... "
        	"GSNV... SMR.... HCD.... MAM.... DMAT... "
        	"GSRT... SCF.... HGA.... MBU.... DHIT... "
        	"GMATTD. SCF.... HGA.... MBU.... DD1T... "
        	"GZA.... SCF.... HGA....";
		
		Message messages[28] = {};
		
		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 5);
		assert(DvGetScore(dv) == GAME_START_SCORE - 5 - 6);

		assert(DvGetHealth(dv, PLAYER_LORD_GODALMING) == 9);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == 30);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == ZAGREB);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == MADRID);
		assert(DvGetVampireLocation(dv) == NOWHERE);

		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("Checking where is and lastmove for players sent to the hospital and for dracula hiding\n");

		char *trail =
			"GLS.... SMR.... HCD.... MAM.... DSN.V.. "
        	"GMA.... SCF.... HGA.... MCO.... DSRT... "
        	"GSNV... SMR.... HCD.... MAM.... DMAT... "
        	"GSRT... SCF.... HGA.... MBU.... DHIT... "
        	"GMATTD. SCF.... HGA.... MBU.... DD1T...";
		
		Message messages[25] = {};
		
		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 5);
		assert(DvGetScore(dv) == GAME_START_SCORE - 5 - 6);

		assert(DvGetHealth(dv, PLAYER_LORD_GODALMING) == 0);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == 30);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == ST_JOSEPH_AND_ST_MARY);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == MADRID);
		assert(DvGetVampireLocation(dv) == NOWHERE);

		printf("Test passed!\n");
		DvFree(dv);
	}


	{///////////////////////////////////////////////////////////////////
	
		printf("Test for more rounds\n");

		char *trail =
			"GST.... SAO.... HCD.... MAO.... DMA.V.. " // 0
			"GST.... SST.... HST.... MST.... DTS.... " // sea
			"GST.... SST.... HST.... MST.... DLOT... "
			"GST.... SST.... HST.... MST.... DSJT... "
			"GST.... SST.... HST.... MST.... DHIT... "
			"GST.... SST.... HST.... MST.... DNUT... "
			"GST.... SAO.... HCD.... MAO.... DLET.V. " //
			"GST.... SAO.... HCD.... MAO.... DROT... "
			"GST.... SAO.... HCD.... MAO.... DJMT.M. "
			"GST.... SAO.... HCD.... MAO.... DMIT.M. "
			"GST.... SAO.... HCD.... MAO.... DPAT.M. "
			"GST.... SAO.... HCD.... MAO.... DD1T.M. "
			"GST.... SAO.... HCD.... MAO.... DTS..M. " // sea
			"GST.... SAO.... HCD.... MAO.... DNP.VM. " // 13
			"GST.... SAO.... HCD.... MAO.... DALT.M. "
			"GST.... SAO.... HCD.... MAO.... DATT.M. "
			"GST.... SAO.... HCD.... MAO.... DBAT.M. "
			"GST.... SAO.... HCD.... MST.... DAO..M. " // sea
			"GST.... SAO.... HCD.... MAO.... DBAT... "
			"GST.... SAO.... HCD.... MAO.... DBIT.V. " //
			"GST.... SAO.... HCD.... MAO.... DBB..M. " // sea
			"GST.... SAO.... HCD.... MAO.... DBET.M. "
			"GST.... SAO.... HCD.... MAO.... DBRT.M. "
			"GST.... SAO.... HCD.... MAO.... DBS.... " // sea
			"GST.... SAO.... HCD.... MAO.... DBOT.M. "
			"GST.... SAO.... HCD.... MAO.... DBUT.M. "
			"GST.... SAO.... HCD.... MAO.... DBC.V.. " // 26
			"GST.... SAO.... HCD.... MAO.... DBDT.M. "
			"GST.... SAO.... HCD.... MAO.... DCAT.M. "
			"GST.... SAO.... HCD.... MAO.... DCGT... "
			"GST.... SAO.... HCD.... MAO.... DCDT.M. "
			"GST.... SAO.... HCDTD.. MAO.... DCFT.M. "
			"GST.... SAO.... HCD.... MAO.... DCOT.V. " //
			"GST.... SAO.... HCD.... MAO.... DCNT.M. "
			"GST.... SAO.... HCD.... MAO.... DDUT.M. "
			"GST.... SAO.... HCD.... MAO.... DEDT.M. "
			"GST.... SAO.... HCD.... MAO.... DEC..M. " // sea
			"GST.... SAO.... HCD.... MAO.... DFLT.M. "
			"GST.... SAO.... HCD.... MAO.... DFRT.M. "
			"GST.... SAO.... HCD.... MAO.... DGA.VM. " // 39
			"GST.... SAO.... HCD.... MAO.... DGWT.M. "
			"GST.... SAO.... HCD.... MAO.... DGET.M. "
			"GST.... SAO.... HCD.... MAO.... DGOT... "
			"GST.... SAO.... HCD.... MAO.... DGRT.M. "
			"GST.... SAO.... HCD.... MAO.... DHAT.M. "
			"GST.... SAO.... HCD.... MAO.... DKLT.V. " //
			"GST.... SAO.... HCD.... MAO.... DMNT.M. "
			"GST.... SAO.... HCD.... MAO.... DPRT.M. "
			"GST.... SAO.... HCD.... MAO.... DSNT.M. "
			"GST.... SAO.... HCD.... MAO.... DZUT.M. "
			"GST.... SAO.... HCD.... MAO.... DZAT.M.";

			
		
		Message messages[255] = {};
		
		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 51);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == GAME_START_BLOOD_POINTS 
		                                          + 1 * LIFE_GAIN_CASTLE_DRACULA
                                                  - 6 * LIFE_LOSS_SEA
                                                  - 1 * LIFE_LOSS_HUNTER_ENCOUNTER);
		assert(DvGetScore(dv) == GAME_START_SCORE - 103);
		assert(DvGetVampireLocation(dv) == NOWHERE);
		// trap test
		int numTraps = -1;
		PlaceId *traps = DvGetTrapLocations(dv, &numTraps);
		assert(numTraps == 6);
		sortPlaces(traps, numTraps);
		assert(traps[0] == KLAUSENBURG); // KL
		assert(traps[1] == MANCHESTER);  // MN
		assert(traps[2] == PRAGUE);      // PR
		assert(traps[3] == SANTANDER);   // SN
		assert(traps[4] == ZAGREB);      // ZA
		assert(traps[5] == ZURICH);      // ZU
		free(traps);

		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("Test for encountering Dracula--more rounds and Dracule hid in the last round\n");

		char *trail =
			"GST.... SAO.... HCD.... MAO.... DMA.V.. "
			"GST.... SST.... HST.... MST.... DMNT... "
			"GST.... SST.... HST.... MST.... DLOT... "
			"GST.... SST.... HST.... MST.... DGET... "
			"GST.... SST.... HST.... MST.... DHIT... "
			"GGEVD.. SAO.... HCD.... MAO....";
		
		Message messages[29] = {};
		
		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 5);
		assert(DvGetScore(dv) == GAME_START_SCORE - 5);
		assert(DvGetHealth(dv, PLAYER_LORD_GODALMING) == 5);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == 30);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == GENEVA);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == GENEVA);
		assert(DvGetVampireLocation(dv) == NOWHERE);

		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("Castle Dracular test, heals in castle\n");

		char *trail =
			"GED.... SGE.... HZU.... MCA.... DCF.V.. "  // 0
      		"GMN.... SCFVD.. HGE.... MLS.... DBOT... "  // 1
      		"GLO.... SMR.... HCF.... MMA.... DTOT... "  // 2
      		"GPL.... SMS.... HMR.... MGR.... DBAT... "  // 3
      		"GEC.... SBATD.. HGO.... MAL.... DMS.... "  // 4  -2
      		"GLE.... SZA.... HTS.... MMS.... DTS.... "  // 5  -2
      		"GLE.... SZA.... HTS.... MMS.... DIO.... "  // 6  -2
      		"GLE.... SZA.... HTS.... MMS.... DBS..M. "  // 7  -2
      		"GLE.... SZA.... HTS.... MMS.... DCNT.M. "  // 8
      		"GLE.... SZA.... HTS.... MMS.... DGAT.M. "  // 8
      		"GLE.... SZA.... HTS.... MMS.... DCDT...";  // 10
		
		Message messages[55] = {};
		
		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 11);
		assert(DvGetScore(dv) == GAME_START_SCORE - 17);
		assert(DvGetHealth(dv, PLAYER_LORD_GODALMING) == 9);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == 40 -20 -8 + 10);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == LE_HAVRE);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == CASTLE_DRACULA);
		assert(DvGetVampireLocation(dv) == NOWHERE);

		printf("Test passed!\n");
		DvFree(dv);
	}


	{///////////////////////////////////////////////////////////////////
	
		printf("Dracula move test, cannot move to hospital\n");

		char *trail =
      		"GED.... SGE.... HZU.... MCA.... DZA.V.. "
      		"GED.... SGE.... HZU.... MCA.... DSJT... "
      		"GED.... SGE.... HZU.... MCA....";
		
		Message messages[14] = {};
		
		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 2);
		assert(DvGetScore(dv) == GAME_START_SCORE - 2);
		assert(DvGetHealth(dv, PLAYER_LORD_GODALMING) == 9);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == 40);
		assert(!(DvGetPlayerLocation(dv, PLAYER_DRACULA) == ST_JOSEPH_AND_ST_MARY));

		printf("Test passed!\n");
		DvFree(dv);
	}

	return EXIT_SUCCESS;
}
