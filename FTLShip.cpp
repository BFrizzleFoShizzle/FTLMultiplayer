#include "FTLShip.h"

ship* getPlayerShip(void) {
	return playerShip;
};

ship* getEnemyShip(void) {
	//2nd element of ships is current enemy ship
	return ships+1;
};
