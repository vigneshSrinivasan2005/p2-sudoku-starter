// Sudoku puzzle verifier and solver

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
typedef struct {
    int psize;
    int **grid;
    bool *valid;
} pthreadArgs;

void* checkRows(void* arg);
void* checkCols(void* arg);
void* checkBoxes(void* arg);
void* checkComplete(void* arg);

void* checkComplete(void* arg) {
  pthreadArgs *tp = (pthreadArgs *)arg;
  int psize = tp->psize;
  int **grid = tp->grid;
  bool *complete = tp->valid; //use valid pointer to indicate completeness

  for(int row=1; row<psize+1;row++){
      for(int col=1; col<psize+1;col++){
        if(grid[row][col] == 0){
          *complete = false;
          return NULL;
        }
      }
  }
  return NULL;
}

void* checkBoxes(void* arg){
  pthreadArgs *tp = (pthreadArgs *)arg;
  int psize = tp->psize;
  int **grid = tp->grid;
  bool *valid = tp->valid;

  int boxSize = (int)sqrt(psize);
  int numBoxesPerRow = psize / boxSize;
  int numBoxesPerCol = psize / boxSize;

  for(int i=1; i<=numBoxesPerRow;i++){
    for(int j=1;j<=numBoxesPerCol;j++){
      //for each box
      bool *seen = (bool *)malloc((psize+1) * sizeof(bool));
      bzero(seen, (psize+1)*sizeof(bool));
      for(int row= (i-1)*boxSize +1; row<= i*boxSize; row++){
        for(int col= (j-1)*boxSize +1; col<= j*boxSize; col++){
          if(grid[row][col] !=0){
            if(seen[grid[row][col]]){
              *valid = false;
              return NULL;
            }
            else{
              seen[grid[row][col]] = true;
            }
          }
        }
      }
      free(seen);
    }
  }
  return NULL;
}

void* checkCols(void* arg) {
  pthreadArgs *tp = (pthreadArgs *)arg;
  int psize = tp->psize;
  int **grid = tp->grid;
  bool *valid = tp->valid;

  //now we loop through each row checking if each number appears once
  //use a boolean array to track seen numbers
  for(int col=1; col<psize+1;col++){
      bool *seen = (bool *)malloc((psize+1) * sizeof(bool));
      bzero(seen, (psize+1)*sizeof(bool));
      for(int row=1; row<psize+1;row++){
        if(grid[row][col] != 0){
          //if already seen
          if(seen[grid[row][col]]){
            *valid = false;
            return NULL;
          }
          else{
            seen[grid[row][col]] = true;
          }
        }
      }
      free(seen);
  }
  return NULL;
}

void* checkRows(void* arg) {
  pthreadArgs *tp = (pthreadArgs *)arg;
  int psize = tp->psize;
  int **grid = tp->grid;
  bool *valid = tp->valid;

  //now we loop through each row checking if each number appears once
  //use a boolean array to track seen numbers
  for(int row=1; row<psize+1;row++){
      bool *seen = (bool *)malloc((psize+1) * sizeof(bool));
      bzero(seen, (psize+1)*sizeof(bool));
      for(int col=1; col<psize+1;col++){
        if(grid[row][col] != 0){
          //if already seen
          if(seen[grid[row][col]]){
            *valid = false;
            return NULL;
          }
          else{
            seen[grid[row][col]] = true;
          }
        }
      }
      free(seen);
  }


  return NULL;
}
// takes puzzle size and grid[][] representing sudoku puzzle
// and tow booleans to be assigned: complete and valid.
// row-0 and column-0 is ignored for convenience, so a 9x9 puzzle
// has grid[1][1] as the top-left element and grid[9]9] as bottom right
// A puzzle is complete if it can be completed with no 0s in it
// If complete, a puzzle is valid if all rows/columns/boxes have numbers from 1
// to psize For incomplete puzzles, we cannot say anything about validity
void checkPuzzle(int psize, int **grid, bool *complete, bool *valid) {
  // YOUR CODE GOES HERE and in HELPER FUNCTIONS
  
  *valid = true;
  *complete = true;
  //assume valid and set to invalid in the threads
  pthread_t rowThread, colThread, boxThread, completeThread;
  // allocate separate argument structs for each thread so each thread
  // receives a stable pointer (avoid passing address of a single stack
  // local that might be reused/modified)
  pthreadArgs *argsRow = (pthreadArgs *)malloc(sizeof(pthreadArgs));
  pthreadArgs *argsCol = (pthreadArgs *)malloc(sizeof(pthreadArgs));
  pthreadArgs *argsBox = (pthreadArgs *)malloc(sizeof(pthreadArgs));
  pthreadArgs *argsComplete = (pthreadArgs *)malloc(sizeof(pthreadArgs));

  argsRow->psize = psize;
  argsRow->grid = grid;
  argsRow->valid = valid;

  argsCol->psize = psize;
  argsCol->grid = grid;
  argsCol->valid = valid;

  argsBox->psize = psize;
  argsBox->grid = grid;
  argsBox->valid = valid;

  argsComplete->psize = psize;
  argsComplete->grid = grid;
  argsComplete->valid = complete; // indicate completeness via this pointer

  pthread_create(&rowThread, NULL, checkRows, (void*)argsRow);
  pthread_create(&colThread, NULL, checkCols, (void*)argsCol);
  pthread_create(&boxThread, NULL, checkBoxes, (void*)argsBox);
  pthread_create(&completeThread, NULL, checkComplete, (void*)argsComplete);

  pthread_join(rowThread, NULL);
  pthread_join(colThread, NULL);
  pthread_join(boxThread, NULL);
  pthread_join(completeThread, NULL);

  free(argsRow);
  free(argsCol);
  free(argsBox);
  free(argsComplete);
}

// takes filename and pointer to grid[][]
// returns size of Sudoku puzzle and fills grid
int readSudokuPuzzle(char *filename, int ***grid) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Could not open file %s\n", filename);
    exit(EXIT_FAILURE);
  }
  int psize;
  fscanf(fp, "%d", &psize);
  int **agrid = (int **)malloc((psize + 1) * sizeof(int *));
  for (int row = 1; row <= psize; row++) {
    agrid[row] = (int *)malloc((psize + 1) * sizeof(int));
    for (int col = 1; col <= psize; col++) {
      fscanf(fp, "%d", &agrid[row][col]);
    }
  }
  fclose(fp);
  *grid = agrid;
  return psize;
}

// takes puzzle size and grid[][]
// prints the puzzle
void printSudokuPuzzle(int psize, int **grid) {
  printf("%d\n", psize);
  for (int row = 1; row <= psize; row++) {
    for (int col = 1; col <= psize; col++) {
      printf("%d ", grid[row][col]);
    }
    printf("\n");
  }
  printf("\n");
}

// takes puzzle size and grid[][]
// frees the memory allocated
void deleteSudokuPuzzle(int psize, int **grid) {
  for (int row = 1; row <= psize; row++) {
    free(grid[row]);
  }
  free(grid);
}

// expects file name of the puzzle as argument in command line
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: ./sudoku puzzle.txt\n");
    return EXIT_FAILURE;
  }
  // grid is a 2D array
  int **grid = NULL;
  // find grid size and fill grid
  int sudokuSize = readSudokuPuzzle(argv[1], &grid);
  bool valid = false;
  bool complete = false;
  checkPuzzle(sudokuSize, grid, &complete, &valid);
  printf("Complete puzzle? ");
  printf(complete ? "true\n" : "false\n");
  if (complete) {
    printf("Valid puzzle? ");
    printf(valid ? "true\n" : "false\n");
  }
  printSudokuPuzzle(sudokuSize, grid);
  deleteSudokuPuzzle(sudokuSize, grid);
  return EXIT_SUCCESS;
}
