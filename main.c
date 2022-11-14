/*
Program: A "Space Invaders" game in the C console

The aim is to shoot aliens by firing shots from your ship without getting hit by alien bombs dropping down.
Getting hit by an alien shot causes you to lose a life.
If the aliens reach the bottom of the screen, the game is instantly lost.
Different types of alien are worth different amounts of points.
There is a "rare" UFO which is worth a semi-random amount of points.
The player has four barricades which will block enemy shots but will degrade the more they get hit.
You can only shoot again once your previous shot has hit something.
Every 5 aliens that are destroyed will cause the game to speed up (not including UFOs).
Higher-scoring aliens will shoot more often.
Once all the aliens in a wave have been destroyed the player gains an extra life and 1000 points, then a new wave is spawned slightly lower down.
Once you run out of lives you lose the game.
The game is endless and the aim is to get as high of a score as possible.
Quitting the game will still allow you to save your high-score if you have one.
The game can be paused and muted.

Assignment: ACS130-006
Author: Jamie Canning
Date Created: 15/04/19
*/

//MUST INCLUDE winmm IN LINKER SETTINGS

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <windows.h>
#include <conio.h>
#include <unistd.h>

//CONSTANTS
#define TRUE 1
#define FALSE 0
#define STARTING_LIVES 3
#define MAX_LIVES 8
#define COLOUR_GREEN "\033[1;32m"
#define COLOUR_RED "\033[1;31m"
#define COLOUR_YELLOW "\033[1;33m"
#define COLOUR_MAGENTA "\033[1;35m"
#define COLOUR_CYAN "\033[1;36m"
#define COLOUR_RESET "\033[0m"
#define REFRESHRATE 30
#define MENU_FILE "resources/menu.txt"
#define GAMESCREEN_FILE "resources/gamestage.txt"
#define GAMEOVER_FILE "resources/gameover.txt"
#define MUSIC_SOUND "resources/music.wav"
#define DEATH_SOUND "resources/explosion.wav"
#define SCORESDISPLAY_FILE "resources/scores.txt"
#define SCORES_FILE "resources/scoresdata.txt"
#define NAMEENTER_FILE "resources/nameEnter.txt"
#define CHANCE_UFO 400
#define CHANCE_ALIENBOMB 2000
#define MULTIPLIER_ALIEN 16
#define MULTIPLIER_UFO 2
#define MULTIPLIER_BOMB 2
#define RIGHTBORDER 80
#define LEFTBORDER 2
#define TOPBORDER 3
#define BOTTOMBORDER 19
#define ALIENROWS 5
#define ALIENCOLS 11

//Structure for the player's shot
struct playerShot
{
    int alive;
    char graphic;
    char colour[15];
    COORD coords;
    COORD prevCoords;
};

//Structure for an alien shot
struct alienBomb
{
    int alive;
    char graphic;
    COORD coords;
    COORD prevCoords;
};

//Player structure
struct player
{
    int alive;
    int lives;
    int maxLives;   //Player lives cannot exceed this value
    int length; //Length of graphic
    char graphicNormal[6];
    char graphicShoot[6];
    char graphicDead[6];
    char colour[15];
    int graphicState;
    COORD prevCoords; //Used to erase the old player characters from the screen
    COORD coords;
    struct playerShot shot; //Keeps track of the player's shot

};

//Barricade block structure
struct barricadeBlock
{
    int alive;
    int stage;  //Keeps track of which graphic is to be used
    char stageDisplay[5];   //Different display depending on how many times block has been hit
    COORD coords;
    char colour[15];
};

struct barricade
{
    COORD coords;   //Coords of bottom left block
    struct barricadeBlock blocks[3][4];
};

//Alien structure
struct alien
{
    int alive;  //Boolean, whether alien is alive or not (alive is 1)
    int score;  //Score for when alien shot
    int length; //Length of graphic
    int graphicState;   //Which frame of graphic is next to be printed
    char graphics[2][4];    //Contains graphics
    char colour[15];    //Display colour string
    COORD coords;   //Current coordinates of leftmost graphic character
    COORD prevCoords;   //Coordinates of the alien's previous position
    int graphicMultiplier;  //If equal to multiplier max, graphics can change
    int graphicMultiplierMax;   //How quickly the graphics change
    int graphicNeedsRemoving;   //Whether the previous graphics for this alien needs removing after aliens move down
    int canShoot;   //Can this alien drop a bomb
    struct alienBomb bomb;  //Keeps track of the alien's bomb
};

//UFO structure
struct ufo
{
    int alive;
    int score;
    int length;
    int graphicState;
    char graphics[4][6];
    char colour[15];
    int direction;  //0 for right, 1 for left
    COORD coords;
    COORD prevCoords;
    int graphicMultiplier;
    int graphicMultiplierMax;
};

struct music
{
    int currentNote;    //Which note is next to be played.
    int mute;   //Is music muted?
};

//Wave of aliens structure
struct wave
{
    struct alien aliens[ALIENROWS][ALIENCOLS];    //Always contains 5 rows of 11 aliens
    int direction;  //Right is 0, left is 1
    COORD startingCoords;   //Where the bottom left alien starts
    int waveComplete;   //1 if all aliens destroyed
    int numAliensRemaining; //How many aliens are left in the current wave
};

//Game structure
struct game
{
    int gameScore;  //Score for current game
    int level;  //Used to track difficulty
    struct wave currentWave;    //Current wave of aliens
    struct player player1;  //Stores the player
    struct ufo ufo1;    //Stores UFO
    struct music musicBox;   //Stores music
    int speed;  //How quickly the game updates - based on number of aliens left in current wave
    int gameCounter;    //Essentially a timer for when things happen, increments by 1 every game loop
    struct barricade barricades[4]; //Stores the game barricades
};

//FUNCTION DECLARATIONS
void getUserInput(char *key);   //Get user input
void getMenuChoice();   //Main menu
void displayScreen(char filename[50]);   //Display screen
void displayHighScores();   //Display the high scores
void getScores(int scores[5], char names[5][3]);    //Get high scores as an array
void setScores(int scores[5], char names[5][3]);    //Set scores and names in the file
int checkHighScore(int scores[5], int newScore);    //Check if a new high score has been set
void setHighScore(int scores[5], char names[5][3], int newScore, char newName[3]);  //Set the new high score and corresponding name
void getUserName(char name[3]); //Get the name of new high score
void assignName(char name1[3], char name2[3]);
void gameLoop();    //Main game loop
void drawGame(HANDLE hConsole, struct game *currentGame);   //Draw all entities
void levelUp(struct game *currentGame); //Progress to next level
void updateSpeed(struct game *currentGame); //Change game speed based on number of aliens left in current wave
void movePlayer(struct player *player1, int direction); //Move player in specified direction
void moveAliens(struct wave *thisWave); //Moves aliens
void moveUFO(struct ufo *ufo1); //Moves the ufo
int aliensAtLeftEdge(struct wave thisWave);    //Whether the wave of aliens is at the left edge of the game screen
int aliensAtRightEdge(struct wave thisWave);   //Whether the wave of aliens is at the right edge of the game screen
int aliensAtBottom(struct wave thisWave);  //Whether the wave of aliens has reached the bottom border
void alienGraphicRemovalCheck(struct wave *thisWave, int i, int j);   //Checks each alien to see if it's graphic needs removing when it moves down
void alienShootCheck(struct wave *thisWave);    //Checks which aliens can drop a bomb
void spawnBombs(struct wave *thisWave); //Spawn bombs
struct game initializeGame();   //Create new game
struct wave newWave(struct game currentGame); //Generates a new wave depending on the level
void createWaveAliens(struct alien aliens[ALIENROWS][ALIENCOLS], COORD startingCoords);    //Generate aliens in the wave
struct player createPlayer();   //Create instance of player structure
struct playerShot createPlayerShot();   //Create instance of the player's shot
void playerShoot(struct player *player1);   //Fire shot from player
void updatePlayerShot(struct playerShot *shot); //Keep the shot moving
void killThings(struct game *currentGame);  //Essentially checking whether the player's shot has collided with anything
void killThingsWithBomb(struct game *currentGame);  //Check if an alien bomb has hit anything
int hitBarricade(struct barricade barricades[4], COORD shotCoords); //Checks if a barricade block has been hit
void hitBarricadeBlock(struct barricadeBlock *block);   //Updates a barricade block if it has been hit
struct barricadeBlock createBarricadeBlock(COORD coords);   //Create a barricade block
struct barricade createBarricade(COORD coords); //Create a whole barricade
struct alien createAlien10(COORD coords);   //Create instance of alien worth 10
struct alien createAlien20(COORD coords);   //Create instance of alien worth 20
struct alien createAlien40(COORD coords);   //Create instance of alien worth 40
struct alienBomb createAlienBomb(); //Create instance of alien bomb
void alienShoot(struct alien *alien1);    //Fire bomb from alien
void updateAlienBomb(struct alienBomb *bomb);   //Keep the bomb moving
struct ufo createUFO(int direction); //Create instance of ufo structure
void spawnUFO(struct ufo *ufo1);    //Spawn a ufo
void displayBarricade(HANDLE hConsole, struct barricade barricade1);    //Display the blocks of a barricade
void displayPlayer(HANDLE hConsole, struct player *player1); //Display the player
void killPlayer(struct game *currentGame);  //Lose a life
void removeAllBombs(struct wave *currentWave);  //Kill all alien bombs currently alive
void displayPlayerShot(HANDLE hConsole, struct playerShot shot);    //Display the player's shot
void killPlayerShot(struct playerShot *shot);   //Kill player shot
void displayAlien(HANDLE hConsole, struct alien *alien1);    //Display an alien
void killAlien(struct alien *alien1);   //Kill an alien
void displayAlienBomb(HANDLE hConsole, struct alienBomb bomb);  //Display an alien bomb
void killAlienBomb(struct alienBomb *bomb); //Kill an alien bomb
void displayUFO(HANDLE hConsole, struct ufo *ufo1);    //Display a ufo
void killUFO(struct ufo *ufo1); //Kill the ufo
void clrscn();  //Clear console
void pauseGame();   //Pause the game
void muteMusicToggle(struct game *currentGame); //Toggle whether music is muted
void playSound(char filename[]);    //Play a note
struct music createMusic(); //Create music structure
void updatePlayerScore(struct game *currentGame, int score);    //Update the player's score
void displayScore(HANDLE hConsole, struct game currentGame);    //Display the current score
void displayLives(HANDLE hConsole, struct game currentGame);    //Display the player's lives left
void displayLevel(HANDLE hConsole, struct game currentGame);    //Display the current level
void displayGameOver(HANDLE hConsole, struct game *currentGame);    //Display final screen
void fixBorders(HANDLE hConsole);
int coordsWithinBorders(COORD coords);  //Check if coords are within game borders
void hideCursor();  //Hide the cursor

int main(void)
{
    //Hide cursor
    hideCursor();

    //Set seed for rand()
    srand(time(0));

    //TITLE SCREEN
    getMenuChoice();

    return 0;
}

//Get user input
void getUserInput(char *key)
{
    *key = getch();
}

//Main menu
void getMenuChoice()
{
    clrscn();   //Clear terminal

    int quitGame = FALSE;   //Has user chosen to quit game
    char userChoice;

    //Set up main menu display aliens
    struct alien alien10 = createAlien10((COORD){36, 9});
    struct alien alien20 = createAlien20((COORD){36, 8});
    struct alien alien40 = createAlien40((COORD){36, 7});
    struct ufo ufo1 = createUFO(0);
    ufo1.coords = (COORD){36, 6};

    alien10.graphicMultiplier = MULTIPLIER_ALIEN - 1;
    alien20.graphicMultiplier = MULTIPLIER_ALIEN - 1;
    alien40.graphicMultiplier = MULTIPLIER_ALIEN - 1;
    ufo1.graphicMultiplier = MULTIPLIER_UFO - 1;

    //Display the main menu screen
    displayScreen(MENU_FILE);
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    while(!quitGame){

        hideCursor();

        //Make the aliens move
        Sleep(REFRESHRATE);
        displayAlien(hConsole, &alien10);
        displayAlien(hConsole, &alien20);
        displayAlien(hConsole, &alien40);
        displayUFO(hConsole, &ufo1);

        //Wait for a key to be pressed
        if(kbhit()){

            //Get the key that was pressed
            getUserInput(&userChoice);

            switch(userChoice){
            case 'h':
                clrscn();
                displayHighScores();  //Display high-scores
                clrscn();
                displayScreen(MENU_FILE);   //Display menu again when back from high-scores
                break;

            case 'q':
                clrscn();
                quitGame = TRUE;    //Exit game
                break;

            case ' ':
                clrscn();
                gameLoop(); //Start new game
                clrscn();
                displayScreen(MENU_FILE);   //Display main menu again when back from game
                break;
            }
        }
    }
}

//Display game screen
void displayScreen(char filename[50])
{
    FILE *fin = fopen(filename, "r");  //Open text file

    //Check if file present
    if(fin == NULL){
        printf("ERROR: File not found, please reinstall the game.");
    }

    char c;
    while(fscanf(fin, "%c", &c) != EOF){
        printf("%c", c);    //Print to console
    }

    //Close file
    fclose(fin);
}

void displayHighScores()
{
    //Display the high-scores screen
    displayScreen(SCORESDISPLAY_FILE);
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    int scores[5] = {};
    char names[5][3];

    //Store the scores from file in arrays
    getScores(scores, names);

    int i, j;
    for(i = 0; i < 5; i++){
        //Print scores
        SetConsoleCursorPosition(hConsole, (COORD){42, 6 + i*2});
        printf("%d", scores[i]);

        //Print names
        SetConsoleCursorPosition(hConsole, (COORD){36, 6 + i*2});
        for(j = 0; j < 3; j++){
            printf("%c", names[i][j]);
        }
    }

    char userChoice;
    int returnToMenu = FALSE;

    //Wait for 'm' key to be pressed to return to main menu
    while(!returnToMenu){
        if(kbhit()){

            getUserInput(&userChoice);

            switch(userChoice)
            {
                case 'm':
                    returnToMenu = TRUE;
                    break;
            }
        }
    }
}

//Get the scores and names from the file
void getScores(int scores[5], char names[5][3])
{
    //Open scores file
    FILE *fin = fopen(SCORES_FILE, "r");

    //Check if file exists
    if(fin == NULL){
        printf("ERROR: File not found, please reinstall the game.");
    }

    int counter = 0;
    int nameCounter = 0;
    char c = 0;

    //Keep iterating through file
    while(fscanf(fin, "%c", &c) != EOF){

        nameCounter = 0;

        //Get first name characters
        while(c != ','){
            names[counter][nameCounter++] = c;
            fscanf(fin, "%c", &c);
        }

        //Get score for that name
        fscanf(fin, "%d", &scores[counter]);
        counter++;

        //Scan the trailing comma;
        fscanf(fin, "%c", &c);
    }

    //Close the file
    fclose(fin);
}

//Set the scores and names in the file
void setScores(int scores[5], char names[5][3])
{
    //Open the scores file
    FILE *fin = fopen(SCORES_FILE, "w");

    int i, j;
    for(i = 0; i < 5; i++){
        for(j = 0; j < 3; j++){
            //Print characters of name
            fprintf(fin, "%c", names[i][j]);
        }
        //Print score surrounded by commas
        fprintf(fin, ",%d,", scores[i]);
    }

    //Close the file
    fclose(fin);
}

//Check for new high score
int checkHighScore(int scores[5], int newScore)
{
    int i;
    for(i = 0; i < 5; i++)
    {
        //Check if new score is greater than any of the high scores
        if(scores[i] < newScore){
            return TRUE;
        }
    }

    return FALSE;
}

//Set new high score and name
void setHighScore(int scores[5], char names[5][3], int newScore, char newName[3])
{
    int i, temp;
    char tempName[3];
    for(i = 0; i < 5; i++)
    {
        if(scores[i] < newScore){
            //Store high score and name from file in temp before overwriting with new score and name
            temp = scores[i];
            assignName(tempName, names[i]);
            scores[i] = newScore;
            assignName(names[i], newName);

            //Set newScore as previous high score, which will be greater than next high score and so will shift all scores down
            newScore = temp;
            assignName(newName, tempName);
        }
    }
}

//Used to easily assign a username to a different variable as it is always 3 characters
void assignName(char name1[3], char name2[3])
{
    int i;
    for(i = 0; i < 3; i++)
    {
        name1[i] = name2[i];
    }
}

void getUserName(char name[3])
{
    clrscn();
    //Display screen for name entering
    displayScreen(NAMEENTER_FILE);
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    char userChoice;
    int nameComplete = FALSE;
    int counter = 0;
    name[0] = '_';
    name[1] = '_';
    name[2] = '_';

    while(!nameComplete){
        getUserInput(&userChoice);

        switch(userChoice)
        {
            //If a backspace entered
            case '\010':
                if(counter > 0){
                    //As long as characters have been entered, remove most recently entered character
                    name[--counter] = '_';
                }
                break;

            //If enter key pressed
            case '\015':
                if(counter > 2){
                    //Finish entering name if 3 characters have been entered
                    nameComplete = TRUE;
                }

            default:
                if(counter <= 2 && userChoice >= 33 && userChoice <= 126 && userChoice != '_' && userChoice != ','){
                    name[counter++] = toupper(userChoice);
                }
        }

        //Display currently entered characters
        int i;
        for(i = 0; i < 3; i++){
            SetConsoleCursorPosition(hConsole, (COORD){37 + 3*i, 11});
            if(name[i] == '_'){
                printf("_");
            }
            else{
                printf("%c", name[i]);
            }
        }
    }
}

//Main game loop
void gameLoop()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  //Handle for console operations
    displayScreen(GAMESCREEN_FILE); //Display the main game screen
    struct game currentGame = initializeGame(); //Create a new game struct
    int quitGame = FALSE;
    playSound(MUSIC_SOUND); //Start music

    //Runs while game has not been quit and player still has lives left
    while(currentGame.player1.lives > 0 && quitGame == FALSE){

        hideCursor();

        //Increment game counter
        currentGame.gameCounter++;
        currentGame.gameCounter %= 128;

        //Get user input
        if(kbhit()){

            switch(getch()){
                case '.':
                    //Move player right
                    movePlayer(&currentGame.player1, 0);
                    break;

                case ',':
                    //Move player left
                    movePlayer(&currentGame.player1, 1);
                    break;

                case 'p':
					//Pause game
                    pauseGame();
                    break;

                case 'q':
					//Exit game loop
                    quitGame = TRUE;
                    break;

                case 'm':
					//Toggle music mute
                    muteMusicToggle(&currentGame);
                    break;

                case ' ':
                    //Shoot if there is not already a shot on screen
                    if(currentGame.player1.shot.alive == FALSE){
                        playerShoot(&currentGame.player1);
                    }
                    break;
            }
        }

        //Update game

        //End game if aliens reach bottom of screen
        if(aliensAtBottom(currentGame.currentWave) == TRUE){
            currentGame.player1.lives = 1;
            killPlayer(&currentGame);
        }

        //Move aliens
        if(currentGame.gameCounter % MULTIPLIER_ALIEN == 0){
            moveAliens(&currentGame.currentWave);   //Move the aliens
        }

        //Bombs
        if(currentGame.gameCounter % MULTIPLIER_BOMB == 0){
            //Chance to spawn bombs from aliens
            spawnBombs(&currentGame.currentWave);
            //Check if anything has been hit
            killThingsWithBomb(&currentGame);
        }

        //Move player's shot if shot alive
        if(currentGame.player1.shot.alive == TRUE){
            updatePlayerShot(&currentGame.player1.shot);
            killThings(&currentGame);   //Check if anything has been hit
            if(currentGame.currentWave.numAliensRemaining <= 0){
                //Check if wave has been defeated
                currentGame.currentWave.waveComplete = TRUE;
            }
        }

        //Chance to spawn UFO
        if(currentGame.ufo1.alive == FALSE){
            if(rand() % CHANCE_UFO == 0){
                spawnUFO(&currentGame.ufo1);
            }
        }
        else{
            if(currentGame.gameCounter % MULTIPLIER_UFO == 0){
                //If there is currently a UFO alive, move it
                moveUFO(&currentGame.ufo1);
            }
        }

        //Draw all entities
        drawGame(hConsole, &currentGame);

        //Check for level up
        if(currentGame.currentWave.waveComplete == TRUE){
            levelUp(&currentGame);   //Advance to next level
        }

        Sleep(currentGame.speed);
    }

    //Turn off music
    PlaySound(NULL, NULL, SND_ASYNC);

    //Display game over screen
    if(quitGame == FALSE){
        currentGame.player1.graphicState = -1;
        displayGameOver(hConsole, &currentGame);
    }

    //Check for high score
    int scores[5] = {};
    char names[5][3] = {};
    char newName[3];

    //Get current high scores
    getScores(scores, names);

    //If a new high score has been set
    if(checkHighScore(scores, currentGame.gameScore) == TRUE){
        getUserName(newName);   //Get name for high score
        setHighScore(scores, names, currentGame.gameScore, newName);    //Adjust arrays
        setScores(scores, names);   //Write to file
    }

}

//Draw game entities
void drawGame(HANDLE hConsole, struct game *currentGame)
{
    int i, j, k;

    //Draw barricades
    for(k = 0; k < 4; k++){
        displayBarricade(hConsole, (*currentGame).barricades[k]);
    }

    //Draw aliens
    for(i = 0; i < ALIENROWS; i++){
        for(j = 0; j < ALIENCOLS; j++){
            displayAlien(hConsole, &(*currentGame).currentWave.aliens[i][j]);
            displayAlienBomb(hConsole, (*currentGame).currentWave.aliens[i][j].bomb);
        }
    }

    //Self explanatory
    displayPlayer(hConsole, &(*currentGame).player1);
    displayUFO(hConsole, &(*currentGame).ufo1);
    displayScore(hConsole, (*currentGame));
    displayLives(hConsole, (*currentGame));
    displayLevel(hConsole, (*currentGame));
    fixBorders(hConsole);
}

//Level up
void levelUp(struct game *currentGame)
{
    //Player gets score + 1000
    updatePlayerScore(&(*currentGame), 1000);

    //Reset speed and counter
    (*currentGame).speed = REFRESHRATE;
    (*currentGame).gameCounter = 0;

    //Increase level number
    (*currentGame).level++;

    //Kill the ufo
    if((*currentGame).ufo1.alive == TRUE){
        killUFO(&(*currentGame).ufo1);
    }

    //Increase player lives by 1 if not at max
    if((*currentGame).player1.lives < (*currentGame).player1.maxLives){
        (*currentGame).player1.lives++;
    }

    //Create a new wave
    (*currentGame).currentWave = newWave((*currentGame));
}

//Speed based on aliens remaining
void updateSpeed(struct game *currentGame)
{
    int num = (*currentGame).currentWave.numAliensRemaining;
    if(num > 0){
        //Speed increases for every 5 aliens destroyed
        (*currentGame).speed = REFRESHRATE - 18 + (((num-1) / 5) + 1) * 2;
    }
}

//Move player
void movePlayer(struct player *player1, int direction)
{
    //Store current coords of player before moving
    (*player1).prevCoords = (*player1).coords;

    if(direction == 0 && (*player1).coords.X < RIGHTBORDER - 5){
        //Move one to the right if not at right edge
        (*player1).coords.X += 1;
    }
    else if(direction == 1 && (*player1).coords.X > LEFTBORDER){
        //Move one to the left if not at left edge
        (*player1).coords.X -= 1;
    }
}

//Move aliens
void moveAliens(struct wave *thisWave)
{
    int atWall;

    //Check if the wave has reached a wall
    if(((*thisWave).direction == 1 && aliensAtLeftEdge(*thisWave)) || ((*thisWave).direction == 0 && aliensAtRightEdge(*thisWave))){
        atWall = TRUE;
    }
    else{
        atWall = FALSE;
    }

    int i, j;
    for(i = 0; i < ALIENROWS; i++){
        for(j = 0; j < ALIENCOLS; j++){

            //Store current coords of alien before moving
            (*thisWave).aliens[i][j].prevCoords = (*thisWave).aliens[i][j].coords;

            if(atWall == TRUE){
                //Move down
                (*thisWave).aliens[i][j].coords.Y += 1;

                //Switch direction
                (*thisWave).direction += 1;
                (*thisWave).direction %= 2;
            }
            else{
                if((*thisWave).direction == 0){
                    //Move right
                    (*thisWave).aliens[i][j].coords.X += 1;
                }
                else{
                    //Move left
                    (*thisWave).aliens[i][j].coords.X -= 1;
                }
            }

            //Check if aliens need their graphic removing after moving
            alienGraphicRemovalCheck(&(*thisWave), i, j);
        }
    }
}

//Move the ufo
void moveUFO(struct ufo *ufo1)
{
    //Store current coords before moving
    (*ufo1).prevCoords = (*ufo1).coords;

    if((*ufo1).direction == 0){
        //Move right
        (*ufo1).coords.X += 1;
    }
    else if((*ufo1).direction == 1){
        //Move left
        (*ufo1).coords.X -= 1;
    }

    //Kill UFO if at border
    if(((*ufo1).direction == 1 && (*ufo1).coords.X == LEFTBORDER - (*ufo1).length + 1) ||((*ufo1).direction == 0 && (*ufo1).coords.X == RIGHTBORDER)){
        killUFO(&(*ufo1));
    }
}

//Checks whether the aliens have reached the left edge
int aliensAtLeftEdge(struct wave thisWave)
{
    int i, j;
    for(j = 0; j < ALIENCOLS; j++){
        for(i = 0; i < ALIENROWS; i++){
            //Only checks alive aliens
            if(thisWave.aliens[i][j].alive == TRUE){
                if(thisWave.aliens[i][j].coords.X == LEFTBORDER){
                    return TRUE;
                }
                else{
                    return FALSE;
                }
            }
        }
    }

    return FALSE;
}

//Checks whether the aliens have reached the right edge
int aliensAtRightEdge(struct wave thisWave)
{
    int i, j;
    for(j = ALIENCOLS - 1; j >= 0; j--){
        for(i = 0; i < ALIENROWS; i++){
            if(thisWave.aliens[i][j].alive == TRUE){
                if(thisWave.aliens[i][j].coords.X == RIGHTBORDER - thisWave.aliens[i][j].length){
                    return TRUE;
                }
                else{
                    return FALSE;
                }
            }
        }
    }

    return FALSE;
}

//Checks if aliens have reached the bottom border
int aliensAtBottom(struct wave thisWave)
{
    int i, j;
    for(i = 0; i < ALIENROWS; i++){
        for(j = 0; j < ALIENCOLS; j++){
            if(thisWave.aliens[i][j].alive == TRUE){
                if(thisWave.aliens[i][j].coords.Y == BOTTOMBORDER){
                    return TRUE;
                }
                else{
                    return FALSE;
                }
            }
        }
    }

    return FALSE;
}

//Checks whether aliens need their graphics removing when they move down
void alienGraphicRemovalCheck(struct wave *thisWave, int i, int j)
{
    if(i == ALIENROWS - 1){
        //Aliens in top row will always need their graphic removing
        (*thisWave).aliens[i][j].graphicNeedsRemoving = TRUE;
    }
    else if((*thisWave).aliens[i][j].alive == FALSE){
        //Dead aliens will need their graphic removing
        (*thisWave).aliens[i][j].graphicNeedsRemoving = TRUE;
    }
    else if((*thisWave).aliens[i+1][j].alive == FALSE){
        //Aliens below a dead alien will need their graphic removing
        (*thisWave).aliens[i][j].graphicNeedsRemoving = TRUE;
    }
    else{
        (*thisWave).aliens[i][j].graphicNeedsRemoving = FALSE;
    }
}

//Check if aliens can shoot (no aliens below them)
void alienShootCheck(struct wave *thisWave)
{
    int i, j;
    for(i = 0; i < ALIENROWS; i++){
        for(j = 0; j < ALIENCOLS; j++){
            if(i == 0){
                //Aliens on the bottom row can always shoot
                (*thisWave).aliens[i][j].canShoot = TRUE;
            }
            else if((*thisWave).aliens[i-1][j].alive == TRUE || (*thisWave).aliens[i-1][j].canShoot == FALSE){
                //If the alien below is alive or cannot shoot, then this alien cannot shoot
                (*thisWave).aliens[i][j].canShoot = FALSE;
            }
            else{
                (*thisWave).aliens[i][j].canShoot = TRUE;
            }
        }
    }
}

//Drop random bombs
void spawnBombs(struct wave *thisWave)
{
    int i, j;
    for(i = 0; i < ALIENROWS; i++){
        for(j = 0; j < ALIENCOLS; j++){
            if((*thisWave).aliens[i][j].canShoot == TRUE && (*thisWave).aliens[i][j].alive == TRUE){
                if(rand() % (int)(CHANCE_ALIENBOMB/(*thisWave).aliens[i][j].score) == 0 && (*thisWave).aliens[i][j].bomb.alive == FALSE){
                    //Higher scoring aliens have a higher chance of dropping a bomb
                    alienShoot(&(*thisWave).aliens[i][j]);
                }
            }
            updateAlienBomb(&(*thisWave).aliens[i][j].bomb);
        }
    }
}

//Create new game
struct game initializeGame()
{
    struct game newGame;

    newGame.gameScore = 0;  //Score starts at zero
    newGame.level = 1;  //Game starts at level 1
    newGame.currentWave = newWave(newGame); //Generate aliens for initial game
    newGame.player1 = createPlayer();   //Generate player
    newGame.ufo1 = createUFO(0);    //Generate UFO
    newGame.ufo1.alive = FALSE; //UFO starts of not alive
    newGame.musicBox = createMusic();
    newGame.speed = REFRESHRATE;    //Default game speed
    newGame.gameCounter = 0;    //Game counter starts at 0

    //Create barricades
    int i;
    for(i = 0; i < 4; i++){
        newGame.barricades[i] = createBarricade((COORD){LEFTBORDER + 13 + 16*i, BOTTOMBORDER - 2});
    }

    return newGame;
};

//Generate new wave of aliens
struct wave newWave(struct game currentGame)
{
    struct wave newWave;

    newWave.waveComplete = 0;
    newWave.direction = 0;
    newWave.numAliensRemaining = 55;

    //Wave starting coords based on level
    COORD coords;
    switch((currentGame.level - 2) % 8){
        case 0:
            coords = (COORD){19, 11};
            break;

        case 1:
            coords = (COORD){19, 12};
            break;

        case 2:
            coords = (COORD){19, 13};
            break;

        case 3:
            coords = (COORD){19, 13};
            break;

        case 4:
            coords = (COORD){19, 13};
            break;

        case 5:
            coords = (COORD){19, 14};
            break;

        case 6:
            coords = (COORD){19, 14};
            break;

        case 7:
            coords = (COORD){19, 14};
            break;

        default:
            break;
    }

    //First level starting coords only appear once
    if(currentGame.level == 1){
        coords = (COORD){19, 10};
    }

    newWave.startingCoords = coords;    //Assign starting coords for wave

    //Generate aliens in the wave
    createWaveAliens(newWave.aliens, newWave.startingCoords);

    //Check which aliens can shoot
    alienShootCheck(&currentGame.currentWave);

    return newWave;
};

//Generate aliens in the wave
void createWaveAliens(struct alien aliens[5][11], COORD startingCoords)
{
    int i;
    for(i = 0; i < ALIENCOLS; i++){
        //Each alien in a row starts 4 characters to the right of the previous
        aliens[0][i] = createAlien10((COORD){startingCoords.X + 4*i, startingCoords.Y});
        aliens[1][i] = createAlien10((COORD){startingCoords.X + 4*i, startingCoords.Y - 1});
        aliens[2][i] = createAlien20((COORD){startingCoords.X + 4*i, startingCoords.Y - 2});
        aliens[3][i] = createAlien20((COORD){startingCoords.X + 4*i, startingCoords.Y - 3});
        aliens[4][i] = createAlien40((COORD){startingCoords.X + 4*i, startingCoords.Y - 4});
    }
}

//Create instance of player structure
struct player createPlayer()
{
    struct player player1;

    player1.alive = TRUE;
    player1.lives = STARTING_LIVES;
    player1.maxLives = MAX_LIVES;
    player1.length = 5;
    strcpy(player1.graphicNormal, "/-^-\\");
    strcpy(player1.graphicShoot, "\\_|_/");
    strcpy(player1.graphicDead, "%x*/-");
    player1.graphicState = 0;
    strcpy(player1.colour, COLOUR_GREEN);
    player1.coords = (COORD){5,BOTTOMBORDER};
    player1.prevCoords = player1.coords;

    //Initialise the player shot
    player1.shot = createPlayerShot();

    return player1;
};

//Generate the player's shot
struct playerShot createPlayerShot()
{
    struct playerShot shot;

    shot.alive = FALSE;
    shot.graphic = '!';
    strcpy(shot.colour, COLOUR_RESET);

    return shot;
};

//Fire a shot from the player
void playerShoot(struct player *player1)
{
    (*player1).shot.coords = (COORD){(*player1).coords.X + 2, (*player1).coords.Y};
    (*player1).shot.alive = TRUE;
    (*player1).graphicState = 1;
}

//Move the player's active shot
void updatePlayerShot(struct playerShot *shot)
{
    (*shot).prevCoords = (*shot).coords;
    if((*shot).alive == TRUE){
        //Move up by one
        (*shot).coords.Y -= 1;
    }

    if((*shot).coords.Y < TOPBORDER){
        //Kill if at top border
        killPlayerShot(&(*shot));
    }
}

//Check collision of player shot with enemy or barricade
void killThings(struct game *currentGame)
{
    COORD shotCoords = (*currentGame).player1.shot.coords;

    //Check aliens
    //Is X-coord within range of current wave Y-coords?
    if(shotCoords.X >= (*currentGame).currentWave.aliens[0][0].coords.X && shotCoords.X <= (*currentGame).currentWave.aliens[0][ALIENCOLS - 1].coords.X + 2){
        //Is Y-coords within range of current wave Y-coords?
        if(shotCoords.Y <= (*currentGame).currentWave.aliens[0][0].coords.Y && shotCoords.Y >= (*currentGame).currentWave.aliens[ALIENROWS - 1][0].coords.Y){
            int i = 0;
            int j = 0;
            //Find indexes of alien being hit
            while(j < ALIENCOLS && ((*currentGame).currentWave.aliens[0][j].coords.X > shotCoords.X || (*currentGame).currentWave.aliens[0][j].coords.X + (*currentGame).currentWave.aliens[i][j].length - 1 < shotCoords.X)){
                j++;
            }
            while(i < ALIENROWS && (*currentGame).currentWave.aliens[i][j].coords.Y != shotCoords.Y){
                i++;
            }
            if(i < ALIENROWS && j < ALIENCOLS && (*currentGame).currentWave.aliens[i][j].alive == TRUE){
                //Kill the alien
                killAlien(&(*currentGame).currentWave.aliens[i][j]);
                //Update which aliens can shoot
                alienShootCheck(&(*currentGame).currentWave);
                //Decrease number of aliens remaining
                (*currentGame).currentWave.numAliensRemaining--;
                //Remove the player's shot
                killPlayerShot(&(*currentGame).player1.shot);
                //Update the score based on what was killed
                updatePlayerScore(&(*currentGame), (*currentGame).currentWave.aliens[i][j].score);
                //Update the speed
                updateSpeed(&(*currentGame));
            }
        }
    }

    //Check UFO
    if((*currentGame).ufo1.alive == TRUE && (*currentGame).ufo1.coords.Y == shotCoords.Y){
        if((*currentGame).ufo1.coords.X <= shotCoords.X && (*currentGame).ufo1.coords.X + (*currentGame).ufo1.length - 1 >= shotCoords.X){
            killUFO(&(*currentGame).ufo1);
            killPlayerShot(&(*currentGame).player1.shot);
            updatePlayerScore(&(*currentGame), (*currentGame).ufo1.score);
        }
    }

    //Check barricades
    if(hitBarricade((*currentGame).barricades, shotCoords) == TRUE){
        killPlayerShot(&(*currentGame).player1.shot);
    }
}

//Check if the player or barricades have been hit
void killThingsWithBomb(struct game *currentGame)
{
    COORD bombCoords;
    int i, j;
    for(i = 0; i < ALIENROWS; i++){
        for(j = 0; j < ALIENCOLS; j++){
            bombCoords = (*currentGame).currentWave.aliens[i][j].bomb.coords;

            //Check if player hit
            if(bombCoords.Y == (*currentGame).player1.coords.Y && (*currentGame).currentWave.aliens[i][j].bomb.alive == TRUE){
                COORD playerCoords = (*currentGame).player1.coords;
                if(bombCoords.X >= playerCoords.X && bombCoords.X <= playerCoords.X + (*currentGame).player1.length - 1){
                    //Remove the bomb
                    killAlienBomb(&(*currentGame).currentWave.aliens[i][j].bomb);
                    //Kill the player
                    killPlayer(&(*currentGame));
                }
            }

            //Check if barricade hit
            if((*currentGame).currentWave.aliens[i][j].bomb.alive == TRUE && hitBarricade((*currentGame).barricades, bombCoords) == TRUE){
                killAlienBomb(&(*currentGame).currentWave.aliens[i][j].bomb);
            }
        }
    }
}

//Check collision with barricade
int hitBarricade(struct barricade barricades[4], COORD shotCoords)
{
    int i, j, k;
    k = 0;
    if(shotCoords.Y >= barricades[0].coords.Y - 2 && shotCoords.Y <= barricades[0].coords.Y){
        if(shotCoords.X >= barricades[0].coords.X && shotCoords.X <= barricades[3].coords.X + 3){
            //Get index of the hit barricade
            while(shotCoords.X > barricades[k].coords.X + 3){
                k++;
            }
            for(i = 0; i < 3; i++){
                for(j = 0; j < 4; j++){
                    //Get indexes of the block hit
                    if(barricades[k].blocks[i][j].coords.X == shotCoords.X && barricades[k].blocks[i][j].coords.Y == shotCoords.Y && barricades[k].blocks[i][j].alive == TRUE)
                    {
                        //Update the block
                        hitBarricadeBlock(&barricades[k].blocks[i][j]);
                        return TRUE;
                    }
                }
            }
        }
    }
    return FALSE;
}

//Update barricade block graphic when hit
void hitBarricadeBlock(struct barricadeBlock *block)
{
    //Advance the graphic state
    (*block).stage++;

    //Kill the block is stage is >= 4
    if((*block).stage >= 4){
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleCursorPosition(hConsole, (*block).coords);
        printf("%s", (*block).colour);
        printf("%c", (*block).stageDisplay[(*block).stage]);
        printf(COLOUR_RESET);
        (*block).alive = FALSE;
    }
}

//Create barricade block
struct barricadeBlock createBarricadeBlock(COORD coords)
{
    struct barricadeBlock block;

    block.alive = TRUE;
    block.coords = coords;
    block.stage = 0;
    block.stageDisplay[0] = '#';
    block.stageDisplay[1] = '%';
    block.stageDisplay[2] = '+';
    block.stageDisplay[3] = '*';
    block.stageDisplay[4] = ' ';
    strcpy(block.colour, COLOUR_GREEN);

    return block;
};

//Create a barricade
struct barricade createBarricade(COORD coords)
{
    struct barricade newBarricade;

    newBarricade.coords = coords;
    int i, j;
    for(i = 0; i < 3; i++){
        for(j = 0; j < 4; j++){
            newBarricade.blocks[i][j] = createBarricadeBlock((COORD){newBarricade.coords.X + j, newBarricade.coords.Y - i});
            if(i == 0 && (j == 1 || j == 2)){
                //Middle-bottom two blocks "don't exist"
                newBarricade.blocks[i][j].alive = FALSE;
                newBarricade.blocks[i][j].stage = 4;
            }
        }
    }

    return newBarricade;
};

//Create instance of alien worth 10
struct alien createAlien10(COORD coords)
{
    struct alien alien10;

    alien10.alive = TRUE;
    alien10.score = 10;
    alien10.length = 3;
    alien10.graphicState = 0;
    strcpy(alien10.colour, COLOUR_CYAN);
    strcpy(alien10.graphics[0], ">o<");
    strcpy(alien10.graphics[1], "<o>");
    alien10.coords = coords;
    alien10.prevCoords = coords;
    alien10.graphicMultiplier = 0;
    alien10.graphicMultiplierMax = MULTIPLIER_ALIEN;
    alien10.graphicNeedsRemoving = FALSE;
    alien10.bomb = createAlienBomb();

    return alien10;

};

//Create instance of alien worth 20
struct alien createAlien20(COORD coords)
{
    struct alien alien20;

    alien20.alive = TRUE;
    alien20.score = 20;
    alien20.length = 3;
    alien20.graphicState = 0;
    strcpy(alien20.colour, COLOUR_MAGENTA);
    strcpy(alien20.graphics[0], "-o-");
    strcpy(alien20.graphics[1], "\\o/");
    alien20.coords = coords;
    alien20.prevCoords = coords;
    alien20.graphicMultiplier = 0;
    alien20.graphicMultiplierMax = MULTIPLIER_ALIEN;
    alien20.graphicNeedsRemoving = FALSE;
    alien20.bomb = createAlienBomb();

    return alien20;

};

//Create instance of alien worth 40
struct alien createAlien40(COORD coords)
{
    struct alien alien40;

    alien40.alive = TRUE;
    alien40.score = 40;
    alien40.length = 3;
    alien40.graphicState = 0;
    strcpy(alien40.colour, COLOUR_YELLOW);
    strcpy(alien40.graphics[0], ")o(");
    strcpy(alien40.graphics[1], "(o)");
    alien40.coords = coords;
    alien40.prevCoords = coords;
    alien40.graphicMultiplier = 0;
    alien40.graphicMultiplierMax = MULTIPLIER_ALIEN;
    alien40.graphicNeedsRemoving = FALSE;
    alien40.bomb = createAlienBomb();

    return alien40;

};

//Create instance of alien bomb
struct alienBomb createAlienBomb()
{
    struct alienBomb bomb;

    bomb.alive = FALSE;
    bomb.graphic = 'Y';

    return bomb;
};

//Alien drops a bomb
void alienShoot(struct alien *alien1)
{
    (*alien1).bomb.alive = TRUE;
    (*alien1).bomb.coords.X = (*alien1).coords.X + 1;
    (*alien1).bomb.coords.Y = (*alien1).coords.Y + 1;
}

//Keep the bomb moving
void updateAlienBomb(struct alienBomb *bomb)
{
    (*bomb).prevCoords = (*bomb).coords;
    if((*bomb).alive == TRUE){
        //Move down by one
        (*bomb).coords.Y++;
    }
    if((*bomb).coords.Y > BOTTOMBORDER){
        //Remove bomb if it reaches the bottom
        killAlienBomb(&(*bomb));
    }
}

//Create instance of ufo
struct ufo createUFO(int direction)
{
    struct ufo ufo1;

    ufo1.alive = TRUE; //UFO starts not alive, becomes alive when needed to appear
    ufo1.length = 5;
    ufo1.score = 300;
    ufo1.graphicState = 0;
    strcpy(ufo1.colour, COLOUR_RED);
    strcpy(ufo1.graphics[0], "<-o->");
    strcpy(ufo1.graphics[1], "<-o->");
    strcpy(ufo1.graphics[2], "<o-o>");
    strcpy(ufo1.graphics[3], "<o-o>");

    if(direction == 0){
        ufo1.coords = (COORD){LEFTBORDER - ufo1.length + 1, 4};
    }
    else{
        ufo1.coords = (COORD){RIGHTBORDER, 4};
    }

    ufo1.graphicMultiplier = 0;
    ufo1.graphicMultiplierMax = MULTIPLIER_UFO;

    return ufo1;

};

//Spawn a ufo
void spawnUFO(struct ufo *ufo1)
{
    (*ufo1).alive = TRUE;
    //Random starting direction
    (*ufo1).direction = rand() % 2;

    //Adjust spawn coords based on direction
    if((*ufo1).direction == 0){
        (*ufo1).coords = (COORD){LEFTBORDER - (*ufo1).length + 1, 4};
    }
    else{
        (*ufo1).coords = (COORD){RIGHTBORDER, 4};
    }

    //Score is random multiple of 50 between 50 and 300
    (*ufo1).score = (int)((rand() % 6) + 1) * 50;
}

//Display a barricade
void displayBarricade(HANDLE hConsole, struct barricade barricade1)
{
    int i, j;
    for(i = 0; i < 3; i++){
        for(j = 0; j < 4; j++){
            if(barricade1.blocks[i][j].alive == TRUE){
                SetConsoleCursorPosition(hConsole, barricade1.blocks[i][j].coords);
                printf(barricade1.blocks[i][j].colour);
                printf("%c", barricade1.blocks[i][j].stageDisplay[barricade1.blocks[i][j].stage]);
                printf(COLOUR_RESET);
            }
        }
    }
}

//Display the player
void displayPlayer(HANDLE hConsole, struct player *player1)
{
    //Remove old graphic
    if((*player1).coords.X > (*player1).prevCoords.X){
        SetConsoleCursorPosition(hConsole, (*player1).prevCoords);
        printf("%c", ' ');
    }
    else if((*player1).coords.X < (*player1).prevCoords.X){
        SetConsoleCursorPosition(hConsole, (COORD){(*player1).prevCoords.X + (*player1).length - 1, (*player1).prevCoords.Y});
        printf("%c", ' ');
    }

    //Print new graphic
    SetConsoleCursorPosition(hConsole, (*player1).coords);
    printf((*player1).colour);
    if((*player1).graphicState == 0){
        printf("%s", (*player1).graphicNormal);
    }
    else if((*player1).graphicState == -1){
        printf("%s", (*player1).graphicDead);
        (*player1).graphicState = 0;
    }
    else{
        (*player1).graphicState++;
        (*player1).graphicState %= 3;
        printf("%s", (*player1).graphicShoot);
    }
    printf(COLOUR_RESET);

    //Display the player's shot
    if((*player1).shot.alive == TRUE){
        displayPlayerShot(hConsole, (*player1).shot);    //Display the player's shot
    }
}

//Lose a life
void killPlayer(struct game *currentGame)
{
    //Stop music
    PlaySound(NULL, NULL, SND_ASYNC);

    //Play explosion sound if not on mute
    if((*currentGame).musicBox.mute == FALSE){
        PlaySound((DEATH_SOUND), NULL, SND_FILENAME | SND_ASYNC);
    }

    //Decrease lives
    (*currentGame).player1.lives--;
    //Set to death graphic
    (*currentGame).player1.graphicState = -1;
    //Kill UFO if alive
    if((*currentGame).ufo1.alive == TRUE){
        killUFO(&(*currentGame).ufo1);
    }
    //Kill player's shot if alive
    if((*currentGame).player1.shot.alive == TRUE){
        killPlayerShot(&(*currentGame).player1.shot);
    }
    //Remove all alien bombs
    removeAllBombs(&(*currentGame).currentWave);

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    displayPlayer(hConsole, &(*currentGame).player1);
    displayLives(hConsole, (*currentGame));
    //Wait 5 seconds
    sleep(5);
    //Play music again if not muted
    if((*currentGame).musicBox.mute == FALSE){
        playSound(MUSIC_SOUND);
    }
}

//Remove any active alien bombs
void removeAllBombs(struct wave *currentWave)
{
    int i, j;
    for(i = 0; i < ALIENROWS; i++){
        for(j = 0; j < ALIENCOLS; j++){
            if((*currentWave).aliens[i][j].bomb.alive == TRUE){
                //Kill bomb if alive
                killAlienBomb(&(*currentWave).aliens[i][j].bomb);
            }
        }
    }
}

//Display the player's shot
void displayPlayerShot(HANDLE hConsole, struct playerShot shot)
{
    if(shot.alive == FALSE){
        return;
    }

    SetConsoleCursorPosition(hConsole, shot.coords);
    printf(shot.colour);
    printf("%c", shot.graphic);
    printf(COLOUR_RESET);
    if(coordsWithinBorders(shot.prevCoords)){
        SetConsoleCursorPosition(hConsole, shot.prevCoords);
        printf("%c", ' ');
    }
}

//Kill player shot
void killPlayerShot(struct playerShot *shot)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    (*shot).alive = FALSE;
    if(coordsWithinBorders((*shot).coords)){
        SetConsoleCursorPosition(hConsole, (*shot).coords);
        printf("%c", ' ');
    }

    if(coordsWithinBorders((*shot).prevCoords)){
        SetConsoleCursorPosition(hConsole, (*shot).prevCoords);
        printf("%c", ' ');
    }

    (*shot).prevCoords = (*shot).coords;
}

//Display an alien
void displayAlien(HANDLE hConsole, struct alien *alien1)
{
    //Do not draw if alien destroyed
    if(!(*alien1).alive){
        return;
    }

    (*alien1).graphicMultiplier++;  //Increase graphic counter

    //Only update graphic if graphic counter at max
    if((*alien1).graphicMultiplier != (*alien1).graphicMultiplierMax){
        return;
    }

    (*alien1).graphicMultiplier = 0;    //Reset graphic counter

    SetConsoleCursorPosition(hConsole, (*alien1).coords);

    int i;
    printf((*alien1).colour);   //Set colour
    for(i = 0; i < (*alien1).length; i++){
        printf("%c", (*alien1).graphics[(*alien1).graphicState][i]);
    }
    printf(COLOUR_RESET);

    (*alien1).graphicState = ((*alien1).graphicState + 1) % 2;    //Change graphic for next time

    //Remove old graphics
    if((*alien1).coords.X > (*alien1).prevCoords.X){
        SetConsoleCursorPosition(hConsole, (*alien1).prevCoords);
        printf("%c", ' ');
    }
    else if((*alien1).coords.X < (*alien1).prevCoords.X){
        SetConsoleCursorPosition(hConsole, (COORD){(*alien1).prevCoords.X + (*alien1).length - 1, (*alien1).prevCoords.Y});
        printf("%c", ' ');
    }
    else if((*alien1).coords.X == (*alien1).prevCoords.X && (*alien1).coords.Y > (*alien1).prevCoords.Y && (*alien1).graphicNeedsRemoving == TRUE){
        int i;
        for(i = 0; i < (*alien1).length; i++){
            SetConsoleCursorPosition(hConsole, (COORD){(*alien1).prevCoords.X + i, (*alien1).prevCoords.Y});
            printf("%c", ' ');
        }
    }
}

//Kill alien
void killAlien(struct alien *alien1)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    (*alien1).alive = FALSE;
    if(coordsWithinBorders((*alien1).prevCoords)){
        SetConsoleCursorPosition(hConsole, (*alien1).prevCoords);
        printf("   ");
    }
    if(coordsWithinBorders((*alien1).coords)){
        SetConsoleCursorPosition(hConsole, (*alien1).coords);
        printf("   ");
    }
    (*alien1).prevCoords = (*alien1).coords;
}

//Display an alien bomb
void displayAlienBomb(HANDLE hConsole, struct alienBomb bomb)
{
    if(bomb.alive == FALSE){
        return;
    }

    SetConsoleCursorPosition(hConsole, bomb.coords);
    printf("%c", bomb.graphic);
    if(coordsWithinBorders(bomb.prevCoords)){
        SetConsoleCursorPosition(hConsole, bomb.prevCoords);
        printf("%c", ' ');
    }
}

//Kill an alien bomb
void killAlienBomb(struct alienBomb *bomb)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    (*bomb).alive = FALSE;
    if(coordsWithinBorders((*bomb).prevCoords) == TRUE){
        SetConsoleCursorPosition(hConsole, (*bomb).prevCoords);
        printf("%c", ' ');
    }
    (*bomb).prevCoords = (*bomb).coords;
}

//Display a UFO
void displayUFO(HANDLE hConsole, struct ufo *ufo1)
{
    //Do not draw if UFO destroyed
    if(!(*ufo1).alive){
        return;
    }

    (*ufo1).graphicMultiplier++;

    if((*ufo1).graphicMultiplier != (*ufo1).graphicMultiplierMax){
        return;
    }

    (*ufo1).graphicMultiplier = 0;

    if((*ufo1).coords.X < LEFTBORDER){
        SetConsoleCursorPosition(hConsole, (COORD){LEFTBORDER, (*ufo1).coords.Y});
    }
    else{
        SetConsoleCursorPosition(hConsole, (*ufo1).coords);
    }

    int i;
    printf((*ufo1).colour);
    for(i = 0; i < (*ufo1).length; i++){
        if((*ufo1).coords.X + i >= LEFTBORDER && (*ufo1).coords.X + i < RIGHTBORDER){
            printf("%c", (*ufo1).graphics[(*ufo1).graphicState][i]);
        }
    }
    printf(COLOUR_RESET);

    (*ufo1).graphicState = ((*ufo1).graphicState + 1) % 4;

    if((*ufo1).direction == 0 && (*ufo1).prevCoords.X >= LEFTBORDER && (*ufo1).prevCoords.X < RIGHTBORDER){
        SetConsoleCursorPosition(hConsole, (*ufo1).prevCoords);
        printf("%c", ' ');
    }
    else if((*ufo1).direction == 1){
        SetConsoleCursorPosition(hConsole, (COORD){(*ufo1).prevCoords.X + (*ufo1).length - 1, (*ufo1).prevCoords.Y});
        printf("%c", ' ');
    }
}

//Kill the ufo
void killUFO(struct ufo *ufo1)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    (*ufo1).alive = FALSE;

    if((*ufo1).coords.X < LEFTBORDER && (*ufo1).direction == 1){
        SetConsoleCursorPosition(hConsole, (COORD){LEFTBORDER, (*ufo1).coords.Y});
        printf("     ");
    }
    else{
        SetConsoleCursorPosition(hConsole, (*ufo1).coords);
        printf("     ");
        SetConsoleCursorPosition(hConsole, (*ufo1).prevCoords);
        printf("     ");
    }
}

//Clear console
void clrscn()
{
    system("cls");
}

//Enter pause game state
void pauseGame()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleCursorPosition(hConsole, (COORD){35, BOTTOMBORDER + 2});
    printf(COLOUR_RED);
    printf("P A U S E D");
    printf(COLOUR_RESET);

    //Wait for button to be pressed again
    while(getch() != 'p');

    SetConsoleCursorPosition(hConsole, (COORD){35, BOTTOMBORDER + 2});
    printf("           ");
}

//Toggle music mute
void muteMusicToggle(struct game *currentGame)
{
    //Toggle mute variable
    (*currentGame).musicBox.mute = !(*currentGame).musicBox.mute;

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    if((*currentGame).musicBox.mute == TRUE){
        PlaySound(NULL, NULL, SND_ASYNC);
        SetConsoleCursorPosition(hConsole, (COORD){3, 1});
        printf(COLOUR_RED);
        printf("MUTE");
        printf(COLOUR_RESET);
    }
    else{
        playSound(MUSIC_SOUND);
        SetConsoleCursorPosition(hConsole, (COORD){3, 1});
        printf("    ");
    }
}

//Play a sound given the filename
void playSound(char filename[])
{
    PlaySound((filename), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
}

//Set up the music player
struct music createMusic()
{
    struct music musicBox;

    musicBox.currentNote = 0;
    musicBox.mute = FALSE;

    return musicBox;
};

//Update player's score
void updatePlayerScore(struct game *currentGame, int score)
{
    (*currentGame).gameScore += score;
}

//Display the score
void displayScore(HANDLE hConsole, struct game currentGame)
{
    SetConsoleCursorPosition(hConsole, (COORD){12, BOTTOMBORDER + 2});
    printf("%06d", currentGame.gameScore);
}

//Display how many lives the player has left
void displayLives(HANDLE hConsole, struct game currentGame)
{
    SetConsoleCursorPosition(hConsole, (COORD){76, BOTTOMBORDER + 2});
    printf("%d", currentGame.player1.lives);
}

//Display the current level
void displayLevel(HANDLE hConsole, struct game currentGame)
{
    SetConsoleCursorPosition(hConsole, (COORD){76, 1});
    printf("%d", currentGame.level);
}

//Display end screen
void displayGameOver(HANDLE hConsole, struct game *currentGame)
{
    clrscn();
    displayScreen(GAMEOVER_FILE);
    SetConsoleCursorPosition(hConsole, (COORD){29, 10});
    printf("%s", COLOUR_RED);
    printf("G  A  M  E    O  V  E  R");
    printf(COLOUR_RESET);
    displayPlayer(hConsole, &(*currentGame).player1);
    displayScore(hConsole, (*currentGame));
    displayLives(hConsole, (*currentGame));
    displayLevel(hConsole, (*currentGame));

    //Wait 5 seconds
    sleep(5);
}

//Quick fix for UFO sometimes overwriting border characters
void fixBorders(HANDLE hConsole)
{
    SetConsoleCursorPosition(hConsole, (COORD){0, 4});
    printf("||");
    SetConsoleCursorPosition(hConsole, (COORD){RIGHTBORDER, 4});
    printf("||");
    SetConsoleCursorPosition(hConsole, (COORD){2, BOTTOMBORDER + 1});
    printf("==============================================================================||");
    SetConsoleCursorPosition(hConsole, (COORD){0, 0});
    printf("#================================================================================#");
}

//Check if coords are within borders
int coordsWithinBorders(COORD coords)
{
    if(coords.X >= LEFTBORDER && coords.X <= RIGHTBORDER){
        if(coords.Y >= TOPBORDER && coords.Y <= BOTTOMBORDER){
            return TRUE;
        }
    }
    return FALSE;
}

//Hide the cursor
void hideCursor()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  //Handle for console
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &info);
}
