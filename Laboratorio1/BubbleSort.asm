.data

	array: .word 1 5 6 4 9 7 10 11 15 # Arreglo desordenado
	tam: .word 9 #Tamanno del arreglo
	message1: .asciiz "El arreglo desordenado es: \n"
	message3: .asciiz "Ingrese un numero para el arreglo: "
	newLine: .asciiz "\n"
	message2: .asciiz "El arreglo ordenado es: \n"
	message4: .asciiz "Ingrese la longitud del arreglo: "
	space:.asciiz  " " 

.text

main:
	#Procedimiento para pedir el tamanno del arreglo
	jal length_array
	#Procedimiento para pedir la cantidad de elementos del arreglo
	jal insert_array
	
	jal inicializar
	 
	
	#Imprime el arreglo desordenado
	la   $a0, message1    
	li   $v0, 4          
	syscall              
	#Llama al procedimiento imprimir
	jal print_array
	
	#Imprime una nueva linea xd
	la   $a0, newLine
	li   $v0, 4
	syscall  
	
	jal inicializar #Reinicializa $a1, $a0
	
	
	#Llama al procedimiento Burbuja
	jal BubbleSort #ordena el array
	
	#Imprime el mensaje de que esta ordenado el arreglo
	la   $a0, message2    
	li   $v0, 4
	syscall
	
	jal inicializar #Reinicializa $a1, $a0
	#Imprime el arreglo ordenado
	jal print_array
	
	#Fin main
	li $v0, 10
	syscall


inicializar:
	la $a1, array # Carga el array en el registro $a1
	la $a0, tam # Carga el tamanno en el registro $a0
	lw $a0, 0($a0)

jr $ra
#Procedimiento para pedir la longitud del arreglo
	length_array:
	li $v0, 4
	la $a0, message4
	syscall
	
	li $v0, 5
	syscall
	sw $v0, tam			
	jr $ra
	

#Procedimiento para insertar elementos
insert_array:
	
	#se que se repite, pero me da problemas encapsular este fragmento en un funcion ):
	addi $t2, $zero, 0
	#Estas lineas de codigo es para multiplicar 4 por la cantidad de elementos del array y que el indice se mueva correctamente
	lw $t3, tam
	addi $t2, $t2 , 4 
	mult $t2, $t3
	mflo $t3
	move $t2, $zero
	while2:
	
	beq $t2,$t3,exit2
	#imprimir mensaje para pedir numero
	li $v0, 4
	la $a0, message3
	syscall
	#recibir numero por teclado
	li $v0, 5
	syscall
	
	sw $v0, array($t2) #guardar el numero en el arreglo
	
	addi $t2, $t2 ,4 #aumentar contador
	j while2
	
	exit2:

	jr $ra

print_array:
		#Reinicializamos los registros $t0 y $t1
		move $t0, $zero
		move $t1, $zero
		#Estas lineas de codigo es para multiplicar 4 por la cantidad de elementos del array y que el indice se mueva correctamente
		lw $t2, tam
		addi $t1, $t1 , 4 
		mult $t1, $t2
		mflo $t1
	while:
		beq $t0, $t1, exit # El segundo argumento de beq es el resultado de la multiplicaion de la 		
		lw $t6, array($t0)		
		addi $t0, $t0, 4		
		#imprimimos el numero actual
		li $v0,1
		move $a0, $t6
		syscall		
		#Imprimimos un espacio para tener el arreglo bonito xd
		li $v0, 4
		la $a0, space
		syscall		
		j while
	exit: 
    jr $ra


BubbleSort:
	addi $sp, $sp, -12 	# Hace espacio en la pila
	sw $s0, 8($sp)		# Guarda $s0
	sw $s1, 4($sp)		# Guarda $s1
	sw $s2, 0($sp)		# Guarda $s2
	sll $s2, $a0, 2		# Guarda en el registro $s2 el tamanno del vector * 4
	add $s0, $zero, $zero	# El registro $s0 será i
	addi $s2, $s2, -4	# Se le resta un entero al tamanno en $s2 para no salirse del tamanno
for1:
	beq $s0, $s2, f_for2	# Si i = tam - 1 se sale del ciclo principal
	add $s1, $zero, $zero	# $s1 se inicializa en 0
for2:
	beq $s1, $s2, f_for1	# Si j = tam se sale del ciclo secundario
	add $t0, $a1, $s1	# la direccion principal del vector + j dan la direccion v[j]
	lw $t1, 0($t0)		# $t1 = v[j]
	lw $t2, 4($t0)		# $t2 = v[j+1]
	slt $t3, $t2, $t1	# $t3 = 1 si v[j+1] < j[j] sino $t3 = 0
	beq $t3, $zero, no_swap # Si $t3 = 0 evita intercambiar v[j] y v[j+1]
	sw $t2, 0($t0)		# Esta linea y la de abajo intercambian v[j] y v[j+1]
	sw $t1, 4($t0)
no_swap:
	addi $s1, $s1, 4	# j = j + 1
	j for2		
f_for1:
	addi $s0, $s0, 4	# i = i + 1
	j for1		
f_for2:
	lw $s2, 0($sp)		# Restaurar el valor original de $s2
	lw $s1, 4($sp)		# Restaurar el valor original de $s1
	lw $s0, 8($sp)		# Restaurar el valor original de $s0
	addi $sp, $sp, 12
	jr $ra	
