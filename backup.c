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

    

}
