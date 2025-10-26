// Sudoku puzzle verifier and solver

#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int psize;
  int **grid;
  bool *valid;
} pthreadArgs;

void *checkRows(void *arg);
void *checkCols(void *arg);
void *checkBoxes(void *arg);
void *checkComplete(void *arg);

void *checkComplete(void *arg) {
  pthreadArgs *tp = (pthreadArgs *)arg;
  int psize = tp->psize;
  int **grid = tp->grid;
  bool *complete = tp->valid; // use valid pointer to indicate completeness

  for (int row = 1; row < psize + 1; row++) {
    for (int col = 1; col < psize + 1; col++) {
      if (grid[row][col] == 0) {
        *complete = false;
        return NULL;
      }
    }
  }
  return NULL;
}

void *checkBoxes(void *arg) {
  pthreadArgs *tp = (pthreadArgs *)arg;
  int psize = tp->psize;
  int **grid = tp->grid;
  bool *valid = tp->valid;

  int boxSize = (int)sqrt(psize);
  int numBoxesPerRow = psize / boxSize;
  int numBoxesPerCol = psize / boxSize;

  for (int i = 1; i <= numBoxesPerRow; i++) {
    for (int j = 1; j <= numBoxesPerCol; j++) {
      // for each box
      bool *seen = (bool *)malloc((psize + 1) * sizeof(bool));
      memset(seen, 0, (psize + 1) * sizeof(bool));
      for (int row = (i - 1) * boxSize + 1; row <= i * boxSize; row++) {
        for (int col = (j - 1) * boxSize + 1; col <= j * boxSize; col++) {
          if (grid[row][col] != 0) {
            if (seen[grid[row][col]]) {
              *valid = false;
              free(seen);
              return NULL;
            } else {
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

void *checkCols(void *arg) {
  pthreadArgs *tp = (pthreadArgs *)arg;
  int psize = tp->psize;
  int **grid = tp->grid;
  bool *valid = tp->valid;

  // now we loop through each row checking if each number appears once
  // use a boolean array to track seen numbers
  for (int col = 1; col < psize + 1; col++) {
    bool *seen = (bool *)malloc((psize + 1) * sizeof(bool));
    memset(seen, 0, (psize + 1) * sizeof(bool));
    for (int row = 1; row < psize + 1; row++) {
      if (grid[row][col] != 0) {
        // if already seen
        if (seen[grid[row][col]]) {
          *valid = false;
          free(seen);
          return NULL;
        } else {
          seen[grid[row][col]] = true;
        }
      }
    }
    free(seen);
  }
  return NULL;
}

void *checkRows(void *arg) {
  pthreadArgs *tp = (pthreadArgs *)arg;
  int psize = tp->psize;
  int **grid = tp->grid;
  bool *valid = tp->valid;

  // now we loop through each row checking if each number appears once
  // use a boolean array to track seen numbers
  for (int row = 1; row < psize + 1; row++) {
    bool *seen = (bool *)malloc((psize + 1) * sizeof(bool));
    memset(seen, 0, (psize + 1) * sizeof(bool));
    for (int col = 1; col < psize + 1; col++) {
      if (grid[row][col] != 0) {
        // if already seen
        if (seen[grid[row][col]]) {
          *valid = false;
          free(seen);
          return NULL;
        } else {
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
  // assume valid and set to invalid in the threads
  pthread_t rowThread, colThread, boxThread, completeThread;

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
  argsComplete->valid = complete; // completeness using valid pointer

  pthread_create(&rowThread, NULL, checkRows, (void *)argsRow);
  pthread_create(&colThread, NULL, checkCols, (void *)argsCol);
  pthread_create(&boxThread, NULL, checkBoxes, (void *)argsBox);
  pthread_create(&completeThread, NULL, checkComplete, (void *)argsComplete);

  pthread_join(rowThread, NULL);
  pthread_join(colThread, NULL);
  pthread_join(boxThread, NULL);
  pthread_join(completeThread, NULL);

  free(argsRow);
  free(argsCol);
  free(argsBox);
  free(argsComplete);
}

bool isValidSudoku(int psize, int **grid, int row, int col,
                   int num) { // helper function to check if placing num at
                              // grid[row][col] is valid
  // more effient to only check changed row, column, and box
  bool valid = true;
  // check row
  for (int c = 1; c <= psize; c++) {
    if (grid[row][c] == num) {
      valid = false;
      return valid;
    }
  }
  // check column
  for (int r = 1; r <= psize; r++) {
    if (grid[r][col] == num) {
      valid = false;
      return valid;
    }
  }
  // check box
  int boxSize = (int)sqrt(psize);
  int boxRowStart = ((row - 1) / boxSize) * boxSize + 1;
  int boxColStart = ((col - 1) / boxSize) * boxSize + 1;
  for (int r = boxRowStart; r < boxRowStart + boxSize; r++) {
    for (int c = boxColStart; c < boxColStart + boxSize; c++) {
      if (grid[r][c] == num) {
        valid = false;
        return valid;
      }
    }
  }
  return valid;
}

void copyGrid(int psize, int **source, int **dest) {
  for (int i = 1; i <= psize; i++) {
    for (int j = 1; j <= psize; j++) {
      dest[i][j] = source[i][j];
    }
  }
}

bool isCompleteGrid(int psize, int **grid) {
  for (int i = 1; i <= psize; i++) {
    for (int j = 1; j <= psize; j++) {
      if (grid[i][j] == 0) {
        return false;
      }
    }
  }
  return true;
}
void freeGrid(int psize, int **grid) {
  for (int i = 1; i <= psize; i++) {
    free(grid[i]);
  }
  free(grid);
}
// This struct holds a single element of the stack
struct Node {
  int **data;
  struct Node *next; // Pointer to the next item in the stack
};

// Making a Stack struct to enable backtracking
typedef struct Stack {
  struct Node *top;
} Stack;

void Stack_push(Stack *stack, int **data) {
  struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
  newNode->data = data;
  newNode->next = stack->top;
  stack->top = newNode;
}

int **Stack_pop(Stack *stack) {
  if (stack->top == NULL) {
    return NULL; // Stack is empty
  }
  struct Node *temp = stack->top;
  int **poppedData = temp->data;
  stack->top = stack->top->next;
  free(temp);
  return poppedData;
}

int Stack_free(int psize, Stack *stack) {
  int count = 0;
  while (stack->top != NULL) {
    int **poppedData = Stack_pop(stack);
    freeGrid(psize, poppedData);
    count++;
  }
  return count;
}

int Stack_isEmpty(Stack *stack) { return stack->top == NULL; }

void solveSudokuPuzzle(int psize, int **grid, int **answerGrid) {
  // coding a basic backtracking solver here
  // need to push grid states into a stack to backtrack
  Stack stack;
  stack.top = NULL;
  int **temp = (int **)malloc((psize + 1) * sizeof(int *));
  for (int row = 1; row <= psize; row++) {
    temp[row] = (int *)malloc((psize + 1) * sizeof(int));
  }
  copyGrid(psize, grid, temp);
  Stack_push(&stack, temp); // dont want to free the original grid
  while (!Stack_isEmpty(&stack)) {
    int **currentGrid = Stack_pop(&stack);
    if (isCompleteGrid(psize, currentGrid)) {
      // before pushes check validity
      copyGrid(psize, currentGrid, answerGrid);
      freeGrid(psize, currentGrid);
      break;
    }
    // find first empty cell
    int emptyRow = -1;
    int emptyCol = -1;
    // loop through grid to find empty cell
    for (int i = 1; i <= psize; i++) {
      for (int j = 1; j <= psize; j++) {
        if (currentGrid[i][j] == 0) {
          emptyRow = i;
          emptyCol = j;
          break;
        }
      }
      if (emptyRow != -1) {
        break;
      }
    }

    // we know there is an empty cell because not complete
    // place all possible numbers into empty cell if valid
    for (int num = 1; num <= psize; num++) {
      if (isValidSudoku(psize, currentGrid, emptyRow, emptyCol,
                        num)) { // if valiid after placing num
        // create new grid state
        int **newGrid = (int **)malloc((psize + 1) * sizeof(int *));
        for (int row = 1; row <= psize; row++) {
          newGrid[row] = (int *)malloc((psize + 1) * sizeof(int));
        }
        copyGrid(psize, currentGrid, newGrid);
        newGrid[emptyRow][emptyCol] = num;
        Stack_push(&stack, newGrid);
      }
    }

    freeGrid(psize, currentGrid);
  }

  Stack_free(psize, &stack);
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
  // add section for solving incomplete puzzles
  if (!complete && valid) {
    // solve the puzzle
    printf("Solving puzzle...\n");
    int **answerGrid = (int **)malloc((sudokuSize + 1) * sizeof(int *));
    for (int row = 1; row <= sudokuSize; row++) {
      answerGrid[row] = (int *)malloc((sudokuSize + 1) * sizeof(int));
    }
    solveSudokuPuzzle(sudokuSize, grid, answerGrid);
    printf("Solved puzzle:\n");
    printSudokuPuzzle(sudokuSize, answerGrid);
    deleteSudokuPuzzle(sudokuSize, answerGrid);
  }
  deleteSudokuPuzzle(sudokuSize, grid);
  return EXIT_SUCCESS;
}
