
/*
    Manuel Garcia Cotes
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

double **ReservaMatriz(double N);
double *ReservaArray(double N);
void MultiplicaMatrizporVector(double **matriz, double *arrayV, double *arrayR, double N);
double MayorAbsoluto(double *array2, double N, double *array1);
void ImprimirResultadosFichero(char *nombreC, int m, double secM, double secIt, double sec, double *absolut);

int main(int argc, char *argv[])
{
    clock_t cl = clock();
    double *array1;
    double *array2;
    double *arrayAbsolut;
    double **matriz;
    double N = 1200;
    int i, j = 0;
    int m = atoi(argv[1]);
    char *nombreC = argv[0];
    double secMatriz, secIter, secMult;

    /*
        Aqui estan las resrevas de memeoria correspondientes
    */
    array1 = ReservaArray(N);
    array2 = ReservaArray(N);
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

    /*
        Aqui colocamos las variables de medidcion del tiempo y las fucniones necesarias para hacer las iteraciones y calcular el valor absoluto
        La primera iteracion esta separada ya que no se le tiene que aplicar el valor absoluto a ella.
    */
    clock_t begin = clock();
    MultiplicaMatrizporVector(matriz, array1, array2, N);
    clock_t endMult = clock();

    for (i = 0; i < N; i++)
    {
        array1[i] = array2[i];
    }
    // Calculo de las iteraciones
    for (i = 1; i < m; i++)
    {
        MultiplicaMatrizporVector(matriz, array1, array2, N);
        arrayAbsolut[i] = MayorAbsoluto(array2, N, array1);
    }
    clock_t endIter = clock();

    double timeMult = (double)(endMult - begin) * 1000 / CLOCKS_PER_SEC;
    double timeIter = (double)(endIter - begin) * 1000 / CLOCKS_PER_SEC;
    double time = (double)(endIter - cl) * 1000 / CLOCKS_PER_SEC;

    ImprimirResultadosFichero(nombreC, m, timeMult, timeIter, time, arrayAbsolut);

    return 0;
}

/*
    Funcion que calcula el mayor absoluto de los numeros de un vector y luego divide el mayor absoluto con el vector necesario para obetener valores entre -1 y 1.
    Esta fucnoin nos retorna el valor absoluto para posteriormente poder imprmirlo en el fichero de resultados.
*/
double MayorAbsoluto(double *array2, double N, double *array1)
{
    int i;
    double mayor = 0;
    double menor = 0;
    double absolut = 0;
    for (i = 0; i < N; i++)
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
    menor = menor * -1;
    if (mayor >= menor)
    {
        absolut = mayor;
    }
    else if (menor > mayor)
    {
        absolut = menor * -1;
    }

    for (i = 0; i < N; i++)
    {
        array1[i] = array2[i] / absolut;
    }

    return absolut;
}
/*
    Funcion encargada de multiuplicar la matriz por el vector.
*/
void MultiplicaMatrizporVector(double **matriz, double *arrayV, double *arrayR, double N)
{
    int i, j;
    for (i = 0; i < N; i++)
    {
        arrayR[i] = 0;
        for (j = 0; j < N; j++)
        {
            arrayR[i] = arrayR[i] + matriz[i][j] * arrayV[j];
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
