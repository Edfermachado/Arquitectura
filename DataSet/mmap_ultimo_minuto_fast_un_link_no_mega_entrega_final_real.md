
# Lectura de un archivo sgy con Mmap

**Bibliotecas necesarias**

```c
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
```

2. Abrir el archivo SGY con la funcion open() y obtener su tamaño con la funcion Iseek();

```c
int fd = open("archivo.sgy"), O_RDONLY);
off_t size = Iseek(fd,0,SEEK_END);
```

3. Mapear el archivo en memoria con la funcion mmap() y obtener un puntero al inicio del archivo mapeado:

```c
void* ptr =mmap(NULL, size, PROT_READ, MAP_PRIVATE,fd, 0);
```

4. Acceder a los datos del archivo a través del puntero obtenido en el paso anterior. Por ejemplo, para leer el primer byte del archvio

```c
char primer_byte = *((char*)ptr);
```
5. Cuando se termine de utilizar el archivo mapeado en memoria, se debe liberar la memoria co la funcion munmap():

```c
munmap(ptr,size);
```

```c
Edwin Machado,José Rivero, 2023, todos los derechos reservados
```
