#include "graphics.h"
#include "Nokia5110.h"
#include "tm4c123gh6pm.h"
#include "random.h"

#define		MAX_MISSILECOUNT	10
#define		ENEMYSHIPS_COUNT	4
#define		SMALL_ENEMY_ROW_GAP 20
#define		ENEMY_VERTICAL_GAP 10

unsigned long SwitchOn_Prev=0;
unsigned long SwitchOn_Curr=0;
unsigned char KeyPressCount=0;
unsigned char EnemyHordeLeftScreen=0;
unsigned char KillCount=0;
unsigned char PlayerLives=5;

unsigned char NumberOfEnemyShips=ENEMYSHIPS_COUNT;

typedef struct EnemyShip{
	
	unsigned char life;
	const unsigned char *image;
	unsigned long posx;
	unsigned long posy;
}EnemyShip;


typedef struct Fire{
	unsigned char life;
  const unsigned char *image;
	unsigned long posx;
	unsigned long posy;
}Fire;

typedef struct Player{
	long life;
	const unsigned char *image;
	unsigned long posx;
	unsigned long posy;
}Player;

EnemyShip EnemyRow[ENEMYSHIPS_COUNT]={{1,SmallEnemy10PointB,0,ENEMY_VERTICAL_GAP},
																			{1,SmallEnemy10PointB,0 + SMALL_ENEMY_ROW_GAP,ENEMY_VERTICAL_GAP},
																			{1,SmallEnemy10PointB,0 + 2*SMALL_ENEMY_ROW_GAP,ENEMY_VERTICAL_GAP},
																			{1,SmallEnemy10PointB,0 + 3*SMALL_ENEMY_ROW_GAP,ENEMY_VERTICAL_GAP}};

Fire PlayerMissile[MAX_MISSILECOUNT]={{0,Missile0,42,40    },
														{	0,Missile0,42,40	},
														{	0,Missile0,42,40	},
														{	0,Missile0,42,40	},
														{	0,Missile0,42,40	}};

Fire EnemyMissile[ENEMYSHIPS_COUNT] = {{0,Laser0,42,40  },
														{	0,Laser0,42,40	},
														{	0,Laser0,42,40	},
														{	0,Laser0,42,40	}};

Player PlayerShip={1,PlayerShip0,0,SCREENH};

//void Generate_Player(double x_player){
//	Player PlayerShip={1,PlayerShip0,x_player,SCREENH};
//}	

void Generate_Player_Missile(double x,double y,unsigned char count){
	
	unsigned char i = count-1;
		PlayerMissile[i].life = 1;
		PlayerMissile[i].posx = x;
		PlayerMissile[i].posy = y;	
}

void Generate_Enemey_Missile(){
	
	unsigned char i, RandomShip;
//	for(i=0;i<ENEMYSHIPS_COUNT;i++){
//		EnemyMissile[i].life = 1;
//		EnemyMissile[i].posx = EnemyRow[i].posx + ENEMY10W/2 - 1;
//		EnemyMissile[i].posy =EnemyRow[i].posy + ENEMY10H; 
//		
//	}
	
	RandomShip = Random32()%4 ;
 if(EnemyRow[RandomShip].life){
		EnemyMissile[RandomShip].life = 1;
		EnemyMissile[RandomShip].posx = EnemyRow[RandomShip].posx + ENEMY10W/2 - 1;	
		EnemyMissile[RandomShip].posy =EnemyRow[RandomShip].posy + ENEMY10H;
 }
}

//unsigned char Active_Enemy_Missiles(){
//	
//}
	
void Display_all(unsigned char count,double x_player){
	
	unsigned char i;
	Nokia5110_ClearBuffer();
	//EnemyReSpawn();
	
	PlayerShip.posx=x_player;
	
	//Add Enemy horde
	for(i=0;i<ENEMYSHIPS_COUNT;i++){		
		if(EnemyRow[i].life==1){			
			Nokia5110_PrintBMP(EnemyRow[i].posx,EnemyRow[i].posy, EnemyRow[i].image, 0);
			
		}		
	}
	//Add Enemy missiles to screen buffer
	for(i=0;i<ENEMYSHIPS_COUNT;i++){		
		if(EnemyMissile[i].life==1){			
			Nokia5110_PrintBMP(EnemyMissile[i].posx,EnemyMissile[i].posy, EnemyMissile[i].image, 0);}		
	}
	
	
	// Add all the active missiles to screen buffer
	for(i=0;i<count;i++){		
		if(PlayerMissile[i].life==1){			
			Nokia5110_PrintBMP(PlayerMissile[i].posx,PlayerMissile[i].posy, Missile0, 0);}		
	}
	// Add Playership to screen buffer
	if(PlayerShip.life){
		Nokia5110_PrintBMP(PlayerShip.posx,PlayerShip.posy, PlayerShip.image, 0);
		Nokia5110_DisplayBuffer();	
	}
}	

void Move_Missile(unsigned char *count){
	unsigned char i;
	for(i=0;i<*count;i++){
		PlayerMissile[i].posy --;
		
		if(PlayerMissile[i].posy<1){
			PlayerMissile[i].life = 0;
			//*count=*count-1;
		}
	}
	
	
	for(i=0;i<ENEMYSHIPS_COUNT;i++){
		EnemyMissile[i].posy ++;
		
		if(EnemyMissile[i].posy>47){
			EnemyMissile[i].life = 0;
			
			//*count=*count-1;
		}
	}	
}

void PlayerReSpawn(double x_player){
	
	if(PlayerLives>0){
		PlayerShip.life=1;		
	}
	
}

void EnemyReSpawn(){
	unsigned char i,ShipsKilled=0;
	
	if(EnemyHordeLeftScreen == 1){

		for(i=0;i<ENEMYSHIPS_COUNT;i++){
			EnemyRow[i].life=1;
			EnemyRow[i].image=SmallEnemy10PointB;
			EnemyRow[i].posx=i*SMALL_ENEMY_ROW_GAP;
			EnemyRow[i].posy=ENEMY_VERTICAL_GAP;			
		}
		EnemyHordeLeftScreen = 0;		
	}
	
	for(i=0;i<ENEMYSHIPS_COUNT;i++){
		if(!EnemyRow[i].life)
			ShipsKilled++;			
	}
	if(ShipsKilled>2){
		for(i=0;i<ENEMYSHIPS_COUNT;i++){
			if(!EnemyRow[i].life){
				EnemyRow[i].life=1;
				EnemyRow[i].image=SmallEnemy10PointB;
				EnemyRow[i].posx=i*SMALL_ENEMY_ROW_GAP;
				EnemyRow[i].posy=ENEMY_VERTICAL_GAP;
			}				
		}
		
	}
	
}

unsigned long SwitchPressEvent(){
		
	SwitchOn_Prev = SwitchOn_Curr;
		 
		if((GPIO_PORTE_DATA_R & 0x01) == 0x01){
			SwitchOn_Curr=1;
		}
		
		else{
			SwitchOn_Curr=0;		
		}
		if(SwitchOn_Curr!=SwitchOn_Prev)
			KeyPressCount++;
		
		if(KeyPressCount>2)
			KeyPressCount=1;
	
		if(KeyPressCount==2){
			KeyPressCount=0;
			return 1;
		}
		else
			return 0;
}
	

void EnemyHordeMove(unsigned char MoveSelect){
	unsigned char i;
	
	if (MoveSelect == 1){
		for(i=0;i<ENEMYSHIPS_COUNT;i++){		
			if(EnemyRow[i].life==1){			
				EnemyRow[i].posx=EnemyRow[i].posx+3;
				EnemyRow[i].posy++; 
			}		
		}
	}
	if (MoveSelect == 0){
		for(i=0;i<ENEMYSHIPS_COUNT;i++){		
			if(EnemyRow[i].life==1){			
				EnemyRow[i].posx=EnemyRow[i].posx-3;;
				EnemyRow[i].posy++; 
			}		
		}
	}
	
	for(i=0;i<ENEMYSHIPS_COUNT;i++){
				
		if(EnemyRow[i].posy>47){
			EnemyRow[i].life = 0;
			EnemyHordeLeftScreen=1;
			
			//*count=*count-1;
		}
	}
	
}


unsigned char CollisionCheck_EnemyMissile_Player(double x_player){
	unsigned char i,Separate = 0,PlayerExplosion = 0;
	
	for(i=0;i<ENEMYSHIPS_COUNT;i++){
		if(PlayerShip.life){
			if(EnemyMissile[i].life){
				if(((EnemyMissile[i].posx + MISSILEW)<x_player)||((PLAYERW+x_player)<(EnemyMissile[i].posx))
					||((EnemyMissile[i].posy)<(SCREENH-PLAYERH))||((SCREENH)<(EnemyMissile[i].posy-MISSILEH))){
					Separate = 1;					
					}
				else	{
					PlayerExplosion = 1;
					PlayerShip.life=0;
					PlayerLives--;
					EnemyMissile[i].life=0;
					return PlayerExplosion;
					}
			}				
		}	
	}
	if(Separate==1)
		return PlayerExplosion;
	else
		return PlayerExplosion;
	
}

unsigned char *CollisionCheck_PlayerMissile_Enemy(){
	
unsigned char i,j,Separate = 0;
static unsigned char EnemyExplosion[4] = {0,0,0,0};

for(j=0;j<ENEMYSHIPS_COUNT;j++){
		if(!EnemyRow[j].life){
			EnemyExplosion[j]=0;
		}
	}

	for(i=0;i<MAX_MISSILECOUNT;i++){
		if(PlayerMissile[i].life){
			for(j=0;j<ENEMYSHIPS_COUNT;j++){
				if(EnemyRow[j].life){
					if(((EnemyRow[j].posx + ENEMY10W)<PlayerMissile[i].posx)||((MISSILEW+PlayerMissile[i].posx)<(EnemyRow[j].posx))
					||((EnemyRow[j].posy)<(PlayerMissile[i].posy-MISSILEH))||((PlayerMissile[i].posy)<(EnemyRow[j].posy-ENEMY10H))){
						EnemyExplosion[j]=0;
						Separate = 1;
					
					}
					else{
						EnemyExplosion[j]=1;
						EnemyRow[j].life=0;
						PlayerMissile[i].life=0;
						KillCount++;
					}
					
				}
			}
		}
	}
	
	return EnemyExplosion;	
	
}

//unsigned char CheckForCollision(double x_player){
//	
//	
//	
//	
//}
//if(((EnemyMissile[i].posx + MISSILEW)<PlayerMissile[i].posx)||((PLAYERW+PlayerMissile[i].posx)<(EnemyMissile[i].posx))
//					||((EnemyMissile[i].posy)<(PlayerMissile[i].posy-MISSILEH))||((PlayerMissile[i].posy)<(EnemyMissile[i].posy-MISSILEH)))

void DisplayCollision(unsigned char PlayerExplosion, double x_player, unsigned char *EnemyExplosion){

	unsigned char i;
	if(PlayerExplosion){
		Nokia5110_PrintBMP(x_player,SCREENH, BigExplosion1, 0);
	  
	}
	for(i=0;i<ENEMYSHIPS_COUNT;i++){
		if((*(EnemyExplosion+i))==1){
			//EnemyRow[i].life=0;
			Nokia5110_PrintBMP(EnemyRow[i].posx,EnemyRow[i].posy, SmallExplosion0, 0);			
		}		
	}	
	Nokia5110_DisplayBuffer();		
	
}



void Delay100ms(unsigned long count){unsigned long volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}
	

