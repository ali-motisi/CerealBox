/*
 * CerealBox - Copyright 2015 Ali Motisi. All rights reserved.
 */

#include "../../../../common/src/oogame.h"

#define oomax(a,b) ((a) > (b) ? (a) : (b))
#define oomin(a,b) ((a) < (b) ? (a) : (b))

#define CombatGame_ScoreLimit 10

oouint _playerWidth = 40;
oouint _playerHeight = 40;
oouint _playerTurretWidth = 10;
oouint _playerTurretHeight = 10;
oouint _missileWidth = 5;
oouint _missileHeight = 2;

oobool _moveWallUp = ootrue;

oouint _wallColour = 0xffFFCC99;


typedef enum {
    PlayerDirection_North=0,
    PlayerDirection_NorthEast,
    PlayerDirection_East,
    PlayerDirection_SouthEast,
    PlayerDirection_South,
    PlayerDirection_SouthWest,
    PlayerDirection_West,
    PlayerDirection_NorthWest
}PlayerDirection;

typedef struct{
    oofloat x;
    oofloat y;
    PlayerDirection directionFired;
    oobool isFiring;
    
}cbMissile;

typedef struct {
    oofloat x;
    oofloat y;
    oofloat lastValidX;
    oofloat lastValidY;
    oofloat spawnPointX;
    oofloat spawnPointY;
    PlayerDirection direction;
    ooint score;
    oofloat timeOfLastKeyPressed;
    ooint playerIndex;
    cbMissile missile;
    oofloat timeOfDeath;
    oobool alive;
} ooplayer;

typedef struct {
    oofloat x;
    oofloat y;
    oofloat width;
    oofloat height;
    oouint colour;
}arenaWall;

typedef struct {
    arenaWall leftWall;
}gameArena;

typedef struct {
    oobool initialised;
    
    OOGameInput*  input;
    OOGameOutput* output;
    
    oofloat time;
    oofloat tSine;
    ooplayer p1;
    ooplayer p2;
    gameArena arena;
    
    oofloat moveDelta;
} cbgame;


void initGame(cbgame* game) {
    game->initialised = ootrue;
    game->p1.x = 100.f;
    game->p1.y = 100.f;
    game->p1.spawnPointX = 100.f;
    game->p1.spawnPointY = 100.f;
    
    game->p2.x = 800.f;
    game->p2.y = 100.f;
    game->p2.spawnPointX = 800.f;
    game->p2.spawnPointY = 100.f;
    game->p1.playerIndex = 1;
    game->p2.playerIndex = 2;
    
    game->p1.lastValidX = 100.f;
    game->p1.lastValidY = 100.f;
    
    game->p2.lastValidX = 800.f;
    game->p2.lastValidY = 100.f;
    
    game->p1.timeOfDeath = 0.f;
    game->p2.timeOfDeath = 0.f;
    
    game->p1.score = 0;
    game->p2.score = 0;
    
    game->p1.alive = ootrue;
    game->p2.alive = ootrue;
    
    game->arena.leftWall.x = game->output->graphic.width/2;
    game->arena.leftWall.y = 200;
    game->arena.leftWall.width = 20;
    game->arena.leftWall.height = 200;
    game->arena.leftWall.colour = _wallColour;
    
}



void clearScreen(cbgame* game) {
    memset(game->output->graphic.buffer, 0, game->output->graphic.width*game->output->graphic.height*sizeof(oouint));
}

oofloat distanceBetweenTanks(cbgame* g) {
    oofloat dx = g->p1.x - g->p2.x;
    oofloat dy = g->p1.y - g->p2.y;
    return sqrtf(dx*dx+dy*dy);
}

oofloat distanceBetweenMissileAndTank(cbgame *game,ooplayer *missile, ooplayer *tank)
{
    oofloat dx = missile->missile.x - tank->x;
    oofloat dy = missile->missile.y - tank->y;
    
    return sqrtf(dx*dx+dy*dy);
}

void drawRectangle(cbgame* game, ooint x, ooint y, oouint width, oouint height, oouint colour) {
    ooint x2 = x+width-1;
    ooint y2 = y+height-1;
    
    if( x2<0 || y2<0 || x>=game->output->graphic.width || y>=game->output->graphic.height ) {
        // clip
        return;
    }
    
    ooint minX = oomax(0, x);
    ooint maxX = oomin(game->output->graphic.width-1, x2);
    
    ooint minY = oomax(0, y);
    ooint maxY = oomin(game->output->graphic.height-1, y2);
    
    
    for(y=minY; y<=maxY; y++) {
        for(x=minX; x<=maxX; x++) {
            game->output->graphic.buffer[y*game->output->graphic.width+x] = colour;
        }
    }
}

void drawArena(cbgame *game)
{
    //Center Wall
    arenaWall *wall = &game->arena.leftWall;
    drawRectangle(game, wall->x, wall->y, wall->width, wall->height, wall->colour);
}

void respawnPlayer(cbgame *game, ooplayer *player)
{
    player->x = player->spawnPointX;
    player->y = player->spawnPointY;
    player->alive = ootrue;
}

void fireWeapon(cbgame* game, ooplayer *player)
{
    
    switch (player->direction) {
        case PlayerDirection_North:
            player->missile.x = player->x+_playerWidth/2;
            player->missile.y = player->y-_playerTurretHeight;
            break;
        case PlayerDirection_NorthEast:
            player->missile.x = player->x+_playerWidth+_playerTurretWidth/5;
            player->missile.y = player->y-_playerTurretHeight;
            break;
        case PlayerDirection_East:
            player->missile.x = player->x+_playerWidth+_playerTurretWidth/5;
            player->missile.y = player->y+_playerHeight/2;
            break;
        case PlayerDirection_SouthEast:
            player->missile.x = player->x+_playerWidth+_playerTurretWidth/5;
            player->missile.y = player->y+_playerTurretHeight/6+_playerHeight;
            break;
        case PlayerDirection_South:
            player->missile.x = player->x+(_playerWidth/2 - _playerTurretWidth/2);
            player->missile.y = player->y+_playerTurretHeight/6+_playerHeight;
            break;
        case PlayerDirection_SouthWest:
            player->missile.x = player->x-_playerTurretWidth;
            player->missile.y = player->y+_playerTurretHeight/6+_playerHeight;
            break;
        case PlayerDirection_West:
            player->missile.x = player->x-_playerTurretWidth;
            player->missile.y = player->y+(_playerHeight/2 - _playerTurretHeight/2);
            break;
        case PlayerDirection_NorthWest:
            player->missile.x = player->x-_playerTurretWidth;
            player->missile.y = player->y-_playerTurretHeight;
            break;
        default:
            break;
    }
    
    
    
    drawRectangle(game, player->missile.x, player->missile.y, _missileWidth, _missileHeight, 0xffffff00);
    
    player->missile.isFiring = ootrue;
    player->missile.directionFired = player->direction;
}



oobool playerInBounds(cbgame *game,oofloat x, oofloat y, ooint playerIndex)
{
    oobool playerXInBounds = oofalse;
    oobool playerYInbounds = oofalse;
    oofloat area = _playerTurretWidth + 2;
    oofloat yArea = y;
    oofloat xArea = x;
    
    
    if(distanceBetweenTanks(game) < _playerWidth)
    {
        if(playerIndex == 1)
        {
            game->p1.x = game->p1.lastValidX;
            game->p1.y = game->p1.lastValidY;
        }
        else if (playerIndex ==2)
        {
            game->p2.x = game->p2.lastValidX;
            game->p2.y = game->p2.lastValidY;
            
        }
        
    }
    
    
    if(x != -1)
    {
        if(x-area >= 0 && x+area < game->output->graphic.width)
        {
            playerXInBounds = ootrue;
            if(playerIndex ==1)
            {
                game->p1.lastValidX = game->p1.x;
            }
            else if (playerIndex == 2)
            {
                game->p2.lastValidX = game->p2.x;
                
            }
        }
        
    }
    if (y != -1)
    {
        if(y - area*4 >= 0 && y +area < game->output->graphic.height)
        {
            playerYInbounds = ootrue;
            if(playerIndex ==1)
            {
                game->p1.lastValidY = game->p1.y;
            }
            else if (playerIndex == 2)
            {
                game->p2.lastValidY = game->p2.y;
                
            }
        }
        
    }
    
    if(x==-1)
    {
        playerXInBounds = ootrue;
    }
    if(y == -1)
    {
        playerYInbounds = ootrue;
    }
    
    if(xArea + _playerTurretHeight >= game->arena.leftWall.x && xArea <= game->arena.leftWall.x + game->arena.leftWall.width && yArea + _playerTurretHeight >= game->arena.leftWall.y && yArea +_playerTurretHeight <= game->arena.leftWall.y +game->arena.leftWall.height)
    {
        return oofalse;
    }
    
    return playerXInBounds && playerYInbounds;
}
void drawMissile(cbgame*game, ooplayer *player)
{
    oofloat missileSpeed = game->input->dt * 500;
    
    switch (player->missile.directionFired) {
        case PlayerDirection_North:
            player->missile.y -= missileSpeed;
            player->missile.x = player->x+_playerWidth/2;
            break;
        case PlayerDirection_NorthEast:
            player->missile.x += missileSpeed;
            player->missile.y -= missileSpeed;
            break;
        case PlayerDirection_East:
            player->missile.x += missileSpeed;
            player->missile.y = player->y+_playerHeight/2;
            break;
        case PlayerDirection_SouthEast:
            player->missile.x += missileSpeed;
            player->missile.y += missileSpeed;
            break;
        case PlayerDirection_South:
            player->missile.y += missileSpeed;
            player->missile.x = player->x+(_playerWidth/2 - _playerTurretWidth/2);
            break;
        case PlayerDirection_SouthWest:
            player->missile.x -= missileSpeed;
            player->missile.y += missileSpeed;
            break;
        case PlayerDirection_West:
            player->missile.x -= missileSpeed;
            player->missile.y = player->y+(_playerHeight/2 - _playerTurretHeight/2);
            
            break;
        case PlayerDirection_NorthWest:
            player->missile.x -= missileSpeed;
            player->missile.y -= missileSpeed;
            break;
        default:
            break;
            
    }
    
    
    
    if(!playerInBounds(game, player->missile.x, player->missile.y,player->playerIndex))
    {
        player->missile.isFiring = oofalse;
    }
    
    drawRectangle(game, player->missile.x, player->missile.y, _missileWidth, _missileHeight, 0xffffff00);
    
    ooplayer *tank = NULL;
    
    if(player->playerIndex == 1)
    {
        tank = &game->p2;
    }
    else if (player->playerIndex == 2)
    {
        tank = &game->p1;
    }
    
    if(distanceBetweenMissileAndTank(game, player, tank) < _playerWidth/1.05 && tank->alive)
    {
        tank->alive = oofalse;
        tank->timeOfDeath = game->time;
        player->score++;
    }
    
}
void drawPlayer(cbgame* game, ooplayer* player) {
    oouint colour = &game->p1==player ? 0xffff0000 : 0xff00ff00;
    drawRectangle(game, (ooint) player->x, (ooint)player->y, _playerWidth, _playerHeight, colour);
    
    
    //Draw the turrets
    switch (player->direction) {
        case PlayerDirection_North:
            drawRectangle(game, player->x+(_playerWidth/2 - _playerTurretWidth/2),player->y-_playerTurretHeight,_playerTurretWidth , _playerTurretHeight, 0xff0066ff);
            break;
        case PlayerDirection_NorthEast:
            drawRectangle(game, player->x+_playerWidth+_playerTurretWidth/5, player->y-_playerTurretHeight, _playerTurretWidth, _playerTurretHeight, 0xff0066ff);
            break;
        case PlayerDirection_East:
            drawRectangle(game, player->x+_playerWidth+_playerTurretWidth/5, player->y+(_playerHeight/2 - _playerTurretHeight/2),_playerTurretWidth , _playerTurretHeight, 0xff0066ff);
            break;
        case PlayerDirection_SouthEast:
            drawRectangle(game, player->x+_playerWidth+_playerTurretWidth/5, player->y+_playerTurretHeight/6+_playerHeight, _playerTurretWidth, _playerTurretHeight, 0xff0066ff);
            break;
        case PlayerDirection_South:
            drawRectangle(game, player->x+(_playerWidth/2 - _playerTurretWidth/2),player->y+_playerTurretHeight/6+_playerHeight,_playerTurretWidth , _playerTurretHeight, 0xff0066ff);
            break;
        case PlayerDirection_SouthWest:
            drawRectangle(game, player->x-_playerTurretWidth,player->y+_playerTurretHeight/6+_playerHeight,_playerTurretWidth , _playerTurretHeight, 0xff0066ff);
            break;
        case PlayerDirection_West:
            drawRectangle(game, player->x-_playerTurretWidth, player->y+(_playerHeight/2 - _playerTurretHeight/2),_playerTurretWidth , _playerTurretHeight, 0xff0066ff);
            break;
        case PlayerDirection_NorthWest:
            drawRectangle(game, player->x-_playerTurretWidth, player->y-_playerTurretHeight,_playerTurretWidth , _playerTurretHeight, 0xff0066ff);
            break;
        default:
            
            break;
    }
}

void updateGame(cbgame * game) {
    
    oofloat player1Speed = game->input->dt * 100.f;
    oofloat player2Speed = game->input->dt * 100.f;
    oofloat timeThreshold = game->input->dt * 10.f;
    
    ooint p1Direction = game->p1.direction;
    ooint p2Direction = game->p2.direction;
    //Get the direction
    if(game->time > game->p1.timeOfLastKeyPressed + timeThreshold && game->p1.alive)
    {
        if( game->input->keyboard['D'] || game->input->keyboard['d'] ) {
            
            p1Direction++;
            game->p1.timeOfLastKeyPressed = game->time;
        }
        if( game->input->keyboard['A'] || game->input->keyboard['a'] ) {
            
            p1Direction--;
            game->p1.timeOfLastKeyPressed = game->time;
        }
        if(game->input->keyboard['F'] || game->input->keyboard['f'])
        {
            player1Speed *=3;
        }
        else if(game->input->keyboard['G'] || game->input->keyboard['g'])
        {
            if(!game->p1.missile.isFiring)
            {
                fireWeapon(game,&game->p1);
            }
        }
    }
    if(game->time > game->p2.timeOfLastKeyPressed + timeThreshold && game->p2.alive)
    {
        if( game->input->keyboard['L'] || game->input->keyboard['l'] ) {
            
            p2Direction++;
            game->p2.timeOfLastKeyPressed = game->time;
        }
        if( game->input->keyboard['J'] || game->input->keyboard['j'] ) {
            
            p2Direction--;
            game->p2.timeOfLastKeyPressed = game->time;
        }
        if(game->input->keyboard['P'] || game->input->keyboard['p'])
        {
            player2Speed *=3;
        }
        else if(game->input->keyboard['['] || game->input->keyboard['{'])
        {
            if(!game->p2.missile.isFiring)
            {
                fireWeapon(game,&game->p2);
            }
        }
        
    }
    
    
    //Move based on direction.
    if(p1Direction<0)
    {
        p1Direction = PlayerDirection_NorthWest;
    }
    if(p1Direction >PlayerDirection_NorthWest)
    {
        p1Direction = 0;
    }
    if(p2Direction <0)
    {
        p2Direction = PlayerDirection_NorthWest;
    }
    if(p2Direction > PlayerDirection_NorthWest)
    {
        p2Direction = 0;
    }
    
    game->p1.direction = p1Direction;
    game->p2.direction = p2Direction;
    
    oofloat diagDiff = 0.75f;
    
    if((game->input->keyboard['W'] || game->input->keyboard['w']) && game->p1.alive)//Forwards
    {
        switch (game->p1.direction) {
            case PlayerDirection_North:
                if(playerInBounds(game, -1,game->p1.y - player1Speed,game->p1.playerIndex))
                {
                    game->p1.y -= player1Speed;
                }
                break;
            case PlayerDirection_NorthEast:
                if(playerInBounds(game, game->p1.x + _playerWidth+player1Speed,game->p1.y - player1Speed,game->p1.playerIndex))
                {
                    game->p1.y -= player1Speed * diagDiff;
                    game->p1.x += player1Speed * diagDiff;
                    
                }
                break;
            case PlayerDirection_East:
                if(playerInBounds(game,game->p1.x + _playerWidth + player1Speed,-1,game->p1.playerIndex)) //Right
                {
                    game->p1.x += player1Speed;
                }
                break;
            case PlayerDirection_SouthEast:
                if(playerInBounds(game, game->p1.x+_playerWidth+player1Speed, game->p1.y + _playerHeight + player1Speed,game->p1.playerIndex))
                {
                    game->p1.x += player1Speed * diagDiff;
                    game->p1.y += player1Speed * diagDiff;
                }
                break;
            case PlayerDirection_South:
                if(playerInBounds(game, -1, game->p1.y + _playerHeight + player1Speed,game->p1.playerIndex))
                {
                    game->p1.y += player1Speed;
                }
                break;
            case PlayerDirection_SouthWest:
                if(playerInBounds(game, game->p1.x - player1Speed, game->p1.y + _playerHeight + player1Speed,game->p1.playerIndex))
                {
                    game->p1.y += player1Speed * diagDiff;
                    game->p1.x -= player1Speed * diagDiff;
                }
                break;
            case PlayerDirection_West:
                if(playerInBounds(game, game->p1.x - player1Speed, -1,game->p1.playerIndex))
                {
                    game->p1.x -= player1Speed;
                }
                break;
            case PlayerDirection_NorthWest:
                if(playerInBounds(game, game->p1.x - player1Speed, game->p1.y - player1Speed,game->p1.playerIndex))
                {
                    game->p1.x -= player1Speed * diagDiff;
                    game->p1.y -= player1Speed * diagDiff;
                }
                break;
                
            default:
                break;
        }
    }
    
    if((game->input->keyboard['S'] || game->input->keyboard['s']) && game->p1.alive)//Backwards
    {
        switch (game->p1.direction) {
            case PlayerDirection_North:
                if(playerInBounds(game, -1,game->p1.y + player1Speed,game->p1.playerIndex))
                {
                    game->p1.y += player1Speed;
                }
                break;
            case PlayerDirection_NorthEast:
                if(playerInBounds(game, game->p1.x - _playerWidth-player1Speed,game->p1.y + player1Speed,game->p1.playerIndex))
                {
                    game->p1.y += player1Speed * diagDiff;
                    game->p1.x -= player1Speed * diagDiff;
                    
                }
                break;
            case PlayerDirection_East:
                if(playerInBounds(game,game->p1.x - _playerWidth - player1Speed,-1,game->p1.playerIndex)) //Right
                {
                    game->p1.x -= player1Speed;
                }
                break;
            case PlayerDirection_SouthEast:
                if(playerInBounds(game, game->p1.x-_playerWidth+player1Speed, game->p1.y - _playerHeight - player1Speed,game->p1.playerIndex))
                {
                    game->p1.x -= player1Speed * diagDiff;
                    game->p1.y -= player1Speed * diagDiff;
                }
                break;
            case PlayerDirection_South:
                if(playerInBounds(game, -1, game->p1.y - _playerHeight - player1Speed,game->p1.playerIndex))
                {
                    game->p1.y -= player1Speed;
                }
                break;
            case PlayerDirection_SouthWest:
                if(playerInBounds(game, game->p1.x + player1Speed, game->p1.y - _playerHeight - player1Speed,game->p1.playerIndex))
                {
                    game->p1.y -= player1Speed * diagDiff;
                    game->p1.x += player1Speed * diagDiff;
                }
                break;
            case PlayerDirection_West:
                if(playerInBounds(game, game->p1.x + player1Speed, -1,game->p1.playerIndex))
                {
                    game->p1.x += player1Speed;
                }
                break;
            case PlayerDirection_NorthWest:
                if(playerInBounds(game, game->p1.x + player1Speed, game->p1.y + player1Speed,game->p1.playerIndex))
                {
                    game->p1.x += player1Speed * diagDiff;
                    game->p1.y += player1Speed * diagDiff;
                }
                break;
                
            default:
                break;
        }
    }
    
    if((game->input->keyboard['I'] || game->input->keyboard['i']) && game->p2.alive)//Forwards
    {
        switch (game->p2.direction) {
            case PlayerDirection_North:
                if(playerInBounds(game, -1,game->p2.y - player2Speed,game->p2.playerIndex))
                {
                    game->p2.y -= player2Speed;
                }
                break;
            case PlayerDirection_NorthEast:
                if(playerInBounds(game, game->p2.x + _playerWidth+player2Speed,game->p2.y - player2Speed,game->p2.playerIndex))
                {
                    game->p2.y -= player2Speed * diagDiff;
                    game->p2.x += player2Speed * diagDiff;
                    
                }
                break;
            case PlayerDirection_East:
                if(playerInBounds(game,game->p2.x + _playerWidth + player2Speed,-1,game->p1.playerIndex)) //Right
                {
                    game->p2.x += player2Speed;
                }
                break;
            case PlayerDirection_SouthEast:
                if(playerInBounds(game, game->p2.x+_playerWidth+player2Speed, game->p2.y + _playerHeight + player2Speed,game->p2.playerIndex))
                {
                    game->p2.x += player2Speed * diagDiff;
                    game->p2.y += player2Speed * diagDiff;
                }
                break;
            case PlayerDirection_South:
                if(playerInBounds(game, -1, game->p2.y + _playerHeight + player2Speed,game->p2.playerIndex))
                {
                    game->p2.y += player2Speed;
                }
                break;
            case PlayerDirection_SouthWest:
                if(playerInBounds(game, game->p2.x - player2Speed, game->p2.y + _playerHeight + player2Speed,game->p2.playerIndex))
                {
                    game->p2.y += player2Speed * diagDiff;
                    game->p2.x -= player2Speed * diagDiff;
                }
                break;
            case PlayerDirection_West:
                if(playerInBounds(game, game->p2.x - player2Speed, -1,game->p2.playerIndex))
                {
                    game->p2.x -= player2Speed;
                }
                break;
            case PlayerDirection_NorthWest:
                if(playerInBounds(game, game->p2.x - player2Speed, game->p2.y - player2Speed,game->p2.playerIndex))
                {
                    game->p2.x -= player2Speed * diagDiff;
                    game->p2.y -= player2Speed * diagDiff;
                }
                break;
                
            default:
                break;
        }
    }
    
    if((game->input->keyboard['K'] || game->input->keyboard['k']) && game->p2.alive)//Backwards
    {
        switch (game->p2.direction) {
            case PlayerDirection_North:
                if(playerInBounds(game, -1,game->p2.y + player2Speed,game->p2.playerIndex))
                {
                    game->p2.y += player2Speed;
                }
                break;
            case PlayerDirection_NorthEast:
                if(playerInBounds(game, game->p2.x - _playerWidth-player2Speed,game->p2.y + player2Speed,game->p2.playerIndex))
                {
                    game->p2.y += player2Speed * diagDiff;
                    game->p2.x -= player2Speed * diagDiff;
                    
                }
                break;
            case PlayerDirection_East:
                if(playerInBounds(game,game->p2.x - _playerWidth - player2Speed,-1,game->p2.playerIndex)) //Right
                {
                    game->p2.x -= player2Speed;
                }
                break;
            case PlayerDirection_SouthEast:
                if(playerInBounds(game, game->p2.x-_playerWidth+player2Speed, game->p2.y - _playerHeight - player2Speed,game->p2.playerIndex))
                {
                    game->p2.x -= player2Speed * diagDiff;
                    game->p2.y -= player2Speed * diagDiff;
                }
                break;
            case PlayerDirection_South:
                if(playerInBounds(game, -1, game->p2.y - _playerHeight - player2Speed,game->p2.playerIndex))
                {
                    game->p2.y -= player2Speed;
                }
                break;
            case PlayerDirection_SouthWest:
                if(playerInBounds(game, game->p2.x + player2Speed, game->p2.y - _playerHeight - player2Speed,game->p2.playerIndex))
                {
                    game->p2.y -= player2Speed * diagDiff;
                    game->p2.x += player2Speed * diagDiff;
                }
                break;
            case PlayerDirection_West:
                if(playerInBounds(game, game->p2.x + player2Speed, -1,game->p2.playerIndex))
                {
                    game->p2.x += player2Speed;
                }
                break;
            case PlayerDirection_NorthWest:
                if(playerInBounds(game, game->p2.x + player2Speed, game->p2.y + player2Speed,game->p2.playerIndex))
                {
                    game->p2.x += player2Speed * diagDiff;
                    game->p2.y += player2Speed * diagDiff;
                }
                break;
                
            default:
                break;
        }
    }
}



oouint64 gameMemorySize() {
    return sizeof(cbgame);
}

void moveWall(cbgame *game)
{
    oofloat wallSpeed = game->input->dt *50;
 
    if(game->arena.leftWall.y + game->arena.leftWall.height + wallSpeed > game->output->graphic.height)
    {
        _moveWallUp = oofalse;
    }
   
    if(!_moveWallUp && game->arena.leftWall.y - wallSpeed <= 0 + _playerTurretWidth*2)
    {
        _moveWallUp = ootrue;
    }
    
    if(_moveWallUp)
    {
        game->arena.leftWall.y += wallSpeed;

    }
    else if ( !_moveWallUp)
    {
        game->arena.leftWall.y -= wallSpeed;

    }
}

void advanceGame(void* gameMemory, OOGameInput* input, OOGameOutput* output) {
    cbgame * game = (cbgame*) gameMemory;
    oofloat respawnTime = 2.f;
    if(game->p1.score == CombatGame_ScoreLimit)
    {
        //Player 1 wins
    }
    else if (game->p2.score == CombatGame_ScoreLimit)
    {
        //Player 2 Wins
    }
    
    if(!game->p1.alive && game->time >= game->p1.timeOfDeath + respawnTime)
    {
        respawnPlayer(game, &game->p1);
    }
    else if (!game->p2.alive && game->time >= game->p2.timeOfDeath + respawnTime)
    {
        respawnPlayer(game, &game->p2);
    }
    game->input = input;
    game->output = output;
    
    if( !game->initialised ) {
        initGame(game);
    }
    
    game->moveDelta = game->p1.x;
    updateGame(game);
    game->moveDelta -= game->p1.x;
    
    clearScreen(game);
    moveWall(game);
    drawPlayer(game, &game->p2);
    drawPlayer(game, &game->p1);
    drawArena(game);
    

    if(game->p1.missile.isFiring)
    {
        drawMissile(game,&game->p1);
    }
    if(game->p2.missile.isFiring)
    {
        drawMissile(game,&game->p2);
    }
    
    
    game->time += game->input->dt;
}

