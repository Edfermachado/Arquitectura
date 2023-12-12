#include <stdio.h>
#include "libsegy.c" // Reemplaza con la inclusión correcta de tu biblioteca SEGY

int main() {

	FILE *salida;
	salida = fopen("Trazas.txt", "w");

    int fd = segy_open_file("arch.sgy", SEGY_RDONLY); // Abre el archivo SEGY en modo lectura
    if (fd == -1) {
        printf("Error al abrir el archivo SEGY\n");
        return 1;
    }

    segy_file_header file_header;
    int result = segy_read_file_header(fd, &file_header); // Lee el encabezado del archivo SEGY
    if (result != SEGY_NO_ERROR) {
        printf("Error al leer el encabezado del archivo SEGY\n");
        segy_close_file(fd);
        return 1;
    }

    // Imprime los encabezados del archivo SEGY
    printf("Text Header: %s\n", file_header.TextHeader);
    fprintf(salida,"Text Header: %s\n", file_header.TextHeader);
    printf("Job ID: %d\n", file_header.JobID);
    fprintf(salida,"Job ID: %d\n", file_header.JobID);
    printf("Line Number: %d\n", file_header.LineNumber);
    fprintf(salida,"Line Number: %d\n", file_header.LineNumber);
    // Continuar con la impresión de otros campos del encabezado...

    fclose(salida);
    segy_close_file(fd); // Cierra el archivo SEGY
    return 0;
}
