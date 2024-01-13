#include <gui/screen1_screen/Screen1View.hpp>

#include <string.h>
#include <stm32l4xx_hal.h>

extern __IO uint8_t midi_titleBuffer[50];

Screen1View::Screen1View()
{

}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View::midiTitleReady(uint8_t value){

	while(value){
		textArea1Buffer[value] = midi_titleBuffer[value];
		value--;
	}
	textArea1Buffer[value] = midi_titleBuffer[value];
	textArea1.invalidate();

}
