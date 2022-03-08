#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int sudoku_array[9][9];

int check_row(int row) {
  if (row < 0 || 8 < row) { return -1; }
  int i, j;
  int checks[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

  for (i = 0; i < 9; i++) {
    // Verifica que este en el arreglo
    for (j = 0; j < 9; j++) {
      // Encontro el numero
      if (sudoku_array[row][i] == checks[j]) {
        checks[j] = -1;
        break;
      }
      // No encntro el numero
      if (j == 8) { return -1; }
    }
  }
  return 0;
}

int check_column(int column) {
  if (column < 0 || 8 < column) { return -1; }
  int i, j;
  int checks[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

  for (i = 0; i < 9; i++) {
    // Verifica que este en el arreglo
    for (j = 0; j < 9; j++) {
      // Encontro el numero
      if (sudoku_array[i][column] == checks[j]) {
        checks[j] = -1;
        break;
      }
      // No encntro el numero
      if (j == 8) { return -1; }
    }
  }
  return 0;
}

int check_group(int row, int column) {
  if (column != 0 && column != 3 && column != 6 ) { return -1; }
  if (row != 0 && row != 3 && row != 6 ) { return -1; }
  int i, j;
  int checks[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

  for (i = 0; i < 9; i++) {
    // Verifica que este en el arreglo
    int x = row + (i / 3);
    int y = column + (i % 3);
    for (j = 0; j < 9; j++) {
      // Encontro el numero
      if (sudoku_array[x][y] == checks[j]) {
        checks[j] = -1;
        break;
      }
      // No encntro el numero
      if (j == 8) { return -1; }
    }
  }
  return 0;
}

int main(int argc, char** argv) {
  char* sudoku_path = argv[1];
  int i, readFile;

  readFile = open(sudoku_path, O_RDONLY, 0666);\
  struct stat sb;
  if (fstat(readFile, &sb) == -1) {
    perror("No se pudo abrir el archivo");
  }

  char* opened_file = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, readFile, 0);
  for (i = 0; i < 81; i++) { // Getting the strings
    int x = i / 9;
    int y = i % 9;
    sudoku_array[x][y] = opened_file[i] - '0';
  }

  return 0;
}