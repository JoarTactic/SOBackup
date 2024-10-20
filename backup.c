#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <sys/wait.h>

int main(){
    //Directorio a respaldar
    char *docsPath = "./MisDocumentos";  // Ruta relativa
    char *real_docsPath; //Variable donde se almacenará ruta absoluta
    //Directorio donde se almacenará el respaldo
    char *backupPath;
    char *real_backupPath;
    //Las variables anteriores deberán ser asignadas con los argumentos de main que se introduzcan al ejecutar el programa

    //=================================================================================
    // Ejemplo de como asignar memoria dinámicamente para la ruta absoluta
    char *path = "./backup.c";
    char *real_path;
    real_path = realpath(path, NULL);  // realpath() asigna memoria si el segundo argumento es NULL

    if (real_path != NULL) {
        printf("La ruta absoluta es: %s\n", real_path);
        free(real_path);  // Libera la memoria asignada por realpath
    } else {
        perror("Error obteniendo la ruta absoluta");
    }
    //=================================================================================


    free(real_docsPath);
    free(real_backupPath);
    return 0;


    //Creación de los arreglos para los pipes
    int pipefd[2], pipe2fd[2];

    //Creación del pipe 1
    if (pipe(pipefd) < 0){
        perror("No se pudo crear el pipe");
        exit(1);
    }

    //Creación del pipe 2
    if (pipe(pipe2fd) < 0){
        perror("No se pudo crear el pipe");
        exit(1);
    }

    //Creación del proceso hijo
    int pid;
    pid = fork();
    switch (pid) {
        case -1: //Error
            printf("No se ha podido crear un hijo\n");
            exit(-1);
            break;
        case 0: //Hijo
            
            break;
        default: //Padre
            int numero_archivos_respaldados = 0;
            //Cerramos la lectura del padre en el pipe
            close(pipefd[0]);
            //Cerramos la escritura del padre en el pipe2
		    close(pipe2fd[1]);

            printf("PADRE(pid=): generando LISTA DE ARCHIVOS A RESPALDAR\n", getpid());
            //Cambiamos a la ruta de la que haremos respaldo
            chdir(real_path);
            //Creamos un archivo con el número de archivos a respaldar en la primer línea
            system ("ls -l |tail -n +2 |wc -1 > ../listadearchivos.txt");
            //A partir de la segunda linea, hace un append de los nombres de los archivos
            system ("ls -l >> ../listadearchivos.txt");
            //Se agrega "fin" para identificar que ya no quedan mas archivos para respaldar
            system("echo fin >> ../listadearchivos.txt");
            exit(0);
            break;
    }

}
