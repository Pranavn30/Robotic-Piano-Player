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
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
uint8_t RX_Buffer[5]; //"Interrupt!"
uint8_t TX_Buffer[] = "Hello World\r\n";
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_TIM5_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
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
void actuatorMoveDistLoop(double dist, int dir);

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
		if(noteArr[i][1] >= time_max){
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
	int group_min_note = noteArr[0][0];
	int group_min_time = noteArr[0][1];
	for(int i = 0; i < note_len; i++){
		if(noteArr[i][3] != current_group){
			if(group_min_note > highestNote || group_min_note < lowestNote){
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
	if(group_min_note > highestNote || group_min_note < lowestNote){
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

// Sets TIM2 ARR so that it triggers for the next linear actuator movement
// Returns 0 if there is no next linear actuator movement
int setNextLinActuatorTime(int current_time){
	if(actuatorL >= actuatorPosL_len){
		if(actuatorR >= actuatorPosR_len){
			HAL_TIM_Base_Stop_IT(&htim2);
			return 0;
		}
		else{
			TIM2->ARR = actuatorPosR[actuatorR][1] - current_time;
		}
	}
	else if(actuatorR >= actuatorPosR_len){
		if(actuatorL >= actuatorPosL_len){
			HAL_TIM_Base_Stop_IT(&htim2);
			return 0;
		}
		else{
			TIM2->ARR = actuatorPosL[actuatorL][1] - current_time;
		}
	}
	else if(actuatorPosL[actuatorL][1] < actuatorPosR[actuatorR][1]){
		TIM2->ARR = actuatorPosL[actuatorL][1] - current_time;
	}
	else{
		TIM2->ARR = actuatorPosR[actuatorR][1] - current_time;
	}

	return 1;
}

int movements = 0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
	// Move actuator
	if(htim == &htim2){
		movements++;
		HAL_TIM_Base_Stop_IT(&htim2);
		HAL_TIM_Base_Stop_IT(&htim3);
		HAL_TIM_Base_Stop_IT(&htim4);
		int current_time = 0;
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
		HAL_TIM_Base_Start_IT(&htim2);
		HAL_TIM_Base_Start_IT(&htim3);
		HAL_TIM_Base_Start_IT(&htim4);
		setNextLinActuatorTime(current_time);
		movements++;
	}
	else if(htim == &htim3){
		int current_time = 0;
		if(note_dataR[DCRightStart][1] == note_dataL[DCLeftStart][1]){
			current_time = note_dataR[DCRightStart][1];

			while(note_dataR[DCRightStart][1] == current_time){
				//***************************************************
				// Push actuator number "note_dataR[DCRightStart][0]"
				//***************************************************
				DCRightStart++;
			}
			while(note_dataL[DCLeftStart][1] == current_time){
				//***************************************************
				// Push actuator number "note_dataL[DCLeftStart][0]"
				//***************************************************
				DCLeftStart++;
			}
		}
		else if(note_dataR[DCRightStart][1] > note_dataL[DCLeftStart][1]){
			current_time = note_dataL[DCLeftStart][1];

			while(note_dataL[DCLeftStart][1] == current_time){
				//***************************************************
				// Push actuator number "note_dataL[DCLeftStart][0]"
				//***************************************************
				DCLeftStart++;
			}
		}
		else{
			current_time = note_dataR[DCRightStart][1];

			while(note_dataR[DCRightStart][1] == current_time){
				//***************************************************
				// Push actuator number "note_dataR[DCRightStart][0]"
				//***************************************************
				DCRightStart++;
			}
		}
		if(note_dataR[DCRightStart][1] > note_dataL[DCLeftStart][1]){
			TIM3->ARR = note_dataL[DCLeftStart][1] - current_time;
		}
		else{
			TIM3->ARR = note_dataR[DCRightStart][1] - current_time;
		}
	}
	// Pull DC actuator
	else if(htim == &htim4){
		int current_time = 0;
		if(note_dataR_stop[DCRightStop][1] == note_dataL_stop[DCLeftStop][1]){
			current_time = note_dataR_stop[DCRightStop][1];

			while(note_dataR_stop[DCRightStop][1] == current_time){
				//***************************************************
				// Pull actuator number "note_dataR_stop[DCRightStop][0]"
				//***************************************************
				DCRightStop++;
			}
			while(note_dataL_stop[DCLeftStop][1] == current_time){
				//***************************************************
				// Pull actuator number "note_dataL_stop[DCLeftStop][0]"
				//***************************************************
				DCLeftStop++;
			}
		}
		else if(note_dataR_stop[DCRightStop][1] > note_dataL_stop[DCLeftStop][1]){
			current_time = note_dataL_stop[DCLeftStop][1];

			while(note_dataL_stop[DCLeftStop][1] == current_time){
				//***************************************************
				// Pull actuator number "note_dataL_stop[DCLeftStop][0]"
				//***************************************************
				DCLeftStop++;
			}
		}
		else{
			current_time = note_dataR_stop[DCRightStop][1];

			while(note_dataR_stop[DCRightStop][1] == current_time){
				//***************************************************
				// Pull actuator number "note_dataR_stop[DCRightStop][0]"
				//***************************************************
				DCRightStop++;
			}
		}
		if(note_dataR_stop[DCRightStop][1] > note_dataL_stop[DCLeftStop][1]){
			TIM4->ARR = note_dataL_stop[DCLeftStop][1] - current_time;
		}
		else{
			TIM4->ARR = note_dataR_stop[DCRightStop][1] - current_time;
		}
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
	leftMin, leftMin + whiteNoteDist, leftMin + whiteNoteDist * 2.0, leftMin + whiteNoteDist * 3.0,
	leftMin + whiteNoteDist * 4.0
};

// Index 0 = note number 71, Index 15 = note number 96
const double numToDistRight[] = {
	rightMin, 0, rightMin + whiteNoteDist, 0, rightMin + whiteNoteDist * 2.0, rightMin + whiteNoteDist * 3.0, 0, rightMin + whiteNoteDist * 4.0, 0
	, rightMin + whiteNoteDist * 5.0, 0, rightMin + whiteNoteDist * 6.0, rightMin + whiteNoteDist * 7.0, 0, rightMin + whiteNoteDist * 8.0, 0
	, rightMin + whiteNoteDist * 9.0, rightMin + whiteNoteDist * 10.0, 0, rightMin + whiteNoteDist * 11.0, 0, rightMin + whiteNoteDist * 12.0, 0
	, rightMin + whiteNoteDist * 13.0, rightMin + whiteNoteDist * 14.0, 0, rightMin + whiteNoteDist * 15.0
};

// Move actuator a given distance in a given direction, takes in distance d argument in inches
void moveActuatorDistLeft(double d, int dir){
	int on_time = (int) ((d / inPerSec) * 1000);

	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, dir);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 0);

    if(on_time){
        HAL_Delay(on_time);
    }


    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 1);
}

// Move actuator a given distance in a given direction, takes in distance d argument in inches
void moveActuatorDistRight(double d, int dir){
	int on_time = (int) ((d / inPerSec) * 1000);

	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, dir);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 0);

    if(on_time){
    	HAL_Delay(on_time);
    }

    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 1);
}

// Move the left linear actuator to the correct position for the current note number
void moveActuatorLeft(int note_num){
	double d = numToDistLeft[abs((note_num - defaultLeft))];
	double delta_x = current_pos_left - d;

	if(delta_x > 0){
		// Set stepper direction appropriately
		moveActuatorDistLeft(delta_x, 1);
	}
	else{
		// Set stepper direction appropriately
		moveActuatorDistLeft(delta_x * -1, 0);
	}

	current_pos_left = d;
}

// Move the right linear actuator to the correct position for the current note number
void moveActuatorRight(int note_num){
	double d = numToDistRight[note_num - defaultRight];
	double delta_x = d - current_pos_right;

	if(delta_x > 0){
		// Set stepper direction appropriately
		actuatorMoveDistLoop(delta_x, 1);
		//moveActuatorDistRight(delta_x, 1);
	}
	else{
		// Set stepper direction appropriately
		actuatorMoveDistLoop(delta_x * -1, 0);
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

	TIM2->ARR = 0;
	HAL_TIM_Base_Start_IT(&htim2);

	HAL_TIM_Base_Start_IT(&htim3);

	HAL_TIM_Base_Start_IT(&htim4);

	// Might need to set initial timer ARR values

	//********************************************************************
	// IMPORTANT: Set TIM2 interrupt as higher priority than TIM3 and TIM4
	//********************************************************************
}

void actuatorMoveDistLoop(double dist, int dir){
	int steps = (int) ((dist / inPerRot) * stepPerRot);

	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, dir);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 0);
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_0, 0);

	if(steps){
		for(int i = 0; i < steps; i++){
			HAL_GPIO_WritePin(GPIOG, GPIO_PIN_0, 1);
			HAL_Delay(1);
			HAL_GPIO_WritePin(GPIOG, GPIO_PIN_0, 0);
			//********************************************
			// CHANGE SPEED BY CHANGING HAL_Delay BELOW
			//********************************************
			HAL_Delay(1);
		}
	}

	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 1);
}

// DC ACTUATOR CODE
void ON1_PCF8574()
{
	uint8_t data_write1[1] = {0x01};
	uint8_t data_write2[1] = {0x00};
	uint8_t data_write3[1] = {0x02};
	HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &data_write1[0], 1, 1000);
	HAL_Delay(200);
	HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &data_write2[0], 1, 1000);
	HAL_Delay(200);
	HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &data_write3[0], 1, 1000);
	HAL_Delay(200);
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
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_TIM5_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  //********************************************************
  // Might need to move this to where ever we start the song
  //********************************************************

  HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_4);
  TIM5->CCR4 = 25;
  uint8_t data_write2[1] = {0x00};
  uint8_t data_write3[1] = {0x02};

  HAL_UART_Receive_IT(&huart3, RX_Buffer, sizeof(RX_Buffer));
  HAL_UART_Transmit_IT(&huart3, TX_Buffer, sizeof(TX_Buffer));
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 1);

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



  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, 1);
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  /*moveActuatorDistLeft(5, 1);
	  HAL_Delay(1000);
	  moveActuatorDistLeft(5, 0);
	  HAL_Delay(1000);*/

	  actuatorMoveDistLoop(10, 1);
	  HAL_Delay(1000);
	  actuatorMoveDistLoop(10, 0);
	  HAL_Delay(1000);
  }
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
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
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
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
  hi2c1.Init.Timing = 0x00000E14;
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
  htim2.Init.Prescaler = 39999;
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

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 39999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

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
  htim4.Init.Prescaler = 0;
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
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
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
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7|GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pins : PE2 PE3 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF13_SAI1;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PF0 PF1 PF2 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PF7 PF8 */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PC0 PC1 PC2 PC3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PC4 PC5 PC10 PC11 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB2 PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PG0 PG1 PG6 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : PE7 PE8 PE9 PE10
                           PE11 PE12 PE13 */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

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

  /*Configure GPIO pins : PC8 PC9 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_12;
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

  /*Configure GPIO pins : PB3 PB4 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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


			/*//MOTOR1
			if (RX_Buffer[0] == 'F')
	        {
	          forward(GPIOA, Motor1A, Motor1B);
	        }
	        else if (RX_Buffer[0] == 'B')
	        {
	          backward(GPIOA, Motor1A, Motor1B);
	        }
//	        else if (RX_Buffer[0] == 'S')
//	        {
//	          stop(GPIOA, Motor1A, Motor1B);
//	        }

	        //MOTOR2
	        if (RX_Buffer[1] == 'F')
	        {
	          forward(GPIOA, Motor2A, Motor2B);
	        }
	        else if (RX_Buffer[1] == 'B')
	        {
	          backward(GPIOA, Motor2A, Motor2B);
	        }
//	        else if (RX_Buffer[1] == 'S')
//	        {
//	          stop(GPIOA, Motor2A, Motor2B);
//	        }

	        //MOTOR3
	        if (RX_Buffer[1] == 'F')
	        {
	        	forward(GPIOA, Motor2A, Motor2B);
	        }
	        else if (RX_Buffer[1] == 'B')
	        {
	        	backward(GPIOA, Motor2A, Motor2B);
	        }
//	        else if (RX_Buffer[1] == 'S')
//	        {
//	        	stop(GPIOA, Motor2A, Motor2B);
//	        }*/


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
	        		title_data[title_index + i] = RX_Buffer[i];
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
	        		note_dataR[note_dataR_len][1] = start_Time;
	        		note_dataR[note_dataR_len][2] = end_Time;
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
	        		note_dataL[note_dataL_len][1] = start_Time;
	        		note_dataL[note_dataL_len][2] = end_Time;
	        		if(noteNum < lowestNote){
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
