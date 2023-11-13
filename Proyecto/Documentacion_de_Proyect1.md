# Documentación del codigo
### Este código es una simulación simple de diferentes tipos de caché utilizando tres estrategias: correspondencia directa, asociativa de dos vías y completamente asociativa. Aquí hay una explicación de cada función:

# Función main:
## Entrada de datos: 
Lee el número de casos de prueba (casosPrueba).

## Bucle externo: 
Realiza el siguiente proceso para cada caso de prueba:

a. Lee el número de consultas (num).

b. Realiza un bucle para cada consulta, leyendo las direcciones de los bloques (dirBloque).

c. Llama a las tres funciones de simulación con las direcciones de los bloques.

# Función correspondenciaDirecta:
## Inicialización: 
Inicializa los arreglos bloqueCacheVal y cacheCorrDir.
## Procesamiento: 
Calcula en qué bloque se puede guardar cada dirección utilizando el operador % y el número 4 (ya que se usan 4 bloques en correspondencia directa).

## Simulación:
 Verifica si la dirección está en la cache. Si está, imprime "ACIERTO"; de lo contrario, imprime "FALLO" y actualiza la cache.

## Función asociativaDeDosVias:
## Inicialización: 
Inicializa arreglos y variables necesarios.

## Procesamiento: 
Calcula el conjunto al que pertenece cada dirección utilizando el operador % y el número 2 (ya que se están utilizando dos conjuntos en la asociativa de dos vías).

## Simulación:
 Verifica si la dirección está en alguno de los conjuntos. Si está, imprime "ACIERTO"; de lo contrario, imprime "FALLO" y actualiza el conjunto correspondiente.

# Función completamenteAsociativa:
## Inicialización: 
Inicializa el conjunto y variables necesarios.

## Procesamiento:
 Utiliza la función esta para verificar si la
 dirección está en el conjunto.
## Simulación: 
Verifica si la dirección está en el conjunto. Si está, imprime "ACIERTO"; de lo contrario, imprime "FALLO" y actualiza el conjunto.
## Función esta:
### Entrada: 
Un conjunto y un valor.
## Salida: 
Retorna 1 si el valor está en el conjunto, 0 de lo contrario.

Esencialmente, este programa simula cómo funcionaría una caché con diferentes estrategias de organización en respuesta a consultas de direcciones de bloques. Los mensajes "ACIERTO" indican que la dirección estaba en la caché, mientras que "FALLO" indica que la dirección no estaba en la caché y se realizó una actualización. 

# Estudiantes

José Rivero C.I:  28.492.353

Edwin Machado C.I: 30.532.641