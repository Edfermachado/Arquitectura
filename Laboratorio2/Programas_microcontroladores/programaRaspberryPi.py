import RPi.GPIO as GPIO
import time

# Configuración de pines
led_pin = 18  # Cambia este número por el pin al que está conectado tu LED

# Configurar modo de pines
GPIO.setmode(GPIO.BCM)
GPIO.setup(led_pin, GPIO.OUT)

# Función para encender y apagar el LED
def toggle_led(pin):
    GPIO.output(pin, GPIO.HIGH)  # Enciende el LED
    time.sleep(1)  # Espera 1 segundo
    GPIO.output(pin, GPIO.LOW)   # Apaga el LED

try:
    # Encender y apagar el LED en un bucle infinito
    while True:
        toggle_led(led_pin)

except KeyboardInterrupt:
    # Limpiar configuración de pines al salir
    GPIO.cleanup()