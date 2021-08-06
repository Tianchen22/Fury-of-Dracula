CC	 = gcc
CFLAGS = -Wall -Werror
LDFLAGS = -lm 
BINS = testGameView testHunterView testDraculaView testMap dracula hunter

OBJS = GameView.o Map.o Places.o Mapdata.o main_game_view.o Queue.o Player.o

LIBS =

all: $(BINS)

all: ${BINS}
Mapdata: Mapdata.c Mapdata.h

hunter: playerHunter.o hunter.o HunterView.o $(OBJS) $(LIBS)
dracula: playerDracula.o dracula.o DraculaView.o $(OBJS) $(LIBS)

playerDracula.o: main_player.c dracula.h Game.h DraculaView.h GameView.h Places.h
	$(CC) $(CFLAGS) -DI_AM_DRACULA -c $< -o $@

playerHunter.o: main_player.c hunter.h Game.h HunterView.h GameView.h Places.h
	$(CC) $(CFLAGS) -c $< -o $@


testGameView:		testGameView.o testUtils.o GameView.o main_game_view.o Map.o Mapdata.o Places.o Player.o Queue.o 
testGameView.o:	testGameView.c GameView.h main_game_view.h Map.h Mapdata.h Places.h Game.h Player.h Queue.h

testHunterView:	testHunterView.o HunterView.o testUtils.o GameView.o main_game_view.o Map.o Mapdata.o Places.o Player.o Queue.o 
testHunterView.o:	testHunterView.c HunterView.h GameView.h main_game_view.h Map.h Mapdata.h Places.h Game.h Player.h Queue.h

testDraculaView:	testDraculaView.o DraculaView.o testUtils.o GameView.o main_game_view.o Map.o Mapdata.o Places.o Player.o Queue.o 
testDraculaView.o:	testDraculaView.c DraculaView.h GameView.h main_game_view.h Map.h Mapdata.h Places.h Game.h Player.h Queue.h

testMap: testMap.o Map.o Places.o Mapdata.o
testMap.o: testMap.c Map.c Map.h  Places.h Mapdata.h

Places.o:	Places.c Places.h Game.h
Player.o:	Player.c Places.h Game.h Queue.h
Map.o:          Map.c Map.h Mapdata.h Places.h
Mapdata.o:      Mapdata.c Map.h Mapdata.h Places.h
GameView.o:	GameView.c GameView.h main_game_view.c main_game_view.h Game.h Player.h Queue.h  Mapdata.h
main_game_view.o:	main_game_view.c main_game_view.h Game.h Player.h Queue.h  
HunterView.o:	HunterView.c HunterView.h GameView.h main_game_view.h Game.h  Mapdata.h
DraculaView.o:	DraculaView.c DraculaView.h GameView.h main_game_view.h Game.h  Mapdata.h
Queue.o: Queue.c Queue.h Places.h 
testUtils.o: testUtils.c Places.h Game.h



.PHONY: clean
clean:
	-rm -f ${BINS} *.o core Mapdata.*

