#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

#include "stm32l4xx_hal.h"


extern __IO uint8_t rx_counter;
extern __IO uint8_t midiTitleReady;


Model::Model() : modelListener(0)
{

}

void Model::tick()
{
	if(midiTitleReady){
		midiTitleReady = 0;
		modelListener -> midiTitleReady((uint8_t)rx_counter);
		rx_counter = 0;

	}
}
