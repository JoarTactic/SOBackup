#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

#define ARCHIVO_RESPALDO "ultimo_respaldo.txt"

/**
 * num_args: Contador de Argumentos
 * args: Argumentos
 */
int main(int num_args, char *args[]){
    //Si no recibe los argumentos suficientes correspondientes
    //a la ruta relativa del directorio a respaldar y donde se
    //guardara el respaldo, entonces termina el programa
    if(num_args != 3){
        //Mensaje de error
        fprintf(stderr, "Forma de uso: %s ./<directorio_a_respaldar> ./<directorio_del_respaldo>\n", args[0]);
        return 1;
    }

    //Directorio a respaldar
    char *docsPath = args[1];  // Ruta relativa
    char *real_docsPath; //Variable donde se almacenará ruta absoluta
    //Directorio donde se almacenará el respaldo
    char *backupPath = args[2];
    char *real_backupPath;
    //Las variables anteriores deberán ser asignadas con los argumentos de main que se introduzcan al ejecutar el programa

    //Fecha y hora actual
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    // Calcular el tamaño necesario para la cadena del nombre completo
    // <nombre_base>_YYYY-MM-DD_HH-MM-SS\0 (necesitamos suficiente espacio)
    int tamano = strlen(backupPath) + 20 + 1;  // 20 para "YYYY-MM-DD_HH-MM-SS" y 1 para '\0'

    // Asignacion de memoria dinámicamente para el nombre completo del directorio
    char *real_backupName = (char *)malloc(tamano * sizeof(char));
    if (real_backupName == NULL) {
        fprintf(stderr, "Error al asignar memoria al nombre del directorio del respaldo.\n");
        free(real_backupName);
        return 1;
    }


    // Crear la cadena con el nombre base + fecha y hora
    //snprintf es usado para imprimir un string especifico con tamaño especifico y formato
    //snprintf(str, size, const char *format, ...);
    snprintf(real_backupName, tamano, "%s_%04d-%02d-%02d_%02d-%02d-%02d",
             backupPath, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, 
             tm.tm_hour, tm.tm_min, tm.tm_sec);
    //Se hacen esas sumas porque la funcion time() toma de referencia
    //el tiempo que ha pasado desde el 1 de Enero de 1970


    // Leer el nombre del último respaldo si existe
    //Se abre el archivo
    FILE *archivo_respaldo = fopen(ARCHIVO_RESPALDO, "r");
    if (archivo_respaldo != NULL) {
        // Longitud del nombre anterior (longitud de la linea):
        //Situa el puntero de lectura en la posición 0 respecto al final del archivo
        fseek(archivo_respaldo, 0, SEEK_END);
        //ftell Devuelve la posición del puntero de lectura 
        //del archivo, en forma de entero
        long tamano_anterior = ftell(archivo_respaldo);
        //Regresa el puntero al inicio del archivo
        rewind(archivo_respaldo);

        if (tamano_anterior > 0) {
            // Asignar memoria dinámica para el nombre anterior
            char *nombre_anterior = (char *)malloc((tamano_anterior + 1) * sizeof(char));
            if (nombre_anterior == NULL) {
                fprintf(stderr, "Error al asignar memoria para el nombre anterior.\n");
                free(nombre_anterior);  // Liberar memoria del nombre nuevo
                fclose(archivo_respaldo); //Se cierra el archivo
                return 1;
            }
            else{ //Si se logro asignar memoria
                //fgets(nombre_cadena, numero_caracteres, FILE)
                //Lee y almacena una cadena de caracteres, sea introducida 
                //por archivo o por teclado (stdin). Es este caso, 
                //archivo_respaldo. Al final agrega \n (salto de linea)
                fgets(nombre_anterior, tamano_anterior + 1, archivo_respaldo);
                //strcspn busca y devuelve la posición del primer 
                //carácter de la cadena nombre_anterior que coincida con \n
                //Luego, le asgina un caracter nulo (0) para truncar es parte
                nombre_anterior[strcspn(nombre_anterior, "\n")] = 0;

                // Verificar si el directorio anterior existe y eliminarlo
                //struct stat es una estructura en C para almacenar información de archivos
                //Se inicializa todos los campos en 0
                struct stat st = {0};
                //stat fue exitoso --> entonces el archivo o directorio nombre_anterior existe
                //Posteriormente procede a llenarse st con la información
                //de nombre_anterior
                if (stat(nombre_anterior, &st) == 0) {
                    printf("Eliminando el directorio anterior: '%s'\n", nombre_anterior);
                    // Calcular el tamaño necesario para el comando
                    size_t comando_size = strlen("rm -rf ") + strlen(nombre_anterior) + 1; // +1 para el carácter nulo
                    // Asignar memoria dinámica para el comando
                    char *comando = malloc(comando_size);
                    if (comando == NULL) {
                        fprintf(stderr, "Error al asignar memoria para el comando.\n");
                        return 1;
                    }

                    //snprintf se usa para formatear y almacenar un comando
                    snprintf(comando, comando_size, "rm -rf %s", nombre_anterior);
                    system(comando);  // Elimina el directorio
                    // Liberar la memoria asignada
                    free(comando);
                } else {
                    printf("El directorio '%s' no existe.\n", nombre_anterior);
                }
                // Liberar la memoria asignada al nombre anterior
                free(nombre_anterior);
            }
        }
        fclose(archivo_respaldo);
    }

    // Crear el nuevo directorio de respaldo
    // Permisos 0700:
    //7: Permiso de lectura, escritura y ejeccion para el propietario
    //0 Para el grupo y otros (no tienen permisos)
    if (mkdir(real_backupName, 0700) == 0) {
        printf("Directorio de respaldo '%s' creado correctamente.\n", real_backupName);
        // Guardar el nombre del nuevo respaldo en el archivo
        archivo_respaldo = fopen(ARCHIVO_RESPALDO, "w");
        if (archivo_respaldo != NULL) {
            fprintf(archivo_respaldo, "%s\n", real_backupName);
            fclose(archivo_respaldo);
        } else {
            fprintf(stderr, "Error al guardar el nombre del respaldo.\n");
        }
    } else {
        fprintf(stderr, "Error al crear el directorio de respaldo.\n");
        free(real_backupName);  // Libera memoria antes de salir
        return 1;
    }
    // Liberar la memoria asignada
    free(real_backupName);

    free(real_docsPath);
    free(real_backupPath);
    return 0;

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
