#include <stdio.h>
#include <stdlib.h>
#include "libsegy.h"

#define TRACE_HEADER_SIZE 240 // Tamaño del encabezado de la traza en bytes
#define SAMPLES_PER_TRACE 1000 // Número de muestras por traza

int main() {
    FILE *segy_file;
    segy_file = fopen("arch.sgy", "rb"); // Abre el archivo SEGY en modo lectura binaria
    if (segy_file == NULL) {
        printf("Error al abrir el archivo SEGY\n");
        return 1;
    }

    // Salta el encabezado general y los encabezados de las trazas si es necesario
    fseek(segy_file, 3600L, SEEK_SET); // Salta los primeros 3600 bytes que contienen el encabezado general

    // Lee y muestra algunos datos de la primera traza
    float trace_data[SAMPLES_PER_TRACE]; // Almacena los datos de la traza
    fseek(segy_file, TRACE_HEADER_SIZE, SEEK_CUR); // Salta el encabezado de la traza (240 bytes)
    fread(trace_data, sizeof(float), SAMPLES_PER_TRACE, segy_file); // Lee los datos de la traza

    // Imprime algunas muestras de la traza
    printf("Datos de la primera traza:\n");
    for (int i = 0; i < SAMPLES_PER_TRACE; ++i) {
        printf("Muestra %d: %.2f\n", i + 1, trace_data[i]);
    }

    fclose(segy_file); // Cierra el archivo SEGY
    return 0;
}
