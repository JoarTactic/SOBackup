//Casillas Herrera Leonardo Didier
//Castillo Bautista Samantha Lucia
//Romero Cervantes Jorge Arturo

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <fcntl.h>

#define ARCHIVO_RESPALDO "ultimo_respaldo.txt"
#define LISTADO_ARCHIVOS "./listado_Archivos.txt"
#define MAX_BUFFER 128
// Función recursiva para eliminar archivos/directorios y listar su eliminación
void eliminarDirectorio(const char *ruta_relativa)
{
    struct stat info;
    // Se obtene información del archivo/directorio
    if (stat(ruta_relativa, &info) == 0)
    {
        if (S_ISDIR(info.st_mode))
        {                                             // Si es un directorio. Función de la libreria stat
            DIR *directorio = opendir(ruta_relativa); // opendir de la liberia dirent.h
            // La estructura dirent representa las entradas de un directorio
            // Se usa para interactuar con el contenido del directorio y saber información
            // como el nombre de un archivo/subdirectorio
            struct dirent *entrada;

            if (directorio == NULL)
            {
                fprintf(stderr, "Error abriendo el directorio '%s'.\n", ruta_relativa);
                return;
            }

            // Lee el contenido del directorio
            while ((entrada = readdir(directorio)) != NULL)
            {
                // Ignorar "." y "..", correspondiente a el directorio actual y el directorio padre
                // respectivamente. Son entradas especiales presentes en todos los directorios de
                // sistemas tipo UNIX
                if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0)
                {
                    continue; // Se salta a la siguiente iteración del while
                }

                // Se calcula el tamaño necesario para almacenar la ruta relativa completa
                size_t longitud_ruta = strlen(ruta_relativa) + strlen(entrada->d_name) + 2; // +2 para '/' y '\0'
                // Se asigna memoria dinámica
                char *ruta_elemento = (char *)malloc(longitud_ruta * sizeof(char));
                if (ruta_elemento == NULL)
                {
                    fprintf(stderr, "Error: No se pudo asignar memoria al nombre de subdirectorio.\n");
                    closedir(directorio); // Asegurarse de cerrar el directorio antes de salir por falla
                    return;
                }
                // Construir la ruta del elemento
                snprintf(ruta_elemento, longitud_ruta, "%s/%s", ruta_relativa, entrada->d_name);
                // Llamada recursiva para eliminar el subdirectorio
                eliminarDirectorio(ruta_elemento);
            }
            closedir(directorio);

            // Eliminar el directorio vacío
            if(rmdir(ruta_relativa) == 0){ printf("removed directory '%s'\n", ruta_relativa);}
            else {fprintf(stderr, "Error al eliminar el directorio '%s'.\n", ruta_relativa);}
        }
        else
        { // si es un archivo en vez de un directorio
            // Con unlink() de desvincula el archivo con el directorio, y posteriormente se libera su memoria
            if (unlink(ruta_relativa) == 0)
                printf("removed '%s'\n", ruta_relativa);
            else
                fprintf(stderr, "Error al eliminar el archivo '%s'.\n", ruta_relativa);
        }
    }
    else
    {
        fprintf(stderr, "Error al obtener la información de '%s'.\n", ruta_relativa);
    }
}

// Crear nueva carpeta de respaldo, y borrar la vieja
void renovar_carpeta_respaldo(char *backupPath){
    // Fecha y hora actual
        time_t t = time(NULL);
        struct tm tiempo = *localtime(&t);

        // Calcular el tamaño necesario para la cadena del nombre completo
        // <nombre_base>_YYYY-MM-DD_HH-MM-SS\0 (necesitamos suficiente espacio)
        int tamano = strlen(backupPath) + 20 + 1; // 20 para "YYYY-MM-DD_HH-MM-SS" y 1 para '\0'

        // Asignacion de memoria dinámicamente para el nombre completo del directorio
        char *real_backupName = (char *)malloc(tamano * sizeof(char));
        if (real_backupName == NULL)
        {
            fprintf(stderr, "Error al asignar memoria al nombre del directorio del respaldo.\n");
            free(real_backupName);
            return;
        }

        // Crear la cadena con el nombre base + fecha y hora
        // snprintf es usado para imprimir un string especifico con tamaño especifico y formato
        // snprintf(str, size, const char *format, ...);
        snprintf(real_backupName, tamano, "%s_%04d-%02d-%02d_%02d-%02d-%02d",
                 backupPath, tiempo.tm_year + 1900, tiempo.tm_mon + 1, tiempo.tm_mday,
                 tiempo.tm_hour, tiempo.tm_min, tiempo.tm_sec);
        // Se hacen esas sumas porque la funcion time() toma de referencia
        // el tiempo que ha pasado desde el 1 de Enero de 1900

        // Leer el nombre del último respaldo si existe
        // Se abre el archivo
        FILE *archivo_respaldo = fopen(ARCHIVO_RESPALDO, "r");
        if (archivo_respaldo != NULL)
        {
            // Longitud del nombre anterior (longitud de la linea):
            // Situa el puntero de lectura en la posición 0 respecto al final del archivo
            fseek(archivo_respaldo, 0, SEEK_END);
            // ftell Devuelve la posición del puntero de lectura
            // del archivo, en forma de entero
            long tamano_anterior = ftell(archivo_respaldo);
            // Regresa el puntero al inicio del archivo
            rewind(archivo_respaldo);

            if (tamano_anterior > 0)
            {
                // Asignar memoria dinámica para el nombre anterior
                char *nombre_anterior = (char *)malloc((tamano_anterior + 1) * sizeof(char));
                if (nombre_anterior == NULL)
                {
                    fprintf(stderr, "Error al asignar memoria para el nombre anterior.\n");
                    free(nombre_anterior);    // Liberar memoria del nombre nuevo
                    fclose(archivo_respaldo); // Se cierra el archivo
                    return;
                }
                else
                { // Si se logro asignar memoria
                    // fgets(nombre_cadena, numero_caracteres, FILE)
                    // Lee y almacena una cadena de caracteres, sea introducida
                    // por archivo o por teclado (stdin). Es este caso,
                    // archivo_respaldo. Al final agrega \n (salto de linea)
                    fgets(nombre_anterior, tamano_anterior + 1, archivo_respaldo);
                    // strcspn busca y devuelve la posición del primer
                    // carácter de la cadena nombre_anterior que coincida con \n
                    // Luego, le asgina un caracter nulo (0) para truncar es parte
                    nombre_anterior[strcspn(nombre_anterior, "\n")] = 0;

                    // Verificar si el directorio anterior existe y eliminarlo
                    // struct stat es una estructura en C para almacenar información de archivos
                    // Se inicializa todos los campos en 0
                    struct stat info = {0};
                    // stat fue exitoso --> entonces el archivo o directorio nombre_anterior existe
                    // Posteriormente procede a llenarse st con la información
                    // de nombre_anterior
                    if (stat(nombre_anterior, &info) == 0)
                    {
                        printf("PADRE(pid=%d): borrando respaldo viejo...\n", getpid());
                        eliminarDirectorio(nombre_anterior);
                    }
                    else
                    {
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
        // 7: Permiso de lectura, escritura y ejeccion para el propietario
        // 0 Para el grupo y otros (no tienen permisos)
        if (mkdir(real_backupName, 0700) == 0)
        {
            printf("creando respaldo nuevo en '%s'\n", real_backupName);
            // Guardar el nombre del nuevo respaldo en el archivo
            archivo_respaldo = fopen(ARCHIVO_RESPALDO, "w");
            if (archivo_respaldo != NULL)
            {
                fprintf(archivo_respaldo, "%s\n", real_backupName);
                fclose(archivo_respaldo);
                //Tambien se guarda en la variable para su uso futuro:}
                char *backupPath = (char *)malloc((strlen(real_backupName) + 1) * sizeof(char)); // +1 para el terminador '\0'
                if (backupPath == NULL) {
                    fprintf(stderr, "Error al guardar el nombre del nuevo respaldo en una variable.\n");
                    return; // Retorna NULL si falla la asignación
                }

                // Copiar el contenido del buffer al puntero `linea`
                strcpy(backupPath, real_backupName);
            }
            else
            {
                fprintf(stderr, "Error al guardar el nombre del respaldo.\n");
            }
        }
        else
        {
            fprintf(stderr, "Error al crear el directorio de respaldo.\n");
            free(real_backupName); // Libera memoria antes de salir
            return;
        }
}


// Para poner en un txt la lista
void enlistarArchivos(const char *ruta_relativa, FILE *archivo, int *contador)
{
    struct stat info;
    // Se obtene información del archivo/directorio
    if (stat(ruta_relativa, &info) == 0)
    {
        if (S_ISDIR(info.st_mode))
        {
            DIR *objetivo = opendir(ruta_relativa);
            struct dirent *entrada;

            if (objetivo == NULL)
            {
                fprintf(stderr, "Error abriendo el directorio '%s'.\n", ruta_relativa);
                return;
            }
            // Lee el contenido del directorio
            while ((entrada = readdir(objetivo)) != NULL)
            {
                if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0)
                {
                    continue; // Se salta a la siguiente iteración del while
                }
                // Se calcula el tamaño necesario para almacenar la ruta relativa completa
                size_t longitud_ruta = strlen(ruta_relativa) + strlen(entrada->d_name) + 2; // +2 para '/' y '\0'
                // Se asigna memoria dinámica
                char *ruta_elemento = (char *)malloc(longitud_ruta * sizeof(char));
                if (ruta_elemento == NULL)
                {
                    fprintf(stderr, "Error: No se pudo asignar memoria al nombre de subdirectorio.\n");
                    closedir(objetivo); // Asegurarse de cerrar el directorio antes de salir por falla
                    return;
                }
                // Construir la ruta del elemento
                snprintf(ruta_elemento, longitud_ruta, "%s/%s", ruta_relativa, entrada->d_name);
                enlistarArchivos(ruta_elemento, archivo, contador);
                free(ruta_elemento);
            }
            closedir(objetivo);
        }
        else
        {
            char* ruta_truncada = strchr(ruta_relativa, '/');
            char* ruta_archivo = strchr(ruta_truncada + 1, '/');
            fprintf(archivo, "%s\n", ruta_archivo);
            (*contador)++;
        }
    }
    else
    {
        fprintf(stderr, "Error al obtener la información de '%s'.\n", ruta_relativa);
    }
}



void lista_y_contador_archivos(const char *ruta_relativa, int *contador){
    
    FILE *archivo = fopen(LISTADO_ARCHIVOS, "w");
    if (archivo == NULL){
        fprintf(stderr, "Error al abrir el archivo '%s'.\n", LISTADO_ARCHIVOS);
        return;
    }
    //int *contador = 0; // Contador de archivos

    //Reserva la primera linea del archivo para posteriormente poner el número de archivos
    if(ftell(archivo) == 0){
        fprintf(archivo, "\n.");
    }
    enlistarArchivos(ruta_relativa, archivo, contador);

    // Escribir la cantidad de archivos en la primera línea
    rewind(archivo);  // Volver al inicio del archivo
    fprintf(archivo, "%d\n", *contador);  // Escribir el número de archivos

    // Ir al final del archivo y escribir "fin"
    fseek(archivo, 0, SEEK_END);  // Moverse al final del archivo
    fprintf(archivo, "FIN\n");    // Escribir "fin"
    //Cierra el archivo usado
    fclose(archivo);
}

void copiar_archivo(const char *origen, const char *destino) {
    int leer_origen = open(origen, O_RDONLY);
    int escribir_destino = open(destino, O_WRONLY | O_CREAT, 0644);

    if (leer_origen < 0 || escribir_destino < 0) {
        printf("Error abriendo archivos\n");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_BUFFER];
    ssize_t bytes;
    while ((bytes = read(leer_origen, buffer, sizeof(buffer))) > 0) {
        write(escribir_destino, buffer, bytes);
    }

    close(leer_origen);
    close(escribir_destino);
}

/**
 * num_args: Contador de Argumentos
 * args: Argumentos
 */
int main(int num_args, char *args[])
{
    // Si no recibe los argumentos suficientes correspondientes
    // a la ruta relativa del directorio a respaldar y donde se
    // guardara el respaldo, entonces termina el programa
    if (num_args != 3)
    {
        // Mensaje de error
        fprintf(stderr, "Forma de uso: %s ./<directorio_a_respaldar> ./<directorio_del_respaldo>\n", args[0]);
        return 1;
    }

    // Creación de los arreglos para los pipes
    int pipefd[2], pipe2fd[2];

    // Creación de los pipes
    if (pipe(pipefd) < 0 || pipe(pipe2fd) < 0)
    {
        perror("No se pudo crear el pipe");
        exit(1);
    }

    // Creación del proceso hijo
    int pid;
    pid = fork();
    switch (pid){

    case -1: // Error
        printf("No se ha podido crear un hijo\n");
        exit(-1);
        break;

    case 0:                // Hijo
        close(pipefd[1]);  // Cerrar el extremo de escritura del pipe padre-hijo
        close(pipe2fd[0]); // Cerrar el extremo de lectura del pipe hijo-padre
        char buffer[MAX_BUFFER];

        //PASO 1: El hijo imprime un mensaje indicando que está esperando el mensaje del padre 
        printf("Soy hijo, y estoy esperando a mi padre\n");

        //PASO 2: Se queda en espera con un ciclo en el read(…) hasta que el padre envíe el 
        //nombre del archivo a respaldar 
        //Primero se recibe la carperta donde estan los archivos a respaldar, y donde se respaldaran
        read(pipefd[0], buffer, sizeof(buffer));
        // Guardar la carpeta de origen
        char *docsFolder = (char *)malloc((strlen(buffer) + 1) * sizeof(char));
        if (docsFolder == NULL){
            fprintf(stderr, "Error en hijo: No se pudo asignar memoria a la carpeta origen.\n");
        }
        strcpy(docsFolder, buffer);
        // Guardar las carpeta de destino 
        read(pipefd[0], buffer, sizeof(buffer));
        char *docsBackup = (char *)malloc((strlen(buffer) + 1) * sizeof(char));
        if (docsBackup == NULL){
            fprintf(stderr, "Error en hijo: No se pudo asignar memoria a la carpeta destino.\n");
        }
        strcpy(docsBackup, buffer);

        //PASO 3 Y 4: Cuando el hijo recibe el número (cadena de texto): 
        //No olvides convertirla a un entero (atoi(..)) 
        int num_archivos;
        read(pipefd[0], &num_archivos, sizeof(num_archivos));

        //PASO 5: El hijo recibe del padre el nombre del archivo y realiza la 
        //copia del directorio origen al directorio destino. 
        size_t longitud_ruta;
        char ruta_origen[100], ruta_destino[100];
        for (int i = 0; i < num_archivos; i++)
        {
            read(pipefd[0], buffer, sizeof(buffer)); // Recibe el nombre del archivo
            char ruta_origen[MAX_BUFFER], ruta_destino[MAX_BUFFER];
            //Asignar memoria a ruta_origen de forma dinámica
            /*
            longitud_ruta = strlen(docsFolder) + strlen(buffer) + 1;
            char *ruta_origen = (char *)malloc(longitud_ruta * sizeof(char));
            if (ruta_origen == NULL) {
                fprintf(stderr, "Error: no se pudo asignar memoria a ruta_origen.\n");
                return 1;
            }
            */
            //Asignar memoria a ruta_destino de forma dinámica
            /*
            longitud_ruta = strlen(docsBackup) + strlen(buffer) + 1;
            char *ruta_destino = (char *)malloc(longitud_ruta * sizeof(char));
            if (ruta_destino == NULL) {
                fprintf(stderr, "Error: no se pudo asignar memoria a ruta_destino.\n");
                return 1;
            }
            */
            snprintf(ruta_origen, sizeof(ruta_origen), "%s%s", docsFolder, buffer);
            snprintf(ruta_destino, sizeof(ruta_destino), "%s%s", docsBackup, buffer);
            //Comprobacion
            printf("Ruta origen: %s + %s = %s\n",docsFolder, buffer, ruta_origen);
            printf("Ruta destino: %s + %s = %s\n",docsBackup, buffer, ruta_destino);


            /*FILE *origen = fopen(ruta_origen, "r");
            FILE *destino = fopen(ruta_destino, "w");
            if (origen != NULL && destino != NULL)
            {
                char c;
                while ((c = fgetc(origen)) != EOF)
                {
                    fputc(c, destino);
                }
            }
            fclose(origen);
            fclose(destino);
            */

            //De la memoria dinamica
            //free(ruta_origen);
            //free(ruta_destino);
            copiar_archivo(ruta_origen, ruta_destino);
        }

        // Informar al padre cuántos archivos se respaldaron
        write(pipe2fd[1], &num_archivos, sizeof(num_archivos));
        close(pipefd[0]);
        close(pipe2fd[1]);
        exit(0);
        break;

    default: // Padre
        char *docsPath = args[1]; // Directorio a respaldar
        char *backupPath = args[2]; // Directorio donde se almacenará el respaldo

        int numero_archivos_respaldados = 0;
        close(pipefd[0]); // Cerramos la lectura del padre en el pipe
        close(pipe2fd[1]); // Cerramos la escritura del padre en el pipe2

        // PASO 1: GENERAR UN ARCHIVO CON LA LISTA DE NOMBRES DE ARCHIVOS A RESPALDAR Y NUMERO TOTAL
        int contador = 0;
        lista_y_contador_archivos(docsPath, &contador);

        // PASO2: SE CREA EL DIRECTORIO DE RESPALDO. SI YA EXISTE, SE ELIMINA
        renovar_carpeta_respaldo(backupPath);

        // PASO 3: SE ENVIA A HIJO EL NOMBRE DE LOS ARCHIVOS Y EL NUMERO A RESPALDAR
        FILE *archivo = fopen(LISTADO_ARCHIVOS, "r");
        if (archivo == NULL) {
            fprintf(stderr, "Error al abrir el listado de archivos");
            return 1;
        }
        // Le enviamos al hijo la ruta de la carpeta a respaldar y la de respaldo
        char buffer_padre[MAX_BUFFER];
        strcpy(buffer_padre, docsPath);
        write(pipefd[1], buffer_padre, sizeof(buffer_padre));
        strcpy(buffer_padre, backupPath);
        write(pipefd[1], buffer_padre, sizeof(buffer_padre));
        //Variable usada como buffer para almacenar cualquier linea de un archivo .txt
        static char linea[100];
        // Le enviamos al hijo el numero de archivos a respaldar
        if (fgets(linea, sizeof(linea), archivo) == NULL) {
            fprintf(stderr, "Error al obtener el numero de archivos");
        }
        // Convertir la cadena a un entero
        int numero_archivos = atoi(linea);
        // Se manda ese número al hijo
        write(pipefd[1], &numero_archivos, sizeof(numero_archivos));
        
        
        //PASO 4: El padre estará en un ciclo leyendo el nombre del archivo y enviándoselo a su hijo
        while (fgets(linea, sizeof(linea), archivo)) {
            //Pasa la dirección escrita en la linea del archivo
            write(pipefd[1], linea, sizeof(linea));
            // Verificar si la línea contiene la palabra clave "fin"
            if (strstr(linea, "FIN") != NULL) {
                break;
            }
        }
        fclose(archivo);
        // Se cierra la escritura cuando acabe de mandar los nombres al hijo
        close(pipefd[1]);

        //PASO 5: El padre se queda pendiente con read(..) hasta que el hijo envié el número de 
        //archivos respaldados con éxito y realiza la comprobación. 
        int recibido = 0;
        while (1)
        {
            recibido = read(pipe2fd[0], &numero_archivos_respaldados, sizeof(numero_archivos_respaldados));
            if (recibido > 0)
            {
                break;
            }
        }
        printf("PADRE(pid=%d): Recibe el total de %d archivos respaldados con exito\n", getpid(), numero_archivos_respaldados);

        fclose(archivo);
        int archivos = numero_archivos_respaldados;
        // Comprueba el numero de archivos respaldados
        printf("(PADRE PID: %d):Comprobando respaldo...\n", getpid());
        printf("===========================================\n");

        // Comando para listar el respaldo y meterlo a comprobar_respaldo.txt
        char comprobar_respaldo[100];
        strcpy(comprobar_respaldo, "ls -1 ");
        strcpy(comprobar_respaldo + strlen(comprobar_respaldo), backupPath);
        strcpy(comprobar_respaldo + strlen(comprobar_respaldo), " > ../comprobar_respaldo.txt");
        system(comprobar_respaldo);

        // Abre el archivo en modo lectura y va imprimiendo todas las lineas
        archivo = fopen("../comprobar_respaldo.txt", "r");
        for (int i = 0; i < archivos; i++)
        {
            printf("%s\n", fgets(linea, sizeof(linea), archivo));
        }
        // Imprime un mensaje para indicar que terminó de comprobar el respaldo
        printf("ARCHIVOS RESPALDADOS\n");
        fclose(archivo);
        printf("=========================================================\n");
        printf("\nTermino el proceso padre \n");

        exit(0);
        break;
    }
    return 0;
}
