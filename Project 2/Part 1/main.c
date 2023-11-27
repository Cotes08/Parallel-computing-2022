
/*
    Manuel Garcia Cotes
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <omp.h>

unsigned char **ReservaMatriz(int filas, int columnas);
void Leer(unsigned char **matrix, char *fichero, int filas, int columnas);
void Escribir(unsigned char **matrix, int filas, int columnas, int modo);
int CalcularMedia(unsigned char **matriz, int i, int j);
int CalcularMediana(unsigned char **matriz, int i, int j);
int CalcularSobel(unsigned char **matriz, int i, int j);
void ImprimirResultadosFichero(char *nombreC, char *fich, int filas, int columnas, double segundos, double timeLeer, int num_hilos);

// filas=filas
// columnas=ancho
int main(int argc, char *argv[])
{
    clock_t cl = clock();
    clock_t beginLeer, endLeer;
    unsigned char **matriz;
    unsigned char **matrizResultMedia;
    unsigned char **matrizResultMediana;
    unsigned char **matrizResultSOBEL;
    char *nombreC = argv[0];
    char *fich = argv[1];
    int filas = atoi(argv[2]);
    int columnas = atoi(argv[3]);
    int num_hilos = atoi(argv[4]);
    int filas_reparto = filas / num_hilos;
    int adi = filas % num_hilos;
    int i, j, myid, np;
    double segundos, timeEscrbir, timeLeer;

    /*
        Reserva de memeoria para las matrices
    */
    matriz = ReservaMatriz(filas + 2, columnas + 2);
    matrizResultMedia = ReservaMatriz(filas, columnas);
    matrizResultMediana = ReservaMatriz(filas, columnas);
    matrizResultSOBEL = ReservaMatriz(filas, columnas);

    beginLeer = clock();
    Leer(matriz, fich, filas, columnas);
    endLeer = clock();

    // Estos for sirven para rellenear los bordes que no se deben procesar

    // Primera fila
    matriz[0][0] = matriz[2][2];
    matriz[0][columnas + 1] = matriz[2][columnas - 1];
    matriz[filas + 1][0] = matriz[filas - 1][2];
    matriz[filas + 1][columnas + 1] = matriz[filas - 1][columnas - 1];

    for (i = 1; i <= filas; i++)
    {
        matriz[i][0] = matriz[i][2];
        matriz[i][columnas + 1] = matriz[i][columnas - 1];
    }
    for (j = 1; j <= columnas; j++)
    {
        matriz[0][j] = matriz[2][j];
        matriz[filas + 1][j] = matriz[filas - 1][j];
    }

/*
    Aqui comienza los procesados de la foto, dependeiendo del modo que se haya elegido, se seleccionara un porocesado u otro.
    Dentro de cada modo se guarda el tiempo que ha tardado cada modo.
*/
#pragma omp parallel num_threads(num_hilos) shared(matriz, filas, columnas, filas_reparto, adi, matrizResult) private(i, j, myid, np)
    {

        myid = omp_get_thread_num();
        np = omp_get_num_threads();

        // Procesado por media
        if (myid != (np - 1))
        {
            for (i = (myid * filas_reparto) + 1; i <= ((myid + 1) * filas_reparto); i++)
            {
                for (j = 1; j <= columnas; j++)
                {

                    matrizResultMedia[i - 1][j - 1] = CalcularMedia(matriz, i, j);
                }
            }
        }
        else
        {
            for (i = (myid * filas_reparto) + 1; i <= ((myid + 1) * filas_reparto) + adi; i++)
            {
                for (j = 1; j <= columnas; j++)
                {
                    matrizResultMedia[i - 1][j - 1] = CalcularMedia(matriz, i, j);
                }
            }
        }
    }
#pragma omp parallel num_threads(num_hilos) shared(matriz, filas, columnas, filas_reparto, adi, matrizResult) private(i, j, myid, np)
    {
        // Procesado por mediana
        if (myid != (np - 1))
        {
            for (i = (myid * filas_reparto) + 1; i <= ((myid + 1) * filas_reparto); i++)
            {
                for (j = 1; j <= columnas; j++)
                {
                    matrizResultMediana[i - 1][j - 1] = CalcularMediana(matriz, i, j);
                }
            }
        }
        else
        {
            for (i = (myid * filas_reparto) + 1; i <= ((myid + 1) * filas_reparto) + adi; i++)
            {
                for (j = 1; j <= columnas; j++)
                {
                    matrizResultMediana[i - 1][j - 1] = CalcularMediana(matriz, i, j);
                }
            }
        }
    }

#pragma omp parallel num_threads(num_hilos) shared(matriz, filas, columnas, filas_reparto, adi, matrizResult) private(i, j, myid, np)
    {
        // Procesado por SOBEL
        if (myid != (np - 1))
        {
            for (i = (myid * filas_reparto) + 1; i <= ((myid + 1) * filas_reparto); i++)
            {
                for (j = 1; j <= columnas; j++)
                {
                    matrizResultSOBEL[i - 1][j - 1] = CalcularSobel(matriz, i, j);
                }
            }
        }
        else
        {
            for (i = (myid * filas_reparto) + 1; i <= ((myid + 1) * filas_reparto) + adi; i++)
            {
                for (j = 1; j <= columnas; j++)
                {
                    matrizResultSOBEL[i - 1][j - 1] = CalcularSobel(matriz, i, j);
                }
            }
        }
    }
    // Escribir media
    Escribir(matrizResultMedia, filas, columnas, 1);
    // Escribir mediana
    Escribir(matrizResultMediana, filas, columnas, 2);
    // Escribir SOBEL
    Escribir(matrizResultSOBEL, filas, columnas, 3);

    segundos = (double)(clock() - cl) * 1000 / CLOCKS_PER_SEC;
    timeLeer = (double)(endLeer - beginLeer) * 1000 / CLOCKS_PER_SEC;
    ImprimirResultadosFichero(nombreC, fich, filas, columnas, segundos, timeLeer, num_hilos);
    return 0;
}

/*
    Esta funcion es la encargada de resrevar memoria para las matrices dependiendo de las
    filas y columnas que se le indique
*/
unsigned char **ReservaMatriz(int filas, int columnas)
{
    int i;

    unsigned char **matriz = (unsigned char **)malloc(filas * sizeof(unsigned char *));
    if (matriz == NULL)
    {
        printf("Error al reservar memoria");
        return 0;
    }
    for (i = 0; i < filas; i++)
    {
        matriz[i] = (unsigned char *)malloc(columnas * sizeof(unsigned char));
        if (matriz[i] == NULL)
        {
            printf("No se ha podido reservar memoria: \n");
            exit(1);
        }
    }
    return matriz;
}

/*
    Esta funcion es la encargada de leer el fichero
*/
void Leer(unsigned char **matrix, char *fichero, int filas, int columnas)
{
    FILE *f = fopen(fichero, "rb");
    unsigned char buffer;
    int i, j;

    for (i = 1; i <= filas; i++)
    {
        for (j = 1; j <= columnas; j++)
        {
            matrix[i][j] = fgetc(f);
        }
    }
    fclose(f);
}
/*
    Esta funcion es la encargada de generar el fichero .raw
*/
void Escribir(unsigned char **matrix, int filas, int columnas, int modo)
{
    FILE *im;
    if (modo == 1)
    {
        im = fopen("imagenMedia.raw", "wb");
    }
    else if (modo == 2)
    {
        im = fopen("imagenMediana.raw", "wb");
    }
    else if (modo == 3)
    {
        im = fopen("imagenSOBEL.raw", "wb");
    }
    int i, j;
    unsigned char buffer;
    for (i = 0; i < filas; i++)
    {
        fwrite(matrix[i], sizeof(buffer), filas, im);
    }
    fclose(im);
}

/*
    Funcion para hacer el procesado por media
*/
int CalcularMedia(unsigned char **matriz, int i, int j)
{
    int media = 0;
    // Para calcular este numero, tenemos primero que ir sumando el valor de los vecinos uno por uno
    // Vecino 1:
    media = media + matriz[i - 1][j - 1];
    // Vecino 2:
    media = media + matriz[i - 1][j];
    // Vecino 3:
    media = media + matriz[i - 1][j + 1];
    // Vecino 4:
    media = media + matriz[i][j - 1];
    // Centro
    media = media + matriz[i][j];
    // Vecino 5:
    media = media + matriz[i][j + 1];
    // Vecino 6:
    media = media + matriz[i + 1][j - 1];
    // Vecino 7:
    media = media + matriz[i + 1][j];
    // Vecino 8:
    media = media + matriz[i + 1][j + 1];
    media = media / 9;

    return media;
}

/*
    Algoritmo quicksort
*/
void qs(int vector[], int limite_izq, int limite_der)
{
    int izq, der, temporal, pivote;

    izq = limite_izq;
    der = limite_der;
    pivote = vector[(izq + der) / 2];

    do
    {
        while (vector[izq] < pivote && izq < limite_der)
            izq++;
        while (pivote < vector[der] && der > limite_izq)
            der--;
        if (izq <= der)
        {
            temporal = vector[izq];
            vector[izq] = vector[der];
            vector[der] = temporal;
            izq++;
            der--;
        }

    } while (izq <= der);
    if (limite_izq < der)
    {
        qs(vector, limite_izq, der);
    }
    if (limite_der > izq)
    {
        qs(vector, izq, limite_der);
    }
}
/*
    Funcion para llamar a la funcion quicksort
*/
void quicksort(int lista[], int n)
{
    qs(lista, 0, n - 1);
}

/*
    Funcion para hacer el procesado por mediana
*/
int CalcularMediana(unsigned char **matriz, int i, int j)
{
    int array[9];
    int mediana = 0;

    // Vecino 1:
    array[0] = matriz[i - 1][j - 1];
    // Vecino 2:
    array[1] = matriz[i - 1][j];
    // Vecino 3:
    array[2] = matriz[i - 1][j + 1];
    // Vecino 4:
    array[3] = matriz[i][j - 1];
    // Centro
    array[4] = matriz[i][j];
    // Vecino 5:
    array[5] = matriz[i][j + 1];
    // Vecino 6:
    array[6] = matriz[i + 1][j - 1];
    // Vecino 7:
    array[7] = matriz[i + 1][j];
    // Vecino 8:
    array[8] = matriz[i + 1][j + 1];

    // Aqui sacamos el size del array para pasarselo a la funcion.
    int size = 9;
    quicksort(array, size);

    // Aqui sacamos el elemento del centro y se lo pasamos al procesado de mediana
    mediana = array[4];

    return mediana;
}

/*
    Esta es la funcion encargada de hacer el procesado por SOBEL
*/
int CalcularSobel(unsigned char **matriz, int i, int j)
{
    int Sobel = 0;
    int c = 0;
    int f = 0;
    int valor = 0;
    int F = 0, C = 0;

    // Vecino 1:
    valor = matriz[i - 1][j - 1];
    c = c + (valor * -1);
    f = f + (valor * -1);
    // Vecino 2:
    valor = matriz[i - 1][j];
    c = c + (valor * 0);
    f = f + (valor * -2);
    // Vecino 3:
    valor = matriz[i - 1][j + 1];
    c = c + (valor * 1);
    f = f + (valor * -1);
    // Vecino 4:
    valor = matriz[i][j - 1];
    c = c + (valor * -2);
    f = f + (valor * 0);
    // Centro
    valor = matriz[i][j];
    c = c + (valor * 0);
    f = f + (valor * 0);
    // Vecino 5:
    valor = matriz[i][j + 1];
    c = c + (valor * 2);
    f = f + (valor * 0);
    // Vecino 6:
    valor = matriz[i + 1][j - 1];
    c = c + (valor * -1);
    f = f + (valor * 1);
    // Vecino 7:
    valor = matriz[i + 1][j];
    c = c + (valor * 0);
    f = f + (valor * 2);
    // Vecino 8:
    valor = matriz[i + 1][j + 1];
    c = c + (valor * 1);
    f = f + (valor * 1);

    F = pow(f, 2);
    C = pow(c, 2);
    Sobel = F + C;

    Sobel = sqrt(Sobel);

    return Sobel;
}
/*
    Funcion para imprimir el fichero de resultados
*/
void ImprimirResultadosFichero(char *nombreC, char *fich, int filas, int columnas, double segundos, double timeLeer, int num_hilos)
{

    FILE *fr = fopen("Resultados.txt", "w");

    fprintf(fr, "RESULTADOS: \n");
    fprintf(fr, "Sentencia de ejecucion: '%s' '%s' '%i' '%i' '%i'\n", nombreC, fich, filas, columnas, num_hilos);
    fprintf(fr, "Tiempo que tarda en leer la matriz: %.3f ms\n", timeLeer);
    fprintf(fr, "Tiempo de ejecucion: %.3f ms\n", segundos);

    fclose(fr);
}
