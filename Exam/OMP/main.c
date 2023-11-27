/*
    Manuel Garcia Cotes
*/

#include <stdio.h>
#include <stdlib.h> 
#include <time.h>
#include <string.h>
#include <math.h>
#include <omp.h>


int** ReservaMatriz(int filas, int columnas);
void Leer (int ** matrix , char* fichero, int filas);
int CalcularMedia(int** matriz, int i, int j);


//filas=filas
//columnas=ancho
int main(int argc, char *argv[])
{
	
	char* fichero = "matriz300x300.bin";
	int N = 300;
	int** matriz;
	int** reparto;
	int myid, np, filas_reparto, columnas_reparto, adi, i, j, suma=0, aux, num_hilos, filas;
	
	num_hilos = atoi(argv[1]);
	
	
	filas_reparto = (N/num_hilos);
	filas = N-2;
	columnas_reparto = (N/num_hilos);
	adi = N%num_hilos;
	
	
	
	matriz = ReservaMatriz(N, N);
	Leer(matriz, fichero, N);
	
	
	#pragma omp parallel for num_threads(num_hilos) shared(matriz, N, filas_reparto) private(i, j, aux) schedule(static, filas_reparto) reduction(+:suma) default(none)
	
		for(j=0; j<(N-1); j++)
		{	
			for(i=0; i<N-2; i++)
			{
				aux = CalcularMedia(matriz, i, j);
				suma = suma +aux;
			}
		}

	printf("\n Automatizador ditribuido por columnas: %i", suma);
	printf("\n ");
	suma=0;
	
	#pragma omp parallel num_threads(num_hilos) shared(matriz, N, np, adi, filas_reparto, columnas_reparto, filas) private(i, j, aux, myid) reduction(+:suma) default(none)
	{
		//Obtener el id del hilo
		myid= omp_get_thread_num();
		//Obtenemos el numero de hilos
		np= omp_get_num_threads();
		
		
		if(myid != (np-1))
		{
			for(j=(myid*columnas_reparto); j<((myid+1)*columnas_reparto); j++)
			{	
				for(i=0; i<filas; i++)
				{
					aux = CalcularMedia(matriz, i, j);
					suma = suma +aux;
				}
			}
		}
		else
		{
			for(j=myid*columnas_reparto; j<((myid+1)* columnas_reparto); j++)
			{	
				for(i=0; i<filas+adi; i++)
				{
					aux = CalcularMedia(matriz, i, j);
					suma = suma +aux;
				}
			}
		}	
		
		
	}
	printf("\n Sin automatizador ditribuido por columnas: %i", suma);
	printf("\n ");
	suma=0;
	
	#pragma omp parallel for num_threads(num_hilos) shared(matriz, N) private(i, j, aux) schedule(static, 10) reduction(+:suma) default(none)
	
		for(i=0; i<N-2; i++)
		{	
			for(j=0; j<(N-1); j++)
			{
				aux = CalcularMedia(matriz, i, j);
				suma = suma +aux;
			}
		}

	printf("\n Automatizador ditribuido por filas de 10: %i", suma);
	printf("\n ");	
    return 0;    
}

/*
    Esta funcion es la encargada de resrevar memoria para las matrices dependiendo de las 
    filas y columnas que se le indique
*/
int** ReservaMatriz(int filas, int columnas)
{
    int i;

    int ** matriz= (int **)malloc(filas * sizeof(int*));
    if( matriz == NULL)
    {
        printf("Error al reservar memoria");
        return 0;
    }
    for (i = 0; i <filas; i++)
    {
       matriz[i]= (int *)malloc(columnas*sizeof(int ));
       if(matriz[i] == NULL)
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
void Leer (int ** matrix , char* fichero, int filas)
{
	
    int i,j;
    int buffer;
    FILE *file;

    if (file = fopen (fichero, "r"))
    {
        for(i=0;i<filas;i++)
        {	     
            fread(matrix[i], sizeof(int), filas, file);	    	
        }
        fclose(file);
    }
}


/*
    Funcion para hacer el procesado por media
*/
int CalcularMedia(int** matriz, int i, int j)
{
	int media = 0;
	int suma = 0;
	    //Para calcular este numero, tenemos primero que ir sumando el valor de los vecinos uno por uno
	    //Vecino 1:
		media = media + matriz[i][j];
		media = media + matriz[i][j+1];
		media = media + matriz[i+1][j];
		media = media + matriz[i+1][j+1];
		media = media + matriz[i+2][j];
		
    media= media/5;
	
	if(matriz[i+2][j+1] > media)
	{
		suma = suma + 1;
	}
	return suma;
}

