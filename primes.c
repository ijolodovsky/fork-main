#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define READ  0
#define WRITE 1

/*
La criba de Eratóstenes (sieve of Eratosthenes en inglés) es un algoritmo milenario para calcular todos los primos menores a un determinado número natural, n.

Si bien la visualización del algoritmo suele hacerse “tachando” en una grilla, el concepto de criba, o sieve (literalmente: cedazo, tamiz, colador) debe hacernos pensar más en un filtro. En particular, puede pensarse en n filtros apilados, donde el primero filtra los enteros múltiplos de 2, el segundo los múltiplos de 3, el tercero los múltiplos de 5, y así sucesivamente.

Si modelásemos cada filtro como un proceso, y la transición de un filtro al siguiente mediante tuberías (pipes), se puede implementar el algoritmo con el siguiente pseudo-código (ver fuente original, y en particular la imagen que allí se muestra):

p := <leer valor de pipe izquierdo>

imprimir p // asumiendo que es primo

mientras <pipe izquierdo no cerrado>:
    n = <leer siguiente valor de pipe izquierdo>
    si n % p != 0:
        escribir <n> en el pipe derecho
(El único proceso que es distinto, es el primero, que tiene que simplemente generar la secuencia de números naturales de 2 a n. No tiene lado izquierdo.)
*/

void filter(int pipe_read){
   int pipe_hijo_a_nieto[2];
    pipe(pipe_hijo_a_nieto);

    pid_t pid = fork();

    if (pid == 0) {
        // Proceso hijo (nieto)
        close(pipe_hijo_a_nieto[WRITE]);
        filter(pipe_hijo_a_nieto[READ]);
    } else {
        // Proceso padre
        close(pipe_hijo_a_nieto[READ]);

        int num;
        if (read(pipe_read, &num, sizeof(int)) > 0) {
            int primo = num;
            printf("primo %d\n", primo);

            while (read(pipe_read, &num, sizeof(int)) > 0) {
                if (num % primo != 0) {
                    write(pipe_hijo_a_nieto[WRITE], &num, sizeof(int));
                }
            }
        }

        close(pipe_read);
        close(pipe_hijo_a_nieto[WRITE]);
        wait(NULL);
        exit(0);
    }
}

int main(int argc, char *argv[])
{
	if(argc!=2){
		fprintf(stderr, "Modo de uso: %s <n>\n", argv[0]);
		return 1;
	}

	int n = atoi(argv[1]);
	if(n<2){
		fprintf(stderr, "n debe ser mayor o igual a 2.\n");
		return 1;
	}

	int pipe_padre_a_hijo[2];
	pipe(pipe_padre_a_hijo);

	pid_t pid = fork();

	if(pid==0){
		close(pipe_padre_a_hijo[WRITE]);
		filter(pipe_padre_a_hijo[READ]);
	} else{
		close(pipe_padre_a_hijo[READ]);

		for(int i=2; i<=n; i++){
			write(pipe_padre_a_hijo[WRITE], &i, sizeof(int));
		}
		close(pipe_padre_a_hijo[WRITE]);

		wait(NULL);
	}

	return 0;
}