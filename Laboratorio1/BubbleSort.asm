.data

	array: .word 1,5,6,4,9,7,10,11,15 # Arreglo desordenado
	tam: .word 4 #Tamanno del arreglo
	message1: .asciiz "El arreglo desordenado es: "
	message4: .asciiz "Ingrese la longitud del arreglo: "
	message3: .asciiz "Ingrese un numero para el arreglo: "
	space: .asciiz " "
	newLine: .asciiz "\n"
	message2: .asciiz "El arreglo ordenado es: "

.text

	main:
	
	jal length_array
	jal insert_array
	jal print_array
	
	#Fin main
	li $v0, 10
	syscall
	
	BubbleSort:
	
	
	
	jr $ra
	
	
	
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
		
	
	
	#Procedimiento para pedir la longitud del arreglo
	length_array:
	li $v0, 4
	la $a0, message4
	syscall
	
	li $v0, 5
	syscall
	sw $v0, tam			
	jr $ra
	
	#Procedimiento para imprimir el arreglo
	print_array:
		#indice
		addi $t0, $zero, 0
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

