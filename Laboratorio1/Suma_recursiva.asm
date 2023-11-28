.data
	#modificar n para obtener el valor deseado
n:      .word 10        # Valor inicial de n # 
result: .asciiz "El resultado es: "

.text
main:

 
    # Asigna valores a los registros $a0 y $a1
    la $a0, n   # Carga la dirección de 'n' en $a0
    lw $v0, 0($a0) # Carga el valor de 'n' en $v0
    
    #Movemos el contenido de $a0 a $v0
    move $a0, $v0
    
    # Llamada a la función sum
    jal sum
    
    li $v0, 4  #Prepara al sistema para imprimir
    la $a0, result #Imprime el mensaje
    syscall
    
    li $v0, 1            # Prepara al sistema para imprimir
    move $a0, $t0	#Muestra el valor resultado por pantalla
    syscall

    # Salir del programa
    li $v0, 10           # Código de la llamada al sistema para salir (exit)
    syscall

sum:    
    slti $t2, $a0, 1     # Comprueba si n <= 0
    beq $a0, $zero, sum_exit # Salta a sum_exit si n <= 0
    add $a1, $a1, $a0    # Suma n incrimenta el acumulador "acc"
    addi $a0, $a0, -1    # Decrementa n
    j sum                # Salto a sum
sum_exit:
    add $t0, $a1, $zero  # Devuelve el valor de acc
    jr $ra               # Retorno de la rutina
    
    #son las 2:31am voy a soñar con ensamblador ):