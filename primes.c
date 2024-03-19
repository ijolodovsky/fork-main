#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define READ  0
#define WRITE 1

void filter(int pipe_read) {
    int num;
    int primo;

    // Leer el primer número de la tubería
    if (read(pipe_read, &num, sizeof(int)) > 0) {
        primo = num;
        printf("primo %d\n", primo);
    } else {
        // Si no hay datos disponibles para leer, cerrar la tubería y salir
        close(pipe_read);
        return;
    }

    int pipe_hijo_a_nieto[2];
    pipe(pipe_hijo_a_nieto);

    pid_t pid = fork();

    if (pid == 0) {
        // Proceso hijo (nieto)
        close(pipe_read);
        close(pipe_hijo_a_nieto[WRITE]);
        filter(pipe_hijo_a_nieto[READ]);
        exit(0);
    } else {
        // Proceso padre
        close(pipe_hijo_a_nieto[READ]);

        while (read(pipe_read, &num, sizeof(int)) > 0) {
            if (num % primo != 0) {
                write(pipe_hijo_a_nieto[WRITE], &num, sizeof(int));
            }
        }

        // Cerrar todos los extremos de las tuberías en el proceso padre
        close(pipe_read);
        close(pipe_hijo_a_nieto[WRITE]);
        wait(NULL);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Modo de uso: %s <n>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    if (n < 2) {
        fprintf(stderr, "n debe ser mayor o igual a 2.\n");
        return 1;
    }

    int pipe_padre_a_hijo[2];
    pipe(pipe_padre_a_hijo);

    pid_t pid = fork();

    if (pid == 0) {
        // Proceso hijo
        close(pipe_padre_a_hijo[WRITE]);
        filter(pipe_padre_a_hijo[READ]);
        close(pipe_padre_a_hijo[READ]);
        exit(0);
    } else {
        // Proceso padre
        close(pipe_padre_a_hijo[READ]);

        for (int i = 2; i <= n; i++) {
            write(pipe_padre_a_hijo[WRITE], &i, sizeof(int));
        }

        // Cerrar todos los extremos de las tuberías en el proceso padre
        close(pipe_padre_a_hijo[WRITE]);
        wait(NULL);
    }

    return 0;
}
