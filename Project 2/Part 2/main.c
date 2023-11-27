
/*
    Manuel Garcia Cotes
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <omp.h>

double **ReservaMatriz(double N);
double *ReservaArray(double N);
void MultiplicaMatrizporVector(double **matriz, double *arrayV, double *arrayR, int N, int adi, int filas_reparto, int np, int myid);
void ImprimirResultadosFichero(char *nombreC, int m, double secM, double secIt, double sec, double *absolut);
void LeeryEscribir(double **matriz, char *fichero, int N);

int main(int argc, char *argv[])
{
    clock_t cl = clock();
    double *array1;
    double *array2;
    double *vector;
    double *arrayAbsolut;
    double **matriz;
    int N = 15000;
    int i, j, myid, np, absoluto;
    int m = atoi(argv[2]);
    char *nombreC = argv[0];
    char *fichero = argv[1];
    int num_hilos = atoi(argv[3]);
    double secMatriz, secIter, secMult, mayor = 0, menor = 0, absolut = 0;

    int filas_reparto = N / num_hilos;
    int adi = N % num_hilos;

    array1 = ReservaArray(N);
    array2 = ReservaArray(N);
    vector = ReservaArray(num_hilos);
    arrayAbsolut = ReservaArray(m);
    matriz = ReservaMatriz(N);

    /*
        Inicializamos el todo el primer vector a 1
    */
    for (i = 0; i < N; i++)
    {
        array1[i] = 1;
        array2[i] = 0;
    }

    /*
        Aqui conseguimos que la diagonal de la matriz sean 1, que la mitad triangular inferior sean positivos y la mitad triangular superior sean negativos.
    */

    LeeryEscribir(matriz, fichero, N);

    /*
        Aqui colocamos las variables de medidcion del tiempo y las fucniones necesarias para hacer las iteraciones y calcular el valor absoluto
        La primera iteracion esta separada ya que no se le tiene que aplicar el valor absoluto a ella.
    */
    clock_t begin = clock();
#pragma omp parallel num_threads(num_hilos) shared(matriz, N, filas_reparto, adi, array1, array2) private(myid, np) default(none)
    {
        // Obtener el id del hilo
        myid = omp_get_thread_num();
        // Obtener el numero de hilos
        np = omp_get_num_threads();
        MultiplicaMatrizporVector(matriz, array1, array2, N, adi, filas_reparto, np, myid);
    }
    clock_t endMult = clock();

    for (i = 0; i < N; i++)
    {
        array1[i] = array2[i];
    }
#pragma omp parallel num_threads(num_hilos) shared(matriz, N, filas_reparto, adi, array1, array2, arrayAbsolut, m, vector, absoluto) private(i, j, mayor, menor, myid, np) default(none)
    {
        // Obtener el id del hilo
        myid = omp_get_thread_num();
        // Obtener el numero de hilos
        np = omp_get_num_threads();
        // Calculo de las iteraciones
        for (i = 1; i < m; i++)
        {
            MultiplicaMatrizporVector(matriz, array1, array2, N, adi, filas_reparto, np, myid);

            if (myid != np)
            {
                for (i = (myid * filas_reparto); i < ((myid + 1) * filas_reparto); i++)
                {
                    if (array2[i] > mayor)
                    {
                        mayor = array2[i];
                    }
                    else if (array2[i] < menor)
                    {
                        menor = array2[i];
                    }
                }
            }
            else
            {
                for (i = (myid * filas_reparto); i < ((myid + 1) * filas_reparto) + adi; i++)
                {
                    if (array2[i] > mayor)
                    {
                        mayor = array2[i];
                    }
                    else if (array2[i] < menor)
                    {
                        menor = array2[i];
                    }
                }
            }
#pragma omp critical
            {
                menor = menor * -1;
                if (mayor >= menor)
                {
                    absolut = mayor;
                }
                else if (menor > mayor)
                {
                    absolut = menor * -1;
                }
                vector[myid] = absolut;
            }
#pragma omp barrier
#pragma omp master
            {
                mayor = 0;
                menor = 0;
                for (j = 0; i < np; i++)
                {
                    if (vector[j] > mayor)
                    {
                        mayor = vector[j];
                    }
                    else if (vector[j] < menor)
                    {
                        menor = vector[j];
                    }
                }
                menor = menor * -1;
                if (mayor >= menor)
                {
                    absolut = mayor;
                }
                else if (menor > mayor)
                {
                    absolut = menor * -1;
                }
            }
#pragma omp barrier

            if (myid != np)
            {
                for (i = (myid * filas_reparto); i < ((myid + 1) * filas_reparto); i++)
                {
                    array2[i] = array2[i] / absolut;
                }
            }
            else
            {
                for (i = (myid * filas_reparto); i < ((myid + 1) * filas_reparto) + adi; i++)
                {
                    array2[i] = array2[i] / absolut;
                }
            }

            arrayAbsolut[i] = absolut;
        }
    }
    clock_t endIter = clock();

    double timeMult = (double)(endMult - begin) * 1000 / CLOCKS_PER_SEC;
    double timeIter = (double)(endIter - begin) * 1000 / CLOCKS_PER_SEC;
    double time = (double)(endIter - cl) * 1000 / CLOCKS_PER_SEC;

    ImprimirResultadosFichero(nombreC, m, timeMult, timeIter, time, arrayAbsolut);

    return 0;
}

/*
    Funcion encargada de multiuplicar la matriz por el vector.
*/
void MultiplicaMatrizporVector(double **matriz, double *arrayV, double *arrayR, int N, int adi, int filas_reparto, int np, int myid)
{
    int i, j;
    if (myid != np)
    {
        for (i = (myid * filas_reparto); i < ((myid + 1) * filas_reparto); i++)
        {
            arrayR[i] = 0;
            for (j = 0; j < N; j++)
            {
                arrayR[i] = arrayR[i] + matriz[i][j] * arrayV[j];
            }
        }
    }
    else
    {
        for (i = (myid * filas_reparto); i < ((myid + 1) * filas_reparto) + adi; i++)
        {
            arrayR[i] = 0;
            for (j = 0; j < N; j++)
            {
                arrayR[i] = arrayR[i] + matriz[i][j] * arrayV[j];
            }
        }
    }
}

/*
    Funcion para resrevar un array dinamico
*/
double *ReservaArray(double N)
{
    int i;

    double *arr = (double *)malloc(N * sizeof(double));

    if (arr == NULL)
    {
        printf("Error en la reserva de memoria");
    }
    return arr;
}
/*
    Funcion para resrevar una matriz dinamica
*/
double **ReservaMatriz(double N)
{
    int i;

    double **matriz = (double **)malloc(N * sizeof(double *));
    if (matriz == NULL)
    {
        printf("Error al reservar memoria");
        return 0;
    }
    for (i = 0; i < N; i++)
    {
        matriz[i] = (double *)malloc(N * sizeof(double));
        if (matriz[i] == NULL)
        {
            printf("No se ha podido reservar memoria: \n");
            exit(1);
        }
    }
    return matriz;
}

/*
    Funcion que imprime los resultados.
*/
void ImprimirResultadosFichero(char *nombreC, int m, double secM, double secIt, double sec, double *absolut)
{

    FILE *fr = fopen("Resultados.txt", "w");
    int i = 0;
    fprintf(fr, "RESULTADOS: \n");
    fprintf(fr, "Sentencia de ejecucion: '%s' '%i'\n", nombreC, m);
    fprintf(fr, "Mayor absoluto de las iteraciones: ");
    for (i = 1; i < m; i++)
    {
        fprintf(fr, " %f ", absolut[i]);
    }
    fprintf(fr, "\nTiempo de multiplicar matriz por vector: %.3f ms\n", secM);
    fprintf(fr, "Tiempo de las iteraciones: %.3f ms\n", secIt);
    fprintf(fr, "Tiempo global: %.3f ms\n", sec);

    fclose(fr);
}

void LeeryEscribir(double **matriz, char *fichero, int N)
{
    int i, j;
    double buffer;
    FILE *file;

    if (file = fopen(fichero, "r"))
    {
        for (i = 0; i < N; i++)
        {
            fread(matriz[i], sizeof(buffer), N, file);
        }
        fclose(file);
    }
    else
    {

        for (i = 0; i < N; i++)
        {
            for (j = 0; j < N; j++)
            {

                matriz[i][i] = 1;
                if (j != i)
                {
                    if (i > j)
                    {
                        // Parte inferior
                        matriz[i][j] = rand() % 51;
                    }
                    else
                    {
                        // Parte superior
                        matriz[i][j] = rand() % 51 - 50;
                    }
                }
            }
        }
        file = fopen(fichero, "w+");

        for (i = 0; i < N; i++)
        {
            fwrite(matriz[i], sizeof(buffer), N, file);
        }
        fclose(file);
    }
}
