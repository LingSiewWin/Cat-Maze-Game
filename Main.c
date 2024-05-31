#include <stdio.h>
#include <stdlib.h>
#include <conio.h>   
#include <unistd.h> 
#include <time.h>    
#include <windows.h> 

/*If compiler is vscode needs to include windows.h as library*/

#define wall '#'
#define path ' '
#define player '0'
#define exit 'x'
#define energy '+'
#define sol 'O'

//test if game operating
int isRunning = 0;
int isPaused = 1; // controls timer and UI
char buffer;

// player variables
float playerEnergy;
float energyDecline = 0.7;
int declineInterval = 2;
int playerPoints;

//energy variables
int energySpacing = 11;
float energyValue = 2;

int detailedGeneration = 0;
int showSolutions = 0;

// generation parameters
int vertexDimension;
int maxSize;
int dim;

// positional struct
struct pos
{
    int x;
    int y;
};
int t;

//*2D dynamic array
struct pos *solution;
char **maze;

// User Interface
void startMenu();

void printGame();
void endGame();

// others
void freeMemory();
void getSolution(struct pos c[]);

// To generate maze
void generateMaze(int dimension);
int handleGame();
const char *getDifficulty(int difficulty);
void printMaze();
void printStack(struct pos c[]);

void connectVertex(int x, int y, int nx, int ny);
int validateVertex(int x, int y);
int randVertex(int dir[]);
int randRange(int min, int max);

// DFS Stack Controls
void push(struct pos c[], int x, int y);
void pop(struct pos c[]);

// scoreFile
void writeScoreFile(int difficulty);
void printScoreFile();
void printResult(char *cName, int cPoints, char *cDifficultyLevel);
void printWithSpaces(char *str, int width);
FILE *scoreFile;

// Player Controls
void runGame(int difficulty);
DWORD WINAPI runTimer(LPVOID lpParam);
HANDLE timerThread;

// Main
void main()
{
    startMenu();
    CloseHandle(timerThread);
}

void startMenu()
{
    // start timer thread
    HANDLE timerThread = CreateThread(NULL, 0, runTimer, NULL, 0, NULL);
    char startSelection = 0;
    while (startSelection != '4')
    {
        isPaused = 1;
        system("cls");
        printf("              ,-.       _,---._ __  / \\\n");
        printf("             /  )    .-'       `./ /   \\\n");
        printf("            (  (   ,'            `/    /|\n");
        printf("             \\  `-\"             \\'\\   / |\n");
        printf("              `.              ,  \\ \\ /  |\n");
        printf("               /`.          ,'-`----Y   |\n");
        printf("              (            ;        |   '\n");
        printf("              |  ,-.    ,-'         |  /\n");
        printf("              |  | (   |            | /\n");
        printf("              )  |  \\  `.___________|/\n");
        printf("              `--'   `--'\n");
        printf("-------------------------------\n");
        printf("|  Welcome to the Maze Game!   |\n");
        printf("-------------------------------\n");
        printf("|    1. Start New Game         |\n");
        printf("-------------------------------\n");
        printf("|     2. View Scores           |\n");
        printf("-------------------------------\n");
        printf("|       3. Exit                |\n");
        printf("-------------------------------\n");
        printf("Enter your choice: ");

        startSelection = getch();
        system("cls");

        switch (startSelection)
        {
        case '1':
        {
            int difficulty = handleGame();
            isRunning = 1;
            playerEnergy = 10.0;
            runGame(difficulty);
            freeMemory();

            break;
        }
        case '2':
            printScoreFile();
            printf("\nPress Any To Continue...\n");
            buffer = getch();
            break;
        case '3':
            printf("Process Terminated");
            break;
        default:
            printf("Invalid option entered. Please enter a valid number (1, 2, or 3).\n");
            printf("Press Any Key To Continue...\n");
            getch();
            break;
        }
    }
}

int handleGame()
{
    system("cls");

    int difficulty;

    printf("Here's the maze legend:\n");
    printf("Select the difficulty level:\n");
    printf("1. Easy\n");
    printf("2. Normal\n");
    printf("3. Hard\n");
    printf("4. Custom Difficulty\n");
    printf("Please nter your choice: ");
    scanf("%d", &difficulty);

    switch (difficulty)
    {
    case 1:
        generateMaze(5);
        break;
    case 2:
        generateMaze(10);
        break;
    case 3:
        generateMaze(12);
        break;
    case 4:
    {
        int customSize;
        printf("Enter custom size for maze: ");
        scanf("%d", &customSize);
        generateMaze(customSize);
        break;
    }
    default:
        printf("Invalid option entered.\n");
    }
    return difficulty;
}

const char *getDifficulty(int difficulty)
{
    static char difficultyLevel[20]; // static buffer to hold the string

    switch (difficulty)
    {
    case 1:
        strcpy(difficultyLevel, "Easy");
        break;
    case 2:
        strcpy(difficultyLevel, "Medium");
        break;
    case 3:
        strcpy(difficultyLevel, "Hard");
        break;
    case 4:
        strcpy(difficultyLevel, "Custom");
        break;
    default:
        strcpy(difficultyLevel, "Invalid");
        break;
    }

    return difficultyLevel;
}
void runGame(int difficulty)
{
    int x, y, nx, ny;

    char input;
    char pauseInput;

    //Start game, each player starts with 10 points
    playerPoints = 10;
    maze[1][1] = player;
    maze[dim - 2][dim - 2] = exit;
    x = nx = 1;
    y = ny = 1;

    system("cls");
    printMaze();

    while (isRunning == 1)
    {
        printGame();
        isPaused = 0;

        input = getch();
        switch (input)
        {
        case 'w':
            ny--;
            break;
        case 's':
            ny++;
            break;
        case 'a':
            nx--;
            break;
        case 'd':
            nx++;
            break;
        case 'q':

            while (pauseInput != '1' && pauseInput != pauseInput != '2' && pauseInput != '3')
            {
                system("cls");

                // pause timer thread here
                isPaused = 1;

                printf("Pause Menu\n");
                printf("1. Continue Game\n");
                printf("2. Exit To Menu\n");

                pauseInput = getch();
                system("cls");

                switch (pauseInput)
                {
                case '1':
                    isPaused = 0;
                    break;
                case '2':
                    isPaused = 1;
                    return;
                }
            }

            // reset
            pauseInput = 0;
        }

        // check for ending
        if (maze[ny][nx] == exit)
        {
            // win condition
            playerPoints += 10; // Award the player with 10 points
            endGame();

            if (playerPoints > 0)
            {
                writeScoreFile(difficulty);
            }
            return;
        }

        // check for energy
        if (maze[ny][nx] == energy)
        {
            playerEnergy += energyValue;
            playerPoints++;
        }

        // check for obstacles and move
        if (nx >= 0 && ny >= 0 && nx < dim && ny < dim && maze[ny][nx] != wall)
        {
            if (maze[ny][nx] != path) {// If the cell is not a path, deduct 1 point   
                playerPoints--;
                
            }
            maze[y][x] = path;
            maze[ny][nx] = player;
            y = ny;
            x = nx;

            printGame();
        }
        else
        {
        // Player hit the wall, deduct 1 point
        playerPoints--;
        if (playerPoints <= 0)
        {
            endGame();
            printf("You have lost all the points...\n");
            printf("Press Any Key To Return to Menu...\n");
            isRunning = 0;
            getch(); // Wait for any key press to return to menu
            return;
        }
        nx = x;
        ny = y;
}
    }

    //loss condition
    endGame();
    printf("Press Any To Continue...\n");
    buffer = getch();
}

void printGame()
{
    // runs every timer frame and key input
    system("cls");
    printMaze();
    printf("\n");

    printf("Energy : %.1f\n", playerEnergy);
    printf("Points : %d\n", playerPoints);
}

void endGame()
{
    CloseHandle(timerThread);
    system("cls");

    isRunning = 0;
    printf("Your Points : %d\n", playerPoints);
    printf("Your Energy : %.1f\n", playerEnergy);
}

void generateMaze(int dimension)
{
    srand(time(0));

    vertexDimension = dimension;

    // define vertex parameters
    dim = vertexDimension * 2 + 1;
    maxSize = vertexDimension * vertexDimension;

    // allocate memory for stack array
    struct pos *c_stack = (struct pos *)malloc(maxSize * sizeof(struct pos));
    solution = (struct pos *)malloc(maxSize * sizeof(struct pos));
    t = -1;

    // Allocate memory for 2d array in a single block
    maze = (char **)malloc(dim * sizeof(char *));
    maze[0] = (char *)malloc(dim * dim * sizeof(char));
    for (int i = 0; i < dim; i++)
    {
        maze[i] = maze[0] + i * dim;
    }

    // initialize maze
    for (int i = 0; i < dim; i++)
    {
        for (int j = 0; j < dim; j++)
        {
            maze[i][j] = wall;
        }
    }
    for (int i = 0; i < maxSize; i++)
    {
        c_stack[i].x = solution[i].x = -1;
        c_stack[i].y = solution[i].y = -1;
    }

    printf("Maze Processing\n");

    //----DFS Algorithm-----

    struct pos n = {vertexDimension - 1, vertexDimension - 1};
    maze[dim - 2][dim - 2] = path;

    int nVisited = 1;
    int nIndex = -1;
    int zCount;

    int energyStep = energySpacing;

    int hasSolution = 0;

    while (nVisited < maxSize)
    {
        //UP DOWN LEFT RIGHT
        int dir[4] = {0, 0, 0, 0};
        struct pos step[4] = {
            {n.x, n.y + 1},
            {n.x, n.y - 1},
            {n.x - 1, n.y},
            {n.x + 1, n.y},
        };

        // directional validation
        zCount = 0;
        for (int i = 0; i < 4; i++)
        {
            if (validateVertex(step[i].x, step[i].y) == 1)
            {
                dir[i] = 1;
            }
            else
            {
                dir[i] = 0;
                zCount++;
            }
        }

        // pick random neighbor
        if (zCount < 4)
        {
            nIndex = randVertex(dir);
            push(c_stack, n.x, n.y);

            connectVertex(n.x, n.y, step[nIndex].x, step[nIndex].y);

            n.x = step[nIndex].x;
            n.y = step[nIndex].y;
            nVisited++;

            maze[n.y * 2 + 1][n.x * 2 + 1] = path;

            system("cls");
            printf("%d/%d Processed\n", nVisited + 1, maxSize);

            if (detailedGeneration == 1)
            {
                printMaze();
                printf("\nStack : \n");
                for (int i = 0; i < t; i++)
                {
                    printf("Vertex %d {%d, %d}\n", i, c_stack[i].x, c_stack[i].y);
                }
                sleep(0.4);
            }
        }
        else
        {
            pop(c_stack);
            n.x = c_stack[t].x;
            n.y = c_stack[t].y;
        }

        //energy
        if (energyStep == 0)
        {
            maze[n.y * 2 + 1][n.x * 2 + 1] = energy;
            energyStep = randRange(energySpacing - 3, energySpacing + 3);
        }
        energyStep--;

        // get solution
        if (n.x == 0 && n.y == 0 && hasSolution == 0 && showSolutions == 1)
        {
            getSolution(c_stack);
            hasSolution = 1;
        }
    }

    printf("Solution [%d]\n", hasSolution);
    printf("Processing Completed\n");
}

void getSolution(struct pos c[])
{
    if (showSolutions == 1)
    {
        for (int i = 0; i < t; i++)
        {
            maze[c[i].y * 2 + 1][c[i].x * 2 + 1] = sol;
        }
    }
    printf("Solution Found\n");
    sleep(2);
}

int randVertex(int dir[])
{
    int excluded[4] = {-1, -1, -1, -1};
    int index;

    // list index of zeroes into excluded[]
    int i = 0;
    for (int j = 0; j < 4; j++)
    {
        if (dir[j] == 0)
        {
            excluded[i] = j;
            i++;
        }
    }
    // random index
    do
    {
        index = randRange(0, 3);
    } while (index == excluded[0] || index == excluded[1] || index == excluded[2] || index == excluded[3]);
    return index;
}

int randRange(int min, int max)
{
    return (rand() % (max - min + 1)) + min;
}

int validateVertex(int x, int y)
{
    if (x < 0 || y < 0 || x >= vertexDimension || y >= vertexDimension || maze[y * 2 + 1][x * 2 + 1] != wall)
    {
        return 0;
    }
    return 1;
}

void connectVertex(int x, int y, int nx, int ny)
{
    int mx = ((x * 2 + 1) + (nx * 2 + 1)) / 2;
    int my = ((y * 2 + 1) + (ny * 2 + 1)) / 2;
    maze[my][mx] = path;
}

void push(struct pos c[], int x, int y)
{
    if (t >= maxSize)
    {
        printf("Stack Overflow\n");
        return;
    }
    t++;
    c[t].x = x;
    c[t].y = y;
}

void pop(struct pos c[])
{
    if (t < 0)
    {
        printf("Stack Underflow\n");
        return;
    }
    c[t].x = -1;
    c[t].y = -1;
    t--;
}

void printMaze()
{
    printf("\n");
    for (int i = 0; i < dim; i++)
    {
        for (int j = 0; j < dim; j++)
        {
            char symbol = maze[i][j];
            if (symbol == player)
            {
                SetConsoleOutputCP(CP_UTF8);
                printf("%s", "🐈");
            }
            else if (symbol == exit)
            {
                printf("%s", "🐟");
            }
            else if (symbol == energy)
            {
                printf("%s", "🗲");
            }
            else
            {
                printf("%c ", symbol);
            }
        }
        printf("\n");
    }
}


void writeScoreFile(int difficulty)
{

    scoreFile = fopen("scores.txt", "a");
    char name[20];

    getchar();
    printf("\nEnter Name : ");
    fgets(name, 20, stdin);

    // remove newline character
    int len = strlen(name);
    if (name[len - 1] == '\n')
    {
        name[len - 1] = '\0';
    }

    const char *difficultyLevel = getDifficulty(difficulty);

    fprintf(scoreFile, "%s,%d,%s\n", name, playerPoints, difficultyLevel);
    printf("Score Saved\n");
    fclose(scoreFile);
    sleep(2);
}

void printWithSpaces(char *string, int width)
{
    int len = strlen(string);
    for (int i = 0; i < (((width - len) / 2) + (len % 2)); i++)
    {
        printf(" ");
    }
    printf("%s", string);
    for (int i = 0; i < ((width - len) / 2); i++)
    {
        printf(" ");
    }
}

void printResult(char *cName, int cPoints, char *cDifficultyLevel)
{   
    printf("------------------------------------\n");
    printf("|");
    printWithSpaces(cName, 20);
    printf("| ");
    printf("%2d", cPoints);
    printf(" |");
    printWithSpaces(cDifficultyLevel, 8);
    printf("|\n");
    printf("------------------------------------\n");
}

void printScoreFile()
{

    char cName[20];
    int cPoints;
    char cDifficultyLevel[20];

    scoreFile = fopen("scores.txt", "r");

    // make into table form
    printf("Scores : \n\n");
    while (fscanf(scoreFile, "%20[^,],%d,%20[^\n]\n", cName, &cPoints, cDifficultyLevel) == 3)
    {
        printResult(cName, cPoints, cDifficultyLevel);
    }

    fclose(scoreFile);
}

void freeMemory()
{
    free(maze[0]);
    free(maze);
}

// timer thread
DWORD WINAPI runTimer(LPVOID lpParam)
{
    while (1 < 2)
    {
        sleep(declineInterval);

        if (isPaused == 1 || isRunning == 0)
        {
            continue;
        }
        if (playerEnergy <= 0)
        {
            printf("Time's up! Your energy has run out.\n");
            isRunning = 0;
            continue;
        }

        playerEnergy -= energyDecline;
        printGame();
    }
}