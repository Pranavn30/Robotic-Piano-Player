/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_touchgfx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "math.h"
#include "stdlib.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define maxLetters 50

#define TIM7_PRSC 12
//look up table for sin wave generation
#define LUT_SIZE 100
uint16_t SIN_LUT[LUT_SIZE];
const double AMP_PCENT = 90.0; //amplitude of sine wave (0- 100)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef hcrc;

DAC_HandleTypeDef hdac1;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c3;
I2C_HandleTypeDef hi2c4;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi1_tx;
DMA_HandleTypeDef hdma_spi2_tx;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim7;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
uint8_t RX_Buffer[5]; //"Interrupt!"
uint8_t TX_Buffer[] = "Hello World\r\n";
__IO uint8_t midi_titleBuffer[maxLetters] = "EECS 373 Robotic Piano Player";
__IO uint8_t rx_counter = 50;
__IO uint8_t midiTitleReady = 1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_TIM5_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_I2C3_Init(void);
static void MX_I2C4_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_CRC_Init(void);
static void MX_DAC1_Init(void);
static void MX_TIM7_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void fill_LUT(void)
{
	//creates a sine wave look up table centered at VREF/2
	double HALF_AMP = (AMP_PCENT/ 100.0) * 2047.0;//calculates a half amplitude given the desired volume
	uint16_t i = 0;
	for(; i < LUT_SIZE; ++i)
	{
		SIN_LUT[i] = (uint16_t) ( (sin( ( (double) i) * 360.0 / ((double) LUT_SIZE)  * 3.14159265/180.0) * HALF_AMP) + 2048.0);
	}
}

//sets compare value (period) for tim2
void setComp(uint32_t newComp)
{
	HAL_TIM_Base_Stop(&htim7);
	htim7.Instance = TIM7;
	htim7.Init.Prescaler = TIM7_PRSC;
	htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim7.Init.Period = newComp;
	htim7.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
	{
		Error_Handler();
	}

	HAL_TIM_Base_Start(&htim7);
}

//frequency = tim2_clk/tim2_prsclr/LUT_SIZE/tim2_prd
//desired tim2_prd = tm2_clk/tim2_prsclr/LUT_SIZE/frequency
//tim2_clk = 120 MHz, tim2_prsclr = 85, LUT_SIZE = 100
void changeNote(float frequency)
{
	setComp((uint32_t) (120000000.0f/TIM7_PRSC/((float)LUT_SIZE)/frequency) );
}

void speakNote(int noteNum)
{
	HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
	float arg = (float)((noteNum + 3 + 11 - 69)/12.0);
	float frequency = pow(2,arg)*440;
	//printf("frecuencia:  %f \n\r", (float)frequency);
	changeNote(frequency);
}

void stopSpeaker()
{
	HAL_DAC_Stop(&hdac1, DAC_CHANNEL_1);
}

//used to iterate through the LUT
void TIM7_ISR()
{
	static uint32_t index = 0;
	HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, SIN_LUT[index]);
	++index;
	if(index > LUT_SIZE) index = 0;
}


#define Motor1A GPIO_PIN_0
#define Motor1B GPIO_PIN_1
#define Motor2A GPIO_PIN_2
#define Motor2B GPIO_PIN_3

#define Motor3A GPIO_PIN_11
#define Motor3B GPIO_PIN_5
#define Motor4A GPIO_PIN_4
#define Motor4B GPIO_PIN_10


void forward(GPIO_TypeDef* GPIOX, uint16_t MotorA, uint16_t MotorB)
{
    HAL_GPIO_WritePin(GPIOX, MotorA, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOX, MotorB, GPIO_PIN_SET);
}

void backward(GPIO_TypeDef* GPIOX, uint16_t MotorA, uint16_t MotorB)
{
    HAL_GPIO_WritePin(GPIOX, MotorA, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOX, MotorB, GPIO_PIN_RESET);
}

void stop(GPIO_TypeDef* GPIOX, uint16_t MotorA, uint16_t MotorB)
{
    HAL_GPIO_WritePin(GPIOX, MotorA, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOX, MotorB, GPIO_PIN_RESET);
}

#define MAX_GROUP_SIZE 3 // Maximum number of notes in a group
#define MAX_NOTE_DISTANCE 3 // Maximum distance between notes in a group

#define sizeLimit 2000
#define noteColNum 5
#define lowestNote 36
#define highestNote 96
#define defaultRight 60
#define defaultLeft 43
#define DCDelay 250
#define INT_MAX 2147483647

int actuatorPosL[sizeLimit][2];
int actuatorPosL_len = 0;
int actuatorPosR[sizeLimit][2];
int actuatorPosR_len = 0;

int song_length = 0;
int read_notes = 0;

int reading_title = 0;
char title_data[50];
int title_index = 0;

int actuatorL = 0;
int actuatorR = 0;

int DCLeftStart = 0;
int DCLeftStop = 0;
int DCRightStart = 0;
int DCRightStop = 0;

int note_dataL[sizeLimit][noteColNum];
int note_dataL_len = 0;
int note_dataR[sizeLimit][noteColNum];
int note_dataR_len = 0;

int note_dataR_stop[sizeLimit][2];
int note_dataL_stop[sizeLimit][2];

double current_pos_left = 0;
double current_pos_right = 0;

int readyToPlay = 0;

int formGroups(int noteArr[][noteColNum], int len);
int groupRange(int noteArr[][noteColNum], int len, int group1, int group2);
void groupMerge(int noteArr[][noteColNum], int len, int group1, int group2);
void groupContinuity(int noteArr[][noteColNum], int len);
void movementOps(int noteArr[][noteColNum], int len);
int assignActuatorPos(int actuatorPos[][2], int noteArr[][noteColNum], int note_len, int hand);
void splitStartStop(int noteArr[][noteColNum], int note_len, int noteArrStop[][2]);
void sortStop(int noteArr[][2], int note_len);
void assignDCAct(int actuatorPos[][2], int actuator_len, int noteArr[][noteColNum], int note_len);
void moveActuatorLeft(int note_num);
void moveActuatorRight(int note_num);
int nearestMinWhite(int noteNum);
int nearestMaxWhite(int noteNum);
int isBlackKey(int noteNum);
int setNextLinActuatorTime(int current_time);
void actuatorMoveDistLoop(double dist, int dir, int hand);
void GPIO_Activation_Right(int note);
void GPIO_Activation_Left(int note);
void GPIO_Deactivation_Right(int note);
void GPIO_Deactivation_Left(int note);
int setDCStart(int current_time);
int setDCStop(int current_time);
void setNextAction(int current_time);

int formGroups(int noteArr[][noteColNum], int len){
	movementOps(noteArr, len);
	int num_groups = noteArr[len - 1][3];
	for(int i = 0; i < num_groups; i++){
		if(groupRange(noteArr, len, i, i + 1) <= 11){
			groupMerge(noteArr, len, i, i + 1);
			num_groups--;
			groupContinuity(noteArr, len);
			i--;
		}
	}
	return 0;
}

// Given two group numbers, return the collective range
int groupRange(int noteArr[][noteColNum], int len, int group1, int group2){
	int minNote = 100;
	int maxNote = 0;
	for(int i = 0; i < len; i++){
		if(noteArr[i][3] == group1 || noteArr[i][3] == group2){
			if(noteArr[i][4] == 0){
				int min = nearestMinWhite(noteArr[i][0]);
				int max = nearestMaxWhite(noteArr[i][0]);
				if(min < minNote){
					minNote = min;
				}
				if(max > maxNote){
					maxNote = max;
				}
			}
		}
	}
	return maxNote - minNote;
}

// Identify the nearest white note under the argument note number
int nearestMinWhite(int noteNum){
	if(isBlackKey(noteNum)){
		return noteNum - 1;
	}
	return noteNum;
}

// Identify the nearest white note above the argument note number
int nearestMaxWhite(int noteNum){
	if(isBlackKey(noteNum)){
			return noteNum + 1;
		}
		return noteNum;
}


// Identify if a note is a black key
int isBlackKey(int noteNum){
	int isBlack = noteNum % 12;
	if(isBlack == 1 || isBlack == 3 || isBlack == 6 || isBlack == 8 || isBlack == 10){
		return 1;
	}
	return 0;
}
// Given two consecutive groups, combine them into one group using the lower group number; assumes group1 < group2
void groupMerge(int noteArr[][noteColNum], int len, int group1, int group2){
	for(int i = 0; i < len; i++){
		if(noteArr[i][3] == group2){
			noteArr[i][3] = group1;
		}
	}
}

// Reassign group numbers so they are sequential in value
void groupContinuity(int noteArr[][noteColNum], int len){
	int prevGroupNum = 0;
	int replaceGroup = 0;
	int newGroupNum = 0;
	int diff = 0;
	for(int i = 0; i < len; i++){
		diff = noteArr[i][3] - prevGroupNum;
		if(diff > 1 || diff < 0){
			replaceGroup = noteArr[i][3];
			newGroupNum = prevGroupNum + 1;
			while(noteArr[i][3] == replaceGroup && (i < len)){
				noteArr[i][3] = newGroupNum;
				i++;
			}
			i--;
		}
		prevGroupNum = noteArr[i][3];
	}
}

// Search for all movement opportunities and use them as divisions between groups
void movementOps(int noteArr[][noteColNum], int len){
	int current_group = 0;
	int time_max = noteArr[0][2];
	noteArr[0][3] = 0;
	for(int i = 1; i < len; i++){
		if(noteArr[i][1] > time_max){
			current_group++;
			noteArr[i][3] = current_group;
			time_max = noteArr[i][2];
		}
		else if(noteArr[i][2] > time_max){
			time_max = noteArr[i][2];
			noteArr[i][3] = current_group;
		}
		else{
			noteArr[i][3] = current_group;
		}
	}
}

// Assign actuator positions using a note data array
int assignActuatorPos(int actuatorPos[][2], int noteArr[][noteColNum], int note_len, int hand){
	if(note_len < 1){
		return 0;
	}
	int current_group = 0;
	int group_min_note = nearestMinWhite(noteArr[0][0]);
	int group_min_time = noteArr[0][1];
	for(int i = 0; i < note_len; i++){
		if(noteArr[i][3] != current_group){
			if(group_min_note > highestNote || group_min_note < lowestNote || (hand && group_min_note > defaultLeft) || (!hand && group_min_note < defaultRight)){
				if(hand){
					actuatorPos[current_group][0] = defaultLeft;
				}
				else{
					actuatorPos[current_group][0] = defaultRight;
				}
			}
			else{
				actuatorPos[current_group][0] = group_min_note;
			}
			actuatorPos[current_group][1] = group_min_time;
			if(!noteArr[i][4]){
				group_min_note = nearestMinWhite(noteArr[i][0]);
			}
			else{
				group_min_note = 100;
			}
			group_min_time = noteArr[i][1];
			current_group = noteArr[i][3];
		}
		else if(noteArr[i][0] < group_min_note && !noteArr[i][4]){
				group_min_note = nearestMinWhite(noteArr[i][0]);
		}
	}
	if(group_min_note > highestNote || group_min_note < lowestNote || (hand && group_min_note > defaultLeft) || (!hand && group_min_note < defaultRight)){
		if(hand){
			actuatorPos[current_group][0] = defaultLeft;
		}
		else{
			actuatorPos[current_group][0] = defaultRight;
		}
	}
	else{
		actuatorPos[current_group][0] = group_min_note;
	}
	actuatorPos[current_group][1] = group_min_time;

	return current_group + 1;
}

// Split start and stop times for notes
void splitStartStop(int noteArr[][noteColNum], int note_len, int noteArrStop[][2]){
	for(int i = 0; i < note_len; i++){
		noteArrStop[i][0] = noteArr[i][0];
		noteArrStop[i][1] = noteArr[i][2];
	}
}

// Sort stop array
void sortStop(int noteArr[][2], int note_len){
	int key_note, key_time, j;
	for(int i = 1; i < note_len; i++){
		key_note = noteArr[i][0];
		key_time = noteArr[i][1];
		j = i - 1;

		while(j >= 0 && noteArr[j][1] > key_time){
			noteArr[j + 1][0] = noteArr[j][0];
			noteArr[j + 1][1] = noteArr[j][1];
			j--;
		}
		noteArr[j + 1][0] = key_note;
		noteArr[j + 1][1] = key_time;
	}
}

// 12 indicates note should be played on speaker
int DCLookup[][12] = {
		{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{11, 12, 0, 1, 2, 4, 12, 5, 6, 7, 8, 9},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{9, 10, 11, 12, 0, 2, 3, 4, 12, 5, 6, 7},
		{7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 12, 5},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{5, 6, 7, 8, 9, 11, 12, 0, 1, 2, 3, 4},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{4, 12, 5, 6, 7, 9, 10, 11, 12, 0, 1, 2},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{2, 3, 4, 12, 5, 7, 8, 9, 10, 11, 12, 0},
};

// Assign DC actuator activations
void assignDCAct(int actuatorPos[][2], int actuator_len, int noteArr[][noteColNum], int note_len){
	int DCNum = 0;
	for(int i = 0; i < note_len; i++){
		if(!noteArr[i][4]){
			DCNum = DCLookup[actuatorPos[(noteArr[i][3])][0] % 12][noteArr[i][0] % 12];
			if(DCNum < 12){
				noteArr[i][0] = DCNum;
			}
		}

	}
}

int nextMove = 0;

// Sets TIM2 ARR so that it triggers for the next linear actuator movement
// Returns 0 if there is no next linear actuator movement
int setNextLinActuatorTime(int current_time){
	if(actuatorL >= actuatorPosL_len){
		if(actuatorR >= actuatorPosR_len){
			//HAL_TIM_Base_Stop_IT(&htim2);
			//return 0;
			return INT_MAX;
		}
		else{
			//TIM2->ARR = actuatorPosR[actuatorR][1] - current_time - 1;
			return actuatorPosR[actuatorR][1] - current_time;
		}
	}
	else if(actuatorR >= actuatorPosR_len){
		if(actuatorL >= actuatorPosL_len){
			//HAL_TIM_Base_Stop_IT(&htim2);
			//return 0;
		}
		else{
			//TIM2->ARR = actuatorPosL[actuatorL][1] - current_time - 1;
			return actuatorPosL[actuatorL][1] - current_time;
		}
	}
	else if(actuatorPosL[actuatorL][1] < actuatorPosR[actuatorR][1]){
		//TIM2->ARR = actuatorPosL[actuatorL][1] - current_time - 1;
		return actuatorPosL[actuatorL][1] - current_time;
	}
	else{
		//TIM2->ARR = actuatorPosR[actuatorR][1] - current_time - 1;
		return actuatorPosR[actuatorR][1] - current_time;
	}

	return 1;
}


// Sets TIM3 ARR so that it triggers for the next DC actuator push
// Returns 0 if there is no next DC actuator push
int setDCStart(int current_time){
	if(DCLeftStart >= note_dataL_len){
		if(DCRightStart >= note_dataR_len){
			HAL_TIM_Base_Stop_IT(&htim3);
			//return 0;
			return INT_MAX;
		}
		else{
			//TIM3->ARR = note_dataR[DCRightStart][1] - current_time - 1;
			return note_dataR[DCRightStart][1] - current_time;
		}
	}
	else if(DCRightStart >= note_dataR_len){
		if(DCLeftStart >= note_dataL_len){
			HAL_TIM_Base_Stop_IT(&htim3);
			//return 0;
			return INT_MAX;
		}
		else{
			//TIM3->ARR = note_dataL[DCLeftStart][1] - current_time - 1;
			return note_dataL[DCLeftStart][1] - current_time;
		}
	}
	else if(note_dataL[DCLeftStart][1] < note_dataR[DCRightStart][1]){
		//TIM3->ARR = note_dataL[DCLeftStart][1] - current_time - 1;
		return note_dataL[DCLeftStart][1] - current_time;
	}
	else{
		//TIM3->ARR = note_dataR[DCRightStart][1] - current_time - 1;
		return note_dataR[DCRightStart][1] - current_time;
	}
	return 1;
}

// Sets TIM4 ARR so that it triggers for the next DC actuator pull
// Returns 0 if there is no next DC actuator pull
int setDCStop(int current_time){
	if(DCLeftStop >= note_dataL_len){
		if(DCRightStop >= note_dataR_len){
			HAL_TIM_Base_Stop_IT(&htim4);
			// return 0;
			return INT_MAX;
		}
		else{
			//TIM4->ARR = note_dataR_stop[DCRightStop][1] - current_time - 1;
			return note_dataR_stop[DCRightStop][1] - current_time ;
		}
	}
	else if(DCRightStop >= note_dataR_len){
		if(DCLeftStop >= note_dataL_len){
			HAL_TIM_Base_Stop_IT(&htim4);
			//return 0;
			return INT_MAX;
		}
		else{
			//TIM4->ARR = note_dataL_stop[DCLeftStop][1] - current_time - 1;
			return note_dataL_stop[DCLeftStop][1] - current_time;
		}
	}
	else if(note_dataL_stop[DCLeftStop][1] < note_dataR_stop[DCRightStop][1]){
		//TIM4->ARR = note_dataL_stop[DCLeftStop][1] - current_time - 1;
		return note_dataL_stop[DCLeftStop][1] - current_time;
	}
	else{
		//TIM4->ARR = note_dataR_stop[DCRightStop][1] - current_time - 1;
		return note_dataR_stop[DCRightStop][1] - current_time;
	}
	return 1;
}

void setNextAction(int current_time){
	int nextLinAct = setNextLinActuatorTime(current_time);
	int nextDCPush = setDCStart(current_time);
	int nextDCPull = setDCStop(current_time);
	if(nextDCPull == INT_MAX){
		HAL_TIM_Base_Stop_IT(&htim2);
	}
	if(nextDCPull == nextLinAct && nextDCPull <= nextDCPush){
		nextMove = 2;
		if(nextDCPull == 0){
			nextDCPull++;
		}
		TIM2->ARR = nextDCPull;
	}
	else if(nextLinAct <= nextDCPush && nextDCPull > nextLinAct){
		nextMove = 0;
		if(nextLinAct == 0){
			nextLinAct++;
		}
		TIM2->ARR = nextLinAct;
	}
	else{
		if(nextDCPull < nextDCPush && nextDCPull < nextLinAct){
			nextMove = 2;
			if(nextDCPull == 0){
				nextDCPull++;
			}
			TIM2->ARR = nextDCPull;
		}
		else if(nextLinAct < nextDCPush && nextLinAct < nextDCPull){
			nextMove = 0;
			if(nextLinAct == 0){
				nextLinAct++;
			}
			TIM2->ARR = nextLinAct;
		}
		else if(nextDCPush < nextLinAct && nextDCPush < nextDCPull){
			nextMove = 1;
			if(nextDCPush == 0){
				nextDCPush++;
			}
			TIM2->ARR = nextDCPush;
		}
	}
}

int movements = 0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{

	if (htim==&TGFX_T){
	touchgfxSignalVSync();
	}

	else if(htim == &htim2){
	/*HAL_TIM_Base_Stop_IT(&htim2);
	HAL_TIM_Base_Stop_IT(&htim3);
	HAL_TIM_Base_Stop_IT(&htim4);*/
	TIM2->CR1 &= ~TIM_CR1_CEN;
	//TIM3->CR1 &= ~TIM_CR1_CEN;
	//TIM4->CR1 &= ~TIM_CR1_CEN;
	int current_time = 0;
	// Move actuator
	if(nextMove == 0){
		//if(!(DCRightStart > DCRightStop || DCLeftStart > DCLeftStop)){
			if(actuatorL >= actuatorPosL_len){
				moveActuatorRight(actuatorPosR[actuatorR][0]);
				current_time = actuatorPosR[actuatorR][1];
				actuatorR++;
			}
			else if(actuatorR >= actuatorPosR_len){
				moveActuatorLeft(actuatorPosL[actuatorL][0]);
				current_time = actuatorPosL[actuatorL][1];
				actuatorL++;
			}
			else if(actuatorPosL[actuatorL][1] == actuatorPosR[actuatorR][1]){
				moveActuatorRight(actuatorPosR[actuatorR][0]);
				moveActuatorLeft(actuatorPosL[actuatorL][0]);
				current_time = actuatorPosR[actuatorR][1];
				actuatorR++;
				actuatorL++;
			}
			else if(actuatorPosL[actuatorL][1] < actuatorPosR[actuatorR][1]){
				moveActuatorLeft(actuatorPosL[actuatorL][0]);
				current_time = actuatorPosL[actuatorL][1];
				actuatorL++;
			}
			else{
				moveActuatorRight(actuatorPosR[actuatorR][0]);
				current_time = actuatorPosR[actuatorR][1];
				actuatorR++;
			}
			//setNextLinActuatorTime(current_time);
			//setNextAction(current_time);
		//}
	}
	else if(nextMove == 1){
		if(DCRightStart >= note_dataR_len){
			current_time = note_dataL[DCLeftStart][1];
			while(note_dataL[DCLeftStart][1] == current_time && DCLeftStart < note_dataL_len){
				GPIO_Activation_Left(note_dataL[DCLeftStart][0]);
				DCLeftStart++;
			}
		}
		else if(DCLeftStart >= note_dataL_len){
			current_time = note_dataR[DCRightStart][1];
			while(note_dataR[DCRightStart][1] == current_time && DCRightStart < note_dataR_len){
				GPIO_Activation_Right(note_dataR[DCRightStart][0]);
				DCRightStart++;
			}
		}
		else if(note_dataR[DCRightStart][1] == note_dataL[DCLeftStart][1]){
			current_time = note_dataR[DCRightStart][1];

			while(note_dataR[DCRightStart][1] == current_time && DCRightStart < note_dataR_len){
				GPIO_Activation_Right(note_dataR[DCRightStart][0]);
				DCRightStart++;
			}
			while(note_dataL[DCLeftStart][1] == current_time && DCLeftStart < note_dataL_len){
				GPIO_Activation_Left(note_dataL[DCLeftStart][0]);
				DCLeftStart++;
			}
		}
		else if(note_dataR[DCRightStart][1] > note_dataL[DCLeftStart][1]){
			current_time = note_dataL[DCLeftStart][1];

			while(note_dataL[DCLeftStart][1] == current_time && DCLeftStart < note_dataL_len){
				GPIO_Activation_Left(note_dataL[DCLeftStart][0]);
				DCLeftStart++;
			}
		}
		else{
			current_time = note_dataR[DCRightStart][1];

			while(note_dataR[DCRightStart][1] == current_time && DCRightStart < note_dataR_len){
				GPIO_Activation_Right(note_dataR[DCRightStart][0]);
				DCRightStart++;
			}
		}
		//setDCStart(current_time);
		//setNextAction(current_time);
	}
	// Pull DC actuator
	else if(nextMove == 2){
		if(DCRightStop >= note_dataR_len){
			current_time = note_dataL_stop[DCLeftStop][1];
			while(note_dataL_stop[DCLeftStop][1] == current_time && DCLeftStop < note_dataL_len){
				GPIO_Deactivation_Left(note_dataL_stop[DCLeftStop][0]);
				DCLeftStop++;
			}
		}
		else if(DCLeftStop >= note_dataL_len){
			current_time = note_dataR_stop[DCRightStop][1];
			while(note_dataR_stop[DCRightStop][1] == current_time && DCRightStop < note_dataR_len){
				GPIO_Deactivation_Right(note_dataR_stop[DCRightStop][0]);
				DCRightStop++;
			}
		}
		else if(note_dataR_stop[DCRightStop][1] == note_dataL_stop[DCLeftStop][1]){
			current_time = note_dataR_stop[DCRightStop][1];

			while(note_dataR_stop[DCRightStop][1] == current_time && DCRightStop < note_dataR_len){
				GPIO_Deactivation_Right(note_dataR_stop[DCRightStop][0]);
				DCRightStop++;
			}
			while(note_dataL_stop[DCLeftStop][1] == current_time && DCLeftStop < note_dataL_len){
				GPIO_Deactivation_Left(note_dataL_stop[DCLeftStop][0]);
				DCLeftStop++;
			}
		}
		else if(note_dataR_stop[DCRightStop][1] > note_dataL_stop[DCLeftStop][1]){
			current_time = note_dataL_stop[DCLeftStop][1];

			while(note_dataL_stop[DCLeftStop][1] == current_time && DCLeftStop < note_dataL_len){
				GPIO_Deactivation_Left(note_dataL_stop[DCLeftStop][0]);
				DCLeftStop++;
			}
		}
		else{
			current_time = note_dataR_stop[DCRightStop][1];

			while(note_dataR_stop[DCRightStop][1] == current_time && DCRightStop < note_dataR_len){
				GPIO_Deactivation_Right(note_dataR_stop[DCRightStop][0]);
				DCRightStop++;
			}
		}
		//setDCStop(current_time);
		//setNextAction(current_time);
	}

	setNextAction(current_time);
	/*HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start_IT(&htim3);
	HAL_TIM_Base_Start_IT(&htim4);*/
	TIM2->CR1 |= TIM_CR1_CEN;
	//TIM3->CR1 |= TIM_CR1_CEN;
	//TIM4->CR1 |= TIM_CR1_CEN;
	}
}

const double stepPerRot = 200;
const double stepPerSec = 2000;
const double inPerRot = 2.83;
const double inPerSec = ((stepPerSec / stepPerRot) * inPerRot);

// Lookup Table Note Number to Distance on linear actuator
// May need to be a double for accuracy, depends on what units we decide on
// Current numbers are just example fillers

// absolute locations of both extremes for right and left hand respectively
// Currently, rightMin is c3 and leftMin is c2
const double rightMin = 0;
const double leftMin = 0;
const double whiteNoteDist = 0.93; // Distance between white notes in inches

// Index 0 = note number 48, Index 7 = note number 36
// **************************************************************
// NEED TO ADD ZERO FILLERS FOR BLACK NOTES (LEFT HAND ONLY)
// **************************************************************
const double numToDistLeft[] = {
	leftMin, 0, leftMin + whiteNoteDist, leftMin + whiteNoteDist * 2.0, 0, leftMin + whiteNoteDist * 3.0,
	0, leftMin + whiteNoteDist * 4.0
};

// Index 0 = note number 71, Index 15 = note number 96
const double numToDistRight[] = {
	rightMin, 0, rightMin + whiteNoteDist, 0, rightMin + whiteNoteDist * 2.0, rightMin + whiteNoteDist * 3.0, 0, rightMin + whiteNoteDist * 4.0, 0
	, rightMin + whiteNoteDist * 5.0, 0, rightMin + whiteNoteDist * 6.0, rightMin + whiteNoteDist * 7.0, 0, rightMin + whiteNoteDist * 8.0, 0
	, rightMin + whiteNoteDist * 9.0, rightMin + whiteNoteDist * 10.0, 0, rightMin + whiteNoteDist * 11.0, 0, rightMin + whiteNoteDist * 12.0, 0
	, rightMin + whiteNoteDist * 13.0, rightMin + whiteNoteDist * 14.0, 0, rightMin + whiteNoteDist * 15.0
};

/*// Move actuator a given distance in a given direction, takes in distance d argument in inches
void moveActuatorDistLeft(double d, int dir){
	int on_time = (int) ((d / inPerSec) * 1000);

	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, dir);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 0);

    if(on_time){
        HAL_Delay(on_time);
    }


    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 1);
}*/

/*// Move actuator a given distance in a given direction, takes in distance d argument in inches
void moveActuatorDistRight(double d, int dir){
	int on_time = (int) ((d / inPerSec) * 1000);

	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, dir);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 0);

    if(on_time){
    	HAL_Delay(on_time);
    }

    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 1);
}*/

// Move the left linear actuator to the correct position for the current note number
void moveActuatorLeft(int note_num){
	double d = numToDistLeft[abs((note_num - defaultLeft))];
	double delta_x = d - current_pos_left;

	if(delta_x > 0){
		// Set stepper direction appropriately
		actuatorMoveDistLoop(delta_x, 1, 0);
	}
	else{
		// Set stepper direction appropriately
		actuatorMoveDistLoop(delta_x * -1, 0, 0);
	}

	current_pos_left = d;
}

// Move the right linear actuator to the correct position for the current note number
void moveActuatorRight(int note_num){
	double d = numToDistRight[note_num - defaultRight];
	double delta_x = d - current_pos_right;

	if(delta_x > 0){
		// Set stepper direction appropriately
		actuatorMoveDistLoop(delta_x, 1, 1);
		//moveActuatorDistRight(delta_x, 1);
	}
	else{
		// Set stepper direction appropriately
		actuatorMoveDistLoop(delta_x * -1, 0, 1);
		//moveActuatorDistRight(delta_x * -1, 0);
	}

	current_pos_right = d;
}

void play_notes() {
	formGroups(note_dataR, note_dataR_len);
	formGroups(note_dataL, note_dataL_len);

	actuatorPosR_len = assignActuatorPos(actuatorPosR, note_dataR, note_dataR_len, 0);
	actuatorPosL_len = assignActuatorPos(actuatorPosL, note_dataL, note_dataL_len, 1);

	assignDCAct(actuatorPosR, actuatorPosR_len, note_dataR, note_dataR_len);
	assignDCAct(actuatorPosL, actuatorPosL_len, note_dataL, note_dataL_len);
	splitStartStop(note_dataR, note_dataR_len, note_dataR_stop);
	splitStartStop(note_dataL, note_dataL_len, note_dataL_stop);
	sortStop(note_dataR_stop, note_dataR_len);
	sortStop(note_dataL_stop, note_dataL_len);

	/*if(actuatorPosR_len > 0 || actuatorPosL_len > 0){
		setNextLinActuatorTime(0);
		TIM2->DIER = 0;
		TIM2->SR = 0;
		TIM2->EGR = 0;
		HAL_TIM_Base_Start_IT(&htim2);
	}*/

	/*if(song_length > 0){
		setDCStart(0);
		TIM3->DIER = 0;
		TIM3->SR = 0;
		TIM3->EGR = 0;
		HAL_TIM_Base_Start_IT(&htim3);

		setDCStop(0);
		TIM4->DIER = 0;
		TIM4->SR = 0;
		TIM4->EGR = 0;
		HAL_TIM_Base_Start_IT(&htim4);
	}*/

	// *************************
	// THIS IS THE START SEQUENCE
	// *************************

	readyToPlay = 1;
}

// hand = 0 is left, hand = 1 is right
void actuatorMoveDistLoop(double dist, int dir, int hand){
	int steps = (int) ((dist / inPerRot) * stepPerRot);

	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, dir);
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_0, 0);

	if(hand){
		HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 0);
	}
	else{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, 0);
	}

	if(steps){
		for(int i = 0; i < steps; i++){
			HAL_GPIO_WritePin(GPIOG, GPIO_PIN_0, 1);
			HAL_Delay(1);
			HAL_GPIO_WritePin(GPIOG, GPIO_PIN_0, 0);
			//********************************************
			// CHANGE SPEED BY CHANGING HAL_Delay BELOW
			//********************************************
			HAL_Delay(2);
		}
	}

	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, 1);
}

// Test Function to activate the very fist DC actuator that can be used for calibration and to assert that I2C is working
void ON1_PCF8574()
{
	uint8_t data_write1[1] = {0x01};
	uint8_t data_write2[1] = {0x00};
	uint8_t data_write3[1] = {0x02};
	HAL_I2C_Master_Transmit(&hi2c3, (0x20<<1), &data_write1[0], 1, 1000);
	HAL_Delay(200);
	HAL_I2C_Master_Transmit(&hi2c3, (0x20<<1), &data_write2[0], 1, 1000);
	HAL_Delay(200);
	HAL_I2C_Master_Transmit(&hi2c3, (0x20<<1), &data_write3[0], 1, 1000);
	HAL_Delay(200);
}

void ON2_PCF8574()
{
	uint8_t data_write1[1] = {0x04};
	uint8_t data_write2[1] = {0x00};
	uint8_t data_write3[1] = {0x08};
	HAL_I2C_Master_Transmit(&hi2c3, (0x20<<1), &data_write1[0], 1, 1000);
	HAL_Delay(200);
	HAL_I2C_Master_Transmit(&hi2c3, (0x20<<1), &data_write2[0], 1, 1000);
	HAL_Delay(200);
	HAL_I2C_Master_Transmit(&hi2c3, (0x20<<1), &data_write3[0], 1, 1000);
	HAL_Delay(200);
}

void ON3_PCF8574()
{
	uint8_t data_write1[1] = {0x10};
	uint8_t data_write2[1] = {0x00};
	uint8_t data_write3[1] = {0x20};
	HAL_I2C_Master_Transmit(&hi2c3, (0x20<<1), &data_write1[0], 1, 1000);
	HAL_Delay(200);
	HAL_I2C_Master_Transmit(&hi2c3, (0x20<<1), &data_write2[0], 1, 1000);
	HAL_Delay(200);
	HAL_I2C_Master_Transmit(&hi2c3, (0x20<<1), &data_write3[0], 1, 1000);
	HAL_Delay(200);
}

void GPIO_Activation_Right(int note){
     int note_num = note;

     uint8_t P01_ON[1] = {0x01};
     uint8_t P01_STOP[1] = {0x00};

     uint8_t P23_ON[1] = {0x04};
     uint8_t P23_STOP[1] = {0x00};

     uint8_t P45_ON[1] = {0x10};
     uint8_t P45_STOP[1] = {0x00};

     if (note_num == 0){
         HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &P01_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &P01_STOP[0], 1, 1000);
     } else if (note_num == 1){
         HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &P23_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &P23_STOP[0], 1, 1000);
     } else if (note_num == 2){
         HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &P45_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &P45_STOP[0], 1, 1000);
     } else if (note_num == 3){
         HAL_I2C_Master_Transmit(&hi2c1, (0x21<<1), &P01_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c1, (0x21<<1), &P01_STOP[0], 1, 1000);
     } else if (note_num == 4){
         HAL_I2C_Master_Transmit(&hi2c1, (0x21<<1), &P23_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c1, (0x21<<1), &P23_STOP[0], 1, 1000);
     } else if (note_num == 5){
         HAL_I2C_Master_Transmit(&hi2c1, (0x21<<1), &P45_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c1, (0x21<<1), &P45_STOP[0], 1, 1000);
     } else if (note_num == 6){
         HAL_I2C_Master_Transmit(&hi2c2, (0x20<<1), &P01_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c2, (0x20<<1), &P01_STOP[0], 1, 1000);
     } else if (note_num == 7){
         HAL_I2C_Master_Transmit(&hi2c2, (0x20<<1), &P23_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c2, (0x20<<1), &P23_STOP[0], 1, 1000);
       //  speakNote(67);
     } else if (note_num == 8){
         HAL_I2C_Master_Transmit(&hi2c2, (0x20<<1), &P45_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c2, (0x20<<1), &P45_STOP[0], 1, 1000);
     } else if (note_num == 9){
         HAL_I2C_Master_Transmit(&hi2c2, (0x21<<1), &P01_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c2, (0x21<<1), &P01_STOP[0], 1, 1000);
       //  speakNote(69);
     } else if (note_num == 10){
         HAL_I2C_Master_Transmit(&hi2c2, (0x21<<1), &P23_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c2, (0x21<<1), &P23_STOP[0], 1, 1000);
     } else if (note_num == 11){
         HAL_I2C_Master_Transmit(&hi2c2, (0x21<<1), &P45_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c2, (0x21<<1), &P45_STOP[0], 1, 1000);
      //   speakNote(71);
     } else if (note_num > 11){
         speakNote(note_num);
         HAL_Delay(2*DCDelay);
     }

}

void GPIO_Activation_Left(int note){
     int note_num = note;

     uint8_t P01_ON[1] = {0x01};
     uint8_t P01_STOP[1] = {0x00};

     uint8_t P23_ON[1] = {0x04};
     uint8_t P23_STOP[1] = {0x00};

     uint8_t P45_ON[1] = {0x10};
     uint8_t P45_STOP[1] = {0x00};

     if (note_num == 0){
         HAL_I2C_Master_Transmit(&hi2c3, (0x20<<1), &P01_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c3, (0x20<<1), &P01_STOP[0], 1, 1000);
     } else if (note_num == 1){
         HAL_I2C_Master_Transmit(&hi2c3, (0x20<<1), &P23_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c3, (0x20<<1), &P23_STOP[0], 1, 1000);
     } else if (note_num == 2){
         HAL_I2C_Master_Transmit(&hi2c3, (0x20<<1), &P45_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c3, (0x20<<1), &P45_STOP[0], 1, 1000);
     } else if (note_num == 3){
         HAL_I2C_Master_Transmit(&hi2c3, (0x21<<1), &P01_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c3, (0x21<<1), &P01_STOP[0], 1, 1000);
     } else if (note_num == 4){
         HAL_I2C_Master_Transmit(&hi2c3, (0x21<<1), &P23_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c3, (0x21<<1), &P23_STOP[0], 1, 1000);
     } else if (note_num == 5){
         HAL_I2C_Master_Transmit(&hi2c3, (0x21<<1), &P45_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c3, (0x21<<1), &P45_STOP[0], 1, 1000);
     } else if (note_num == 6){
         HAL_I2C_Master_Transmit(&hi2c4, (0x20<<1), &P01_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c4, (0x20<<1), &P01_STOP[0], 1, 1000);
     } else if (note_num == 7){
         HAL_I2C_Master_Transmit(&hi2c4, (0x20<<1), &P23_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c4, (0x20<<1), &P23_STOP[0], 1, 1000);
     } else if (note_num == 8){
         HAL_I2C_Master_Transmit(&hi2c4, (0x20<<1), &P45_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c4, (0x20<<1), &P45_STOP[0], 1, 1000);
     } else if (note_num == 9){
         HAL_I2C_Master_Transmit(&hi2c4, (0x21<<1), &P01_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c4, (0x21<<1), &P01_STOP[0], 1, 1000);
     } else if (note_num == 10){
         HAL_I2C_Master_Transmit(&hi2c4, (0x21<<1), &P23_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c4, (0x21<<1), &P23_STOP[0], 1, 1000);
     } else if (note_num == 11){
         HAL_I2C_Master_Transmit(&hi2c4, (0x21<<1), &P45_ON[0], 1, 1000);
         HAL_Delay(DCDelay);
         HAL_I2C_Master_Transmit(&hi2c4, (0x21<<1), &P45_STOP[0], 1, 1000);
     } else if (note_num > 11){
         speakNote(note_num);
     }

}

void GPIO_Deactivation_Right(int note){
     int note_num = note;

     uint8_t P01_OFF[1] = {0x02};
     uint8_t P01_STOP[1] = {0x00};

     uint8_t P23_OFF[1] = {0x08};
     uint8_t P23_STOP[1] = {0x00};

     uint8_t P45_OFF[1] = {0x20};
     uint8_t P45_STOP[1] = {0x00};

     if (note_num == 0){
              HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &P01_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &P01_STOP[0], 1, 1000);
          } else if (note_num == 1){
              HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &P23_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &P23_STOP[0], 1, 1000);
          } else if (note_num == 2){
              HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &P45_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &P45_STOP[0], 1, 1000);
          } else if (note_num == 3){
              HAL_I2C_Master_Transmit(&hi2c1, (0x21<<1), &P01_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c1, (0x21<<1), &P01_STOP[0], 1, 1000);
          } else if (note_num == 4){
              HAL_I2C_Master_Transmit(&hi2c1, (0x21<<1), &P23_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c1, (0x21<<1), &P23_STOP[0], 1, 1000);
          } else if (note_num == 5){
              HAL_I2C_Master_Transmit(&hi2c1, (0x21<<1), &P45_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c1, (0x21<<1), &P45_STOP[0], 1, 1000);
          } else if (note_num == 6){
              HAL_I2C_Master_Transmit(&hi2c2, (0x20<<1), &P01_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c2, (0x20<<1), &P01_STOP[0], 1, 1000);
          } else if (note_num == 7){
              HAL_I2C_Master_Transmit(&hi2c2, (0x20<<1), &P23_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c2, (0x20<<1), &P23_STOP[0], 1, 1000);
              stopSpeaker();
          } else if (note_num == 8){
              HAL_I2C_Master_Transmit(&hi2c2, (0x20<<1), &P45_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c2, (0x20<<1), &P45_STOP[0], 1, 1000);
          } else if (note_num == 9){
              HAL_I2C_Master_Transmit(&hi2c2, (0x21<<1), &P01_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c2, (0x21<<1), &P01_STOP[0], 1, 1000);
              stopSpeaker();
          } else if (note_num == 10){
              HAL_I2C_Master_Transmit(&hi2c2, (0x21<<1), &P23_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c2, (0x21<<1), &P23_STOP[0], 1, 1000);

          } else if (note_num == 11){
              HAL_I2C_Master_Transmit(&hi2c2, (0x21<<1), &P45_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c2, (0x21<<1), &P45_STOP[0], 1, 1000);
              stopSpeaker();
          } else if (note_num > 11){
              stopSpeaker();
          }
}

void GPIO_Deactivation_Left(int note){
     int note_num = note;

     uint8_t P01_OFF[1] = {0x02};
     uint8_t P01_STOP[1] = {0x00};

     uint8_t P23_OFF[1] = {0x08};
     uint8_t P23_STOP[1] = {0x00};

     uint8_t P45_OFF[1] = {0x20};
     uint8_t P45_STOP[1] = {0x00};

     if (note_num == 0){
              HAL_I2C_Master_Transmit(&hi2c3, (0x20<<1), &P01_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c3, (0x20<<1), &P01_STOP[0], 1, 1000);
          } else if (note_num == 1){
              HAL_I2C_Master_Transmit(&hi2c3, (0x20<<1), &P23_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c3, (0x20<<1), &P23_STOP[0], 1, 1000);
          } else if (note_num == 2){
              HAL_I2C_Master_Transmit(&hi2c3, (0x20<<1), &P45_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c3, (0x20<<1), &P45_STOP[0], 1, 1000);
          } else if (note_num == 3){
              HAL_I2C_Master_Transmit(&hi2c3, (0x21<<1), &P01_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c3, (0x21<<1), &P01_STOP[0], 1, 1000);
          } else if (note_num == 4){
              HAL_I2C_Master_Transmit(&hi2c3, (0x21<<1), &P23_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c3, (0x21<<1), &P23_STOP[0], 1, 1000);
          } else if (note_num == 5){
              HAL_I2C_Master_Transmit(&hi2c3, (0x21<<1), &P45_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c3, (0x21<<1), &P45_STOP[0], 1, 1000);
          } else if (note_num == 6){
              HAL_I2C_Master_Transmit(&hi2c4, (0x20<<1), &P01_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c4, (0x20<<1), &P01_STOP[0], 1, 1000);
          } else if (note_num == 7){
              HAL_I2C_Master_Transmit(&hi2c4, (0x20<<1), &P23_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c4, (0x20<<1), &P23_STOP[0], 1, 1000);
          } else if (note_num == 8){
              HAL_I2C_Master_Transmit(&hi2c4, (0x20<<1), &P45_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c4, (0x20<<1), &P45_STOP[0], 1, 1000);
          } else if (note_num == 9){
              HAL_I2C_Master_Transmit(&hi2c4, (0x21<<1), &P01_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c4, (0x21<<1), &P01_STOP[0], 1, 1000);
          } else if (note_num == 10){
              HAL_I2C_Master_Transmit(&hi2c4, (0x21<<1), &P23_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c4, (0x21<<1), &P23_STOP[0], 1, 1000);
          } else if (note_num == 11){
              HAL_I2C_Master_Transmit(&hi2c4, (0x21<<1), &P45_OFF[0], 1, 1000);
              HAL_Delay(DCDelay);
              HAL_I2C_Master_Transmit(&hi2c4, (0x21<<1), &P45_STOP[0], 1, 1000);
          } else if (note_num > 11){
              stopSpeaker();
          }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_TIM5_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_I2C3_Init();
  MX_I2C4_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_CRC_Init();
  MX_DAC1_Init();
  MX_TIM7_Init();
  MX_TouchGFX_Init();
  /* USER CODE BEGIN 2 */
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
	HAL_Delay(3000);
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);

	fill_LUT();

	HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0x0);
	HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
	stopSpeaker();
	HAL_TIM_Base_Start_IT(&htim7);

  Displ_Init(Displ_Orientat_270);		// initialize the display and set the initial display orientation (here is orientaton: 0°) - THIS FUNCTION MUST PRECEED ANY OTHER DISPLAY FUNCTION CALL.
   touchgfxSignalVSync();		// asks TouchGFX to start initial display drawing
   Displ_BackLight('I');  			// initialize backlight and turn it on at init level

  //********************************************************
  // Might need to move this to where ever we start the song
  //********************************************************

  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_4);
  TIM5->CCR4 = 25;

  HAL_UART_Receive_IT(&huart3, RX_Buffer, sizeof(RX_Buffer));
  HAL_UART_Transmit_IT(&huart3, TX_Buffer, sizeof(TX_Buffer));
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 1);

  /*uint8_t P01_OFF[1] = {0x02};
  uint8_t P01_STOP[1] = {0x00};*/
  /*HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &P01_OFF[0], 1, 1000);
  HAL_Delay(150);
  HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &P01_STOP[0], 1, 1000);*/

  /*HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 1);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, 1);
  //moveActuatorDistRight(0.93, 1);
  HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &data_write3[0], 1, 1000);
  HAL_Delay(200);
  HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &data_write2[0], 1, 1000);
  HAL_Delay(200);
  actuatorMoveDistLoop(0.93 * 7, 1);
  HAL_Delay(100);
  ON1_PCF8574();
  actuatorMoveDistLoop(0.93 * 7, 1);
  HAL_Delay(100);
  ON1_PCF8574();
  actuatorMoveDistLoop(0.93 * 7, 0);
  HAL_Delay(100);
  ON1_PCF8574();
  actuatorMoveDistLoop(0.93 * 7, 0);
  HAL_Delay(100);
  ON1_PCF8574();
  HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &data_write3[0], 1, 1000);
   HAL_Delay(200);
   HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &data_write2[0], 1, 1000);
   HAL_Delay(200);*/

  /*moveActuatorDist(16.25, 1);
  HAL_Delay(100);
  forward(GPIOA, Motor1A, Motor1B);
  forward(GPIOA, Motor2A, Motor2B);
  HAL_Delay(150);
  backward(GPIOA, Motor1A, Motor1B);
  backward(GPIOA, Motor2A, Motor2B);
  HAL_Delay(150);
  forward(GPIOA, Motor1A, Motor1B);
  forward(GPIOA, Motor2A, Motor2B);
  HAL_Delay(150);
  backward(GPIOA, Motor1A, Motor1B);
  backward(GPIOA, Motor2A, Motor2B);
  HAL_Delay(100);

  moveActuatorDist(16.25, 0);
  HAL_Delay(100);
  forward(GPIOA, Motor1A, Motor1B);
  forward(GPIOA, Motor2A, Motor2B);
  HAL_Delay(100);
  backward(GPIOA, Motor1A, Motor1B);
  backward(GPIOA, Motor2A, Motor2B);

  stop(GPIOA, Motor1A, Motor1B);
  stop(GPIOA, Motor2A, Motor2B);*/
  /*HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 1);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, 1);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 0);
  for(int i = 0; i < 200; i++){
  	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_0, 1);
  	HAL_Delay(1);
  	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_0, 0);
  	HAL_Delay(199);
  }
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 1);*/

  /*HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 1);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, 1);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 0);
  HAL_Delay(1000);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 1);*/
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
   int pressed = 0;
   int start = 0;
   int playingSong = 0;
   int prevTouch = 0;
   int currentTouch = 0;


  	  GPIO_Activation_Right(11);
  	  GPIO_Deactivation_Right(11);

  	  GPIO_Activation_Right(10);
  	  GPIO_Deactivation_Right(10);

  	  GPIO_Activation_Right(9);
  	  GPIO_Deactivation_Right(9);

  	  GPIO_Activation_Right(8);
  	  GPIO_Deactivation_Right(8);

  	  GPIO_Activation_Right(7);
  	  GPIO_Deactivation_Right(7);

  	  GPIO_Activation_Right(6);
  	  GPIO_Deactivation_Right(6);

  	  GPIO_Activation_Right(5);
  	  GPIO_Deactivation_Right(5);

  	  GPIO_Activation_Right(4);
  	  GPIO_Deactivation_Right(4);

  	  GPIO_Activation_Right(3);
  	  GPIO_Deactivation_Right(3);

  	  GPIO_Activation_Right(2);
  	  GPIO_Deactivation_Right(2);

  	  GPIO_Activation_Right(1);
  	  GPIO_Deactivation_Right(1);

  	  GPIO_Activation_Right(0);
  	  GPIO_Deactivation_Right(0);


  	  GPIO_Activation_Right(59);
  	  GPIO_Deactivation_Right(59);

  	  GPIO_Activation_Right(58);
  	  GPIO_Deactivation_Right(58);

  	  GPIO_Activation_Right(57);
  	  GPIO_Deactivation_Right(57);

  	  GPIO_Activation_Right(56);
  	  GPIO_Deactivation_Right(56);

  	  GPIO_Activation_Right(55);
  	  GPIO_Deactivation_Right(55);

  	GPIO_Activation_Left(11);
  	GPIO_Deactivation_Left(11);

  	GPIO_Activation_Left(10);
  	GPIO_Deactivation_Left(10);

  	GPIO_Activation_Left(9);
  	GPIO_Deactivation_Left(9);

  	GPIO_Activation_Left(8);
  	GPIO_Deactivation_Left(8);

  	GPIO_Activation_Left(7);
  	GPIO_Deactivation_Left(7);

  	GPIO_Activation_Left(6);
  	GPIO_Deactivation_Left(6);

  	GPIO_Activation_Left(5);
  	GPIO_Deactivation_Left(5);

  	GPIO_Activation_Left(4);
  	GPIO_Deactivation_Left(4);

  	GPIO_Activation_Left(3);
  	GPIO_Deactivation_Left(3);

  	GPIO_Activation_Left(2);
  	GPIO_Deactivation_Left(2);

  	GPIO_Activation_Left(1);
  	GPIO_Deactivation_Left(1);

  	GPIO_Activation_Left(0);
  	GPIO_Deactivation_Left(0);

/*


  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 1);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, 1);
  while (1)
  {
	  if(start){
			  touchgfxSignalVSync();
		  }
	  	  currentTouch = Touch_In_XY_area(120,  140,138,50);
		  if(currentTouch && !prevTouch){
			  pressed = !pressed;
			  if(start == 1){
		  	  touchgfxSignalVSync();		// asks TouchGFX to handle touch and redraw screen
		  	  if(playingSong == 0 && readyToPlay){
		  	    setNextAction(0);
		  		TIM2->DIER = 0;
		  		TIM2->SR = 0;
		  		TIM2->EGR = 0;
		  		TIM2->CNT = 0;
		  		HAL_TIM_Base_Start_IT(&htim2);
		  		playingSong = 1;
		  	  }
		  	  else if(playingSong == 1){
		  		  HAL_TIM_Base_Stop_IT(&htim2);
		  		GPIO_Deactivation_Right(0);
		  		GPIO_Deactivation_Right(1);
		  		GPIO_Deactivation_Right(2);
		  		GPIO_Deactivation_Right(3);
		  		GPIO_Deactivation_Right(4);
		  		GPIO_Deactivation_Right(5);
		  		GPIO_Deactivation_Right(6);
		  		GPIO_Deactivation_Right(7);
		  		GPIO_Deactivation_Right(8);
		  		GPIO_Deactivation_Right(9);
		  		GPIO_Deactivation_Right(10);
		  		GPIO_Deactivation_Right(11);
		  		GPIO_Deactivation_Left(0);
		  		GPIO_Deactivation_Left(1);
		  		GPIO_Deactivation_Left(2);
		  		GPIO_Deactivation_Left(3);
		  		GPIO_Deactivation_Left(4);
		  		GPIO_Deactivation_Left(5);
		  		GPIO_Deactivation_Left(6);
		  		GPIO_Deactivation_Left(7);
		  		GPIO_Deactivation_Left(8);
		  		GPIO_Deactivation_Left(9);
		  		GPIO_Deactivation_Left(10);
		  		GPIO_Deactivation_Left(11);
		  		  moveActuatorRight(defaultRight);
		  		  moveActuatorLeft(defaultLeft);
		  		  actuatorPosL_len = 0;
		  		  actuatorPosR_len = 0;

		  		song_length = 0;
		  		read_notes = 0;

		  		reading_title = 0;
		  		title_index = 0;

		  		actuatorL = 0;
		  		actuatorR = 0;

		  		DCLeftStart = 0;
		  		DCLeftStop = 0;
		  		DCRightStart = 0;
		  		DCRightStop = 0;

		  		note_dataL_len = 0;
		  		note_dataR_len = 0;

		  		playingSong = 0;
		  		HAL_UART_Receive_IT(&huart3, RX_Buffer, sizeof(RX_Buffer));
		  		readyToPlay = 0;
		  	  }
			  }
			  start = 1;
		  }
		  prevTouch = currentTouch;


    /* USER CODE END WHILE */

  MX_TouchGFX_Process();
    /* USER CODE BEGIN 3 */
	  /*moveActuatorLeft(5, 1);
	  HAL_Delay(1000);
	  moveActuatorLeft(5, 0);
	  HAL_Delay(1000);*/


	  /*actuatorMoveDistLoop(10, 1, 1);
	  HAL_Delay(1000);
	  actuatorMoveDistLoop(10, 0, 1);
	  HAL_Delay(1000);*/

	  /*actuatorMoveDistLoop(5, 1, 0);
	  HAL_Delay(1000);
	  actuatorMoveDistLoop(5, 0, 0);
	  HAL_Delay(1000);*/





	  /*GPIO_Activation_Left(0);
	  	  GPIO_Deactivation_Left(0);
	  	  GPIO_Activation_Left(1);
	  	  GPIO_Deactivation_Left(1);
	  	  GPIO_Activation_Left(2);
	  	  GPIO_Deactivation_Left(2);
	  	  GPIO_Activation_Left(3);
	  	  GPIO_Deactivation_Left(3);
	  	  GPIO_Activation_Left(4);
	  	  GPIO_Deactivation_Left(4);
	  	  GPIO_Activation_Left(5);
	  	  GPIO_Deactivation_Left(5);
	  	  GPIO_Activation_Left(6);
	  	  GPIO_Deactivation_Left(6);
	  	  GPIO_Activation_Left(7);
	  	  GPIO_Deactivation_Left(7);
	  	  GPIO_Activation_Left(8);
	  	  GPIO_Deactivation_Left(8);
	  	  GPIO_Activation_Left(9);
	  	  GPIO_Deactivation_Left(9);
	  	  GPIO_Activation_Left(10);
	  	  GPIO_Deactivation_Left(10);
	  	  GPIO_Activation_Left(11);
	  	  GPIO_Deactivation_Left(11);*/

	  /*ON1_PCF8574();
	  ON2_PCF8574();
	  ON3_PCF8574();*/


//}

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 60;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief DAC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC1_Init(void)
{

  /* USER CODE BEGIN DAC1_Init 0 */

  /* USER CODE END DAC1_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC1_Init 1 */

  /* USER CODE END DAC1_Init 1 */

  /** DAC Initialization
  */
  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_HighFrequency = DAC_HIGH_FREQUENCY_INTERFACE_MODE_ABOVE_80MHZ;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC1_Init 2 */

  /* USER CODE END DAC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x307075B1;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x307075B1;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.Timing = 0x307075B1;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief I2C4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C4_Init(void)
{

  /* USER CODE BEGIN I2C4_Init 0 */

  /* USER CODE END I2C4_Init 0 */

  /* USER CODE BEGIN I2C4_Init 1 */

  /* USER CODE END I2C4_Init 1 */
  hi2c4.Instance = I2C4;
  hi2c4.Init.Timing = 0x307075B1;
  hi2c4.Init.OwnAddress1 = 0;
  hi2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c4.Init.OwnAddress2 = 0;
  hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c4.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C4_Init 2 */

  /* USER CODE END I2C4_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 59999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 10000;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 100;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 39999;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 39;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 49;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */
  HAL_TIM_MspPostInit(&htim5);

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 12;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 90;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_EnableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  HAL_PWREx_EnableVddIO2();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7|GPIO_PIN_8|DISPL_RST_Pin|DISPL_DC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_10
                          |GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DISPL_CS_GPIO_Port, DISPL_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_0|GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(TOUCH_CS_GPIO_Port, TOUCH_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pins : PE2 PE3 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_SAI1;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PF7 PF8 DISPL_RST_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|DISPL_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : PC1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PC4 PC5 PC8 PC10
                           PC11 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_10
                          |GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : DISPL_CS_Pin DISPL_DC_Pin */
  GPIO_InitStruct.Pin = DISPL_CS_Pin|DISPL_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PG0 PG6 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : TOUCH_INT_Pin */
  GPIO_InitStruct.Pin = TOUCH_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(TOUCH_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PE7 PE8 PE9 PE10
                           PE12 PE13 */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_12|GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : TOUCH_CS_Pin */
  GPIO_InitStruct.Pin = TOUCH_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(TOUCH_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PE14 PE15 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF3_TIM1_COMP1;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_SAI2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB14 */
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_TIM15;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PG7 PG8 */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF8_LPUART1;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : PC6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_SAI2;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PC9 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA8 PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PD0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PG9 PG10 */
  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : PB3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	//__NOP(); // for debugging
	//Commented out ->Instance
		if (huart->Instance == USART3)
	    {

	        // RX_Buffer[0] = note number
	        // RX_Buffer[1] = start time upper half, RX_Buffer[2] = start time lower half
	        // RX_Buffer[3] = end time upper half, RX_Buffer[4] = end time lower half
	        int noteNum = RX_Buffer[0];
	        int start_Time = (RX_Buffer[1] << 8) + RX_Buffer[2];
	        int end_Time = (RX_Buffer[3] << 8) + RX_Buffer[4];
	        if(noteNum == 1 && start_Time == end_Time){
	        	// Starting to send title
	        	reading_title = 1;
	        	title_index = 0;
	        }
	        else if(noteNum == 2 && start_Time == end_Time){
	        	// Title done, do other stuff
	        	reading_title = 0;
	        }
	        else if(reading_title){
	        	for(int i = 0; i < 5; i++){
	        		midi_titleBuffer[title_index + i] = RX_Buffer[i];
	        	}
	        	title_index += 5;
	        }
	        // noteNum = 0 and start_Time = end_Time indicates the beginning of a song
	        // noteNum = 255 and start_time = end_Time indicates the end of a song
	        if(noteNum == 3 && start_Time == end_Time){
	        	song_length = 0;
	        	note_dataR_len = 0;
	        	note_dataL_len = 0;
	        	read_notes = 1;
	        }
	        else if (noteNum == 4 && start_Time == end_Time){
	        	note_dataR[note_dataR_len][0] = 0;
	            note_dataR[note_dataR_len][1] = 0;
	            note_dataR[note_dataR_len][2] = 0;
	            note_dataL[note_dataL_len][0] = 0;
	            note_dataL[note_dataL_len][1] = 0;
	            note_dataL[note_dataL_len][2] = 0;
	        	song_length = note_dataR_len + note_dataL_len;
	            read_notes = 0;
	            play_notes();
	        }
	        else if(read_notes){
	        	if(noteNum >= 60){
	        		note_dataR[note_dataR_len][0] = noteNum;
	        		note_dataR[note_dataR_len][1] = start_Time + 2;
	        		note_dataR[note_dataR_len][2] = end_Time + 2;
	        		if(noteNum > highestNote){
	        			note_dataR[note_dataR_len][4] = 1;
	        		}
	        		else{
	        			note_dataR[note_dataR_len][4] = 0;
	        		}
	        		note_dataR_len++;
	        	}
	        	else{
	        		note_dataL[note_dataL_len][0] = noteNum;
	        		note_dataL[note_dataL_len][1] = start_Time + 2;
	        		note_dataL[note_dataL_len][2] = end_Time + 2;
	        		if(noteNum < lowestNote || noteNum > 54){
	        			note_dataR[note_dataR_len][4] = 1;
	        		}
	        		else{
	        			note_dataR[note_dataR_len][4] = 0;
	        		}
	        		note_dataL_len++;
	        	}
	        }




	         // Similar logic for the last 2 motors using ReceivedData[2] and ReceivedData[3]
	        //memset(RX_Buffer, 0, sizeof(RX_Buffer));
	        HAL_UART_Receive_IT(&huart3, RX_Buffer, sizeof(RX_Buffer));
	    }

}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
