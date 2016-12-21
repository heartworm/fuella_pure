GCC=avr-gcc -mmcu=atmega328p
FLAGS=-Os -DF_CPU=16000000UL
OBJCOPY=avr-objcopy -O ihex -R .eeprom

AVRDUDE=avrdude -c arduino -p ATMEGA328P -P COM3 -U flash:w:

all: 
	$(GCC) $(FLAGS) -c -o main.o main.c
	$(GCC) $(FLAGS) -c -o lcd.o lcd.c
	$(GCC) main.o lcd.o -o main
	$(OBJCOPY) main main.hex

flash: 
	$(AVRDUDE)main.hex
	
clean:
	rm *.hex
	rm *.o
	rm main