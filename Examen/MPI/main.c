/*
    Manuel Garcia Cotes
*/

#include <stdio.h>
#include <stdlib.h> 
#include <time.h>
#include <string.h>
#include <math.h>
#include <mpi.h>


int** ReservaMatriz(int filas, int columnas);
void Leer (int ** matrix , char* fichero, int filas);
int CalcularMedia(int** matriz, int i, int j);
int* ReservaArray(int N);


//filas=filas
//columnas=ancho
int main(int argc, char *argv[])
{
	
	char* fichero = "matriz300x300.bin";
	int N = 300;
	int** matriz;
	int** reparto;
	int* array;
	int myrank, nproces, filas_reparto, adi, i, j, media, suma=0, aux;
	
	
	
	MPI_Init(&argc,&argv);
	//Obtener el numero de procesos
	MPI_Comm_size(MPI_COMM_WORLD,&nproces);
	//Obtenemos el rank necesario
	MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
	
	//El status
	MPI_Status status;
	
	filas_reparto = (N/nproces);
	adi = N%nproces;
	
	
	
	if(myrank == 0)
	{
		matriz = ReservaMatriz(N, N);
		reparto = ReservaMatriz(filas_reparto+2, N);
		array = ReservaArray(nproces);
		Leer(matriz, fichero, N);
		
		for(i=0; i<filas_reparto+2; i++)
		{
			reparto[i]=matriz[i];
		}
		
		
			
			
		for(j=1; j<nproces; j++)
		{
			if(j!=(nproces-1))
			{	
				for(i=0; i<filas_reparto+2; i++)
				{
					//Aqui envio a cada proceso sus filas correspondientes.	
					MPI_Send(matriz [(filas_reparto+2 * j) + i], N, MPI_INTEGER, j, 99, MPI_COMM_WORLD);
				}
			}
			else
			{	
				for(i=0; i<filas_reparto+adi; i++)
				{
					//Este send de aqui controla las filas adicionales.
					MPI_Send(matriz [(filas_reparto * j) + i], N, MPI_INTEGER, j, 99, MPI_COMM_WORLD);
				}
			}
		}
	}
	else
	{
		
		if(myrank != (nproces-1))
		{
			reparto = ReservaMatriz(filas_reparto+2, N);
			for(i=0; i<filas_reparto+2; i++)
			{
				//Zona de recepcion de los datos
				MPI_Recv(reparto[i], N, MPI_INTEGER, 0, 99, MPI_COMM_WORLD, &status);	
			}
		}
		else
		{
			reparto = ReservaMatriz(filas_reparto+adi, N);
			for(i=0; i<filas_reparto+adi; i++)
			{
				//Zona de recepcion de los datos controlando filas adicionales.
				MPI_Recv(reparto[i], N, MPI_INTEGER, 0, 99, MPI_COMM_WORLD, &status);
			}
		}			
	}
	

	
	if (myrank != (nproces-1))
	{
		for(i=0; i<(filas_reparto); i++)
		{	
			for(j=0; j<(N-1); j++)
			{
				aux = CalcularMedia(reparto, i, j);
				suma = suma +aux;
			}
		}
	}
	else
	{
		for(i=0; i<((filas_reparto+adi)-2); i++)
		{	
			for(j=0; j<(N-1); j++)
			{
				aux = CalcularMedia(reparto, i, j);
				suma = suma +aux;
			}
		}
	}
		
	
	
	MPI_Gather(&suma, 1, MPI_INTEGER, array, 1, MPI_INTEGER, 0, MPI_COMM_WORLD);
	
	if(myrank == 0)
	{
		suma = 0;
		int k;
		for(k=0; k<nproces; k++)
		{
			suma = suma + array[k];
		}	
		printf("\n Total: %i", suma);
	}	
	
	MPI_Finalize();	
	
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

int* ReservaArray(int N)
{
    int i;

    int* arr = (int*)malloc(N*sizeof(int));

    if (arr==NULL)
    {
        printf("Error en la reserva de memoria");
    }
    return arr;
}
