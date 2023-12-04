#include <8051.h>
#define LED_PIN P1_0 // Suponiendo que el LED esta conectado al pin p1.0

main(){

	LED_PIN = 0; // Apaga el LED

	while(1){

		LED_PIN = 1; //Encender el LED

		for(int i = 0; i<30000;i++); // Pequeño retraso para ver el LED encendido

	LED_PIN = 0; // Apagar el LED

		for(int i = 0; i<30000;i++); // Pequeño retraso para ver el LED apagado

		}




}
