#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <pthread.h>
#include <omp.h>

int sudoku_array[9][9];
int columns = 0;
int rows = 0;

int check_row(int row) {
  if (row < 0 || 8 < row) { return -1; }
  int i, j;
  int checks[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  printf("El hilo que revisa la fila es: %d\n", syscall(SYS_gettid));

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
  printf("En la revision de columna %d, el siguiente es un thread en ejecucion: %d\n", column, syscall(SYS_gettid));
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
        return -1;
      }
      // No encntro el numero
      if (j == 8) { return -1; }
    }
  }
  return 0;
}

void* check_all_columns(void *arg) {
  printf("El hilo que revisa las columnas es: %d\n", syscall(SYS_gettid));
  int i, check;

  omp_set_num_threads(9);
  omp_set_nested(1);
  #pragma omp parallel for ordered
  for (i = 0; i < 8; i++) {
    check = check_column(i);
    if (check == -1) {
      columns = -1;
    }
  }
  pthread_exit(0);
}

int main(int argc, char** argv) {
  char* sudoku_path = argv[1];
  int i, readFile;
  omp_set_num_threads(4);
  pthread_t thread;

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

  if (fork() == 0) {
    char parent_id[10];
    sprintf(parent_id, "%d", (int)(getppid()));
    execlp("ps", "ps", "-p", parent_id, "-lLf");
  } else {
  
    // Se crea el hilo
    pthread_create(&thread, NULL, check_all_columns, NULL);
    pthread_join(thread, NULL);

    printf("El thread en el que se ejecuta main es: %d\n", syscall(SYS_gettid));

    wait(NULL);
    int check;

    omp_set_num_threads(9);
    omp_set_nested(1);
    #pragma omp parallel for ordered schedule (dynamic)
    for (i = 0; i < 8; i++) { // Verifica las filas el padre
      check = check_row(i);
      if (check == -1) {
        rows = -1;
      }
    }

    // Le indica al usuario cual fue el resultado del sudoku
    if (rows == 0 && columns == 0){
      printf("Sudoku resuelto!\n");
    } else {
      printf("Sudoku invalido\n");
    }

    if (fork() == 0) {
      char parent_id[10];
      sprintf(parent_id, "%d", (int)(getppid()));
      execlp("ps", "ps", "-p", parent_id, "-lLf");
    } else {
      wait(NULL);
    }
  }

  

  pthread_exit(0);
}