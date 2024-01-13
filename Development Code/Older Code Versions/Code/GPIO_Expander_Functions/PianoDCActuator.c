//////////////////////////////////////////////
//      HAND 1 - hi2c1                      //
//////////////////////////////////////////////

//NUMBER 1//////////////////////////////////////////
// void GPIO_Activation_Right(int note){
//     int note_num = note; 
    
//     uint8_t P01_ON[1] = {0x01};
//     uint8_t P01_STOP[1] = {0x00};
    
//     uint8_t P23_ON[1] = {0x04};
//     uint8_t P23_STOP[1] = {0x00};

//     uint8_t P34_ON[1] = {0x10};
//     uint8_t P34_STOP[1] = {0x00};

//     if (note_num == 0){
//         HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &P01_ON[0], 1, 1000);
//         HAL_Delay(150);
//         HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &P01_STOP[0], 1, 1000);
//     } else if (note_num == 1){
//         HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &P23_ON[0], 1, 1000);
//         HAL_Delay(150);
//         HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &P23_STOP[0], 1, 1000);
//     } else if (note_num == 2){
//         HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &P34_ON[0], 1, 1000);
//         HAL_Delay(150);
//         HAL_I2C_Master_Transmit(&hi2c1, (0x20<<1), &P34_STOP[0], 1, 1000);
//     } else if (note_num == 3){
//         HAL_I2C_Master_Transmit(&hi2c1, (0x21<<1), &P01_ON[0], 1, 1000);
//         HAL_Delay(150);
//         HAL_I2C_Master_Transmit(&hi2c1, (0x21<<1), &P01_STOP[0], 1, 1000);
//     } else if (note_num == 4){
//         HAL_I2C_Master_Transmit(&hi2c1, (0x21<<1), &P01_ON[0], 1, 1000);
//         HAL_Delay(150);
//         HAL_I2C_Master_Transmit(&hi2c1, (0x21<<1), &P01_STOP[0], 1, 1000);
//     } else if (note_num == 5){
//         HAL_I2C_Master_Transmit(&hi2c1, (0x21<<1), &P01_ON[0], 1, 1000);
//         HAL_Delay(150);
//         HAL_I2C_Master_Transmit(&hi2c1, (0x21<<1), &P01_STOP[0], 1, 1000);
//     } else if (note_num == 6){
//         HAL_I2C_Master_Transmit(&hi2c2, (0x20<<1), &P01_ON[0], 1, 1000);
//         HAL_Delay(150);
//         HAL_I2C_Master_Transmit(&hi2c2, (0x20<<1), &P01_STOP[0], 1, 1000);
//     } else if (note_num == 7){
//         HAL_I2C_Master_Transmit(&hi2c2, (0x20<<1), &P01_ON[0], 1, 1000);
//         HAL_Delay(150);
//         HAL_I2C_Master_Transmit(&hi2c2, (0x20<<1), &P01_STOP[0], 1, 1000);
//     } else if (note_num == 8){
//         HAL_I2C_Master_Transmit(&hi2c2, (0x20<<1), &P01_ON[0], 1, 1000);
//         HAL_Delay(150);
//         HAL_I2C_Master_Transmit(&hi2c2, (0x20<<1), &P01_STOP[0], 1, 1000);
//     } else if (note_num == 9){
//         HAL_I2C_Master_Transmit(&hi2c2, (0x21<<1), &P01_ON[0], 1, 1000);
//         HAL_Delay(150);
//         HAL_I2C_Master_Transmit(&hi2c2, (0x21<<1), &P01_STOP[0], 1, 1000);
//     } else if (note_num == 10){
//         HAL_I2C_Master_Transmit(&hi2c2, (0x21<<1), &P01_ON[0], 1, 1000);
//         HAL_Delay(150);
//         HAL_I2C_Master_Transmit(&hi2c2, (0x21<<1), &P01_STOP[0], 1, 1000);
//     } else if (note_num == 11){
//         HAL_I2C_Master_Transmit(&hi2c2, (0x21<<1), &P01_ON[0], 1, 1000);
//         HAL_Delay(150);
//         HAL_I2C_Master_Transmit(&hi2c2, (0x21<<1), &P01_STOP[0], 1, 1000);
//     }
    
// }

void GPIO_Activation_Right(int note) {
    uint8_t P_ON[1];
    uint8_t P_STOP[1] = {0x00};

    if (note < 3) {
        P_ON[0] = 0x01;
    } else if (note < 6) {
        P_ON[0] = 0x04;
    } else {
        P_ON[0] = 0x10;
    }

    I2C_HandleTypeDef* selected_hi2c = (note < 6) ? &hi2c1 : &hi2c2;

    HAL_I2C_Master_Transmit(selected_hi2c, (note % 3 == 0 ? 0x20 : 0x21) << 1, P_ON, 1, 1000);
    HAL_Delay(150);
    HAL_I2C_Master_Transmit(selected_hi2c, (note % 3 == 0 ? 0x20 : 0x21) << 1, P_STOP, 1, 1000);
}

void GPIO_Activation_Left(int note) {
    uint8_t P_ON[1];
    uint8_t P_STOP[1] = {0x00};

    if (note < 3) {
        P_ON[0] = 0x01;
    } else if (note < 6) {
        P_ON[0] = 0x04;
    } else {
        P_ON[0] = 0x10;
    }

    I2C_HandleTypeDef* selected_hi2c = (note < 6) ? &hi2c3 : &hi2c4;

    HAL_I2C_Master_Transmit(selected_hi2c, (note % 3 == 0 ? 0x20 : 0x21) << 1, P_ON, 1, 1000);
    HAL_Delay(150);
    HAL_I2C_Master_Transmit(selected_hi2c, (note % 3 == 0 ? 0x20 : 0x21) << 1, P_STOP, 1, 1000);
}

void GPIO_Deactivation_Right(int note) {
    uint8_t P_OFF[1];
    uint8_t P_STOP[1] = {0x00};

    if (note < 3) {
        P_OFF[0] = 0x02;
    } else if (note < 6) {
        P_OFF[0] = 0x08;
    } else {
        P_OFF[0] = 0x20;
    }

    I2C_HandleTypeDef* selected_hi2c = (note < 6) ? &hi2c1 : &hi2c2;

    HAL_I2C_Master_Transmit(selected_hi2c, (note % 3 == 0 ? 0x20 : 0x21) << 1, P_OFF, 1, 1000);
    HAL_Delay(150);
    HAL_I2C_Master_Transmit(selected_hi2c, (note % 3 == 0 ? 0x20 : 0x21) << 1, P_STOP, 1, 1000);
}

void GPIO_Deactivation_Left(int note) {
    uint8_t P_OFF[1];
    uint8_t P_STOP[1] = {0x00};

    if (note < 3) {
        P_OFF[0] = 0x02;
    } else if (note < 6) {
        P_OFF[0] = 0x08;
    } else {
        P_OFF[0] = 0x20;
    }

    I2C_HandleTypeDef* selected_hi2c = (note < 6) ? &hi2c3 : &hi2c4;

    HAL_I2C_Master_Transmit(selected_hi2c, (note % 3 == 0 ? 0x20 : 0x21) << 1, P_OFF, 1, 1000);
    HAL_Delay(150);
    HAL_I2C_Master_Transmit(selected_hi2c, (note % 3 == 0 ? 0x20 : 0x21) << 1, P_STOP, 1, 1000);
}

void GPIO_Activation_Right(int note) {
    uint8_t P_ON[1];
    uint8_t P_STOP[1] = {0x00};

    if (note < 3) {
        P_ON[0] = 0x01;
    } else if (note < 6) {
        P_ON[0] = 0x04;
    } else {
        P_ON[0] = 0x10;
    }

    I2C_HandleTypeDef* selected_hi2c = (note < 6) ? &hi2c1 : &hi2c2;

    HAL_I2C_Master_Transmit(selected_hi2c, (note % 3 == 0 ? 0x20 : 0x21) << 1, P_ON, 1, 1000);
    HAL_Delay(150);
    HAL_I2C_Master_Transmit(selected_hi2c, (note % 3 == 0 ? 0x20 : 0x21) << 1, P_STOP, 1, 1000);
}

void GPIO_Activation_Left(int note) {
    uint8_t P_ON[1];
    uint8_t P_STOP[1] = {0x00};

    if (note < 3) {
        P_ON[0] = 0x01;
    } else if (note < 6) {
        P_ON[0] = 0x04;
    } else {
        P_ON[0] = 0x10;
    }

    I2C_HandleTypeDef* selected_hi2c = (note < 6) ? &hi2c3 : &hi2c4;

    HAL_I2C_Master_Transmit(selected_hi2c, (note % 3 == 0 ? 0x20 : 0x21) << 1, P_ON, 1, 1000);
    HAL_Delay(150);
    HAL_I2C_Master_Transmit(selected_hi2c, (note % 3 == 0 ? 0x20 : 0x21) << 1, P_STOP, 1, 1000);
}

void GPIO_Deactivation_Right(int note) {
    uint8_t P_OFF[1];
    uint8_t P_STOP[1] = {0x00};

    if (note < 3) {
        P_OFF[0] = 0x02;
    } else if (note < 6) {
        P_OFF[0] = 0x08;
    } else {
        P_OFF[0] = 0x20;
    }

    I2C_HandleTypeDef* selected_hi2c = (note < 6) ? &hi2c1 : &hi2c2;

    HAL_I2C_Master_Transmit(selected_hi2c, (note % 3 == 0 ? 0x20 : 0x21) << 1, P_OFF, 1, 1000);
    HAL_Delay(150);
    HAL_I2C_Master_Transmit(selected_hi2c, (note % 3 == 0 ? 0x20 : 0x21) << 1, P_STOP, 1, 1000);
}

void GPIO_Deactivation_Left(int note) {
    uint8_t P_OFF[1];
    uint8_t P_STOP[1] = {0x00};

    if (note < 3) {
        P_OFF[0] = 0x02;
    } else if (note < 6) {
        P_OFF[0] = 0x08;
    } else {
        P_OFF[0] = 0x20;
    }

    I2C_HandleTypeDef* selected_hi2c = (note < 6) ? &hi2c3 : &hi2c4;

    HAL_I2C_Master_Transmit(selected_hi2c, (note % 3 == 0 ? 0x20 : 0x21) << 1, P_OFF, 1, 1000);
    HAL_Delay(150);
    HAL_I2C_Master_Transmit(selected_hi2c, (note % 3 == 0 ? 0x20 : 0x21) << 1, P_STOP, 1, 1000);
}
