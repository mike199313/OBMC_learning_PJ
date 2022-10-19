#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/uio.h>
#include <string.h>
#include <ctype.h>

#define GPIO_SYSFS "/sys/class/gpio/"

struct GPIO{
    unsigned int gpioBase;    // GPIO base number
    unsigned int gpioNum;    // GPIO pin name offset
    char direction[50];    // Direction for export GPIO pin
    char interrupt[50];    // Interrupt for export GPIO pin
    char buffer[50];    // Buffer using for gpiolib function
};

/** @brief To find GPIO base number in aspeed chip at 1e78000 GPIO offset
 *  @returns GPIO base number
 */
unsigned int FindGpioBaseAspeed(){
    unsigned int result = 0;
    char buf[128];
    FILE *fp;

    //get gpio base and cut gpiochip from get result
    if ((fp = popen("ls /sys/bus/platform/devices/1e780000.gpio/gpio  |  grep -i gpiochip | cut -c 9-", "r")) == NULL) {
        printf("ERROR - Could not find gpio base number\n");
        return result;
    }
    fgets(buf, sizeof buf, fp);
    pclose(fp);

    //char array to unsigned int
    sscanf(buf, "%d", &result);

    return result;
}

/** @brief To convert GPIO name to GPIO offset
 *  @param[in] base - Input GPIO base number
 *  @param[in] offsetName - Input GPIO pin name (limited)
 *  @returns GPIO offset (base on one pin group have 8 pin)
 */
unsigned int ConvertGpioOffset(unsigned int base, char offsetName[]){
    unsigned int result = 0;
    unsigned int sum = 0;
    char offsetNameUpper;
    char stringNumber[strlen(offsetName)];    // char array for number character
    unsigned int stringLetter[strlen(offsetName)];    //char array for letter character
    unsigned int initialStringCount[2] = {0,0};    // 0: number count 1: letter count

    for (int i =0 ; i<strlen(offsetName); i++) {
        // Set offsetName character to capital
        offsetNameUpper = toupper(offsetName[i]);

        // Get number character in input offsetName string
        if ('0' <= offsetNameUpper && offsetNameUpper <= '7'){
            stringNumber[initialStringCount[0]] = offsetNameUpper;
            initialStringCount[0]++;
        }

        // Get letter character in input offsetName string
        if ('A' <= offsetNameUpper && offsetNameUpper <= 'Z'){
            stringLetter[initialStringCount[1]] =  offsetNameUpper - 'A';
            initialStringCount[1]++;
        }
    }

    // Check input char array are limited in most 2 and at least 1 letter ,and limited 1 number
    if(initialStringCount[0] != 1 || initialStringCount[1] < 1 || initialStringCount[1] > 2){
        printf("ERROR - Convert Gpio string error\n");
        return result;
    }

    // sum for input more than 1 letter
    for (int i=0; i<initialStringCount[1]; i++){
        sum += stringLetter[i]+26*i;
    }

    //char array to unsigned int
    sscanf(stringNumber, "%d", &result);

    // Base on one pin group have 8 pin
    result += sum*8 + base;

    return result;
}


/** @brief To write data to GPIO path
 *  @param[in] path - Input GPIO path for writing
 *  @param[in] data - Input data for writing to GPIO
 *  @returns success: true, fail: fasle
 */
bool GpioWrite(char path[],char data[]){
    FILE *fp;

    fp = fopen(path,"w");
    if(fp == NULL){
        printf("ERROR - Write data fail\n");
        return false;
    }
    else{
        fwrite(data,1,strlen(data),fp);
        fclose(fp);
        return true;
    }
}

/** @brief To read data from GPIO path
 *  @param[in] path - Input GPIO path for reading
 *  @param[in] gpio - Input GPIO struct and save reading data in GPIO buffer
 */
void GpioRead(char path[],GPIO* gpio){
    FILE *fp;

    fp = fopen(path,"r");
    if(fp == NULL){
        printf("ERROR - Invalid reading path\n");
    }
    else{
        // Read one line from data
        fscanf(fp, "%[^\n]", gpio->buffer);
        fclose(fp);
    }
}

/** @brief To check GPIO_SYSFS path export file exist
 *  @returns success: path exist, fail: path not exist
 */
bool CheckGpioExport(){
    char stringBuffer[50];

    sprintf(stringBuffer, "%sexport",GPIO_SYSFS);
    // Check GPIO_SYSFS path exist
    if(access(stringBuffer, F_OK ) != -1 ) {
        return true;
    }
    else{
        printf("ERROR - GPIO_SYSFS path does not exist\n");
        return false;
    }
}

/** @brief To get GPIO path name and save in GPIO buffer
 *  @param[in] name - Input GPIO file name in GPIO path
 *  @param[in] gpio - Input GPIO struct and save path data in GPIO buffer
 */
void GetGpioPath(char name[],GPIO* gpio){
    sprintf(gpio->buffer, "%sgpio%d/%s",GPIO_SYSFS,gpio->gpioNum,name);
}

/** @brief To export a GPIO using input GPIO number
 *  @param[in] gpioNum - Input GPIO number
 *  @returns success: true, fail: fasle
 */
bool SetGpioExport(unsigned int gpioNum){
    char stringBuffer[50];
    char data[10];

    if(CheckGpioExport()){
        sprintf(data, "%d", gpioNum);
        sprintf(stringBuffer, "%sexport",GPIO_SYSFS);
        GpioWrite(stringBuffer,data);
        return true;
    }
    else{
        printf("ERROR - GPIO_SYSFS path does not exist.\n");
        return false;
    }
}

/** @brief To set value for GPIO pin
 *  @param[in] value - Input GPIO number
 *  @param[in] gpio - Input GPIO struct
 *  @returns success: true, fail: fasle
 */
bool SetGpioValue(unsigned int value,GPIO* gpio){
    // To set GPIO buffer
    GetGpioPath("value",gpio);

    if(value == 0){
        GpioWrite(gpio->buffer,"0");
        return true;
    }
    else if(value == 1){
        GpioWrite(gpio->buffer,"1");
        return true;
    }
    else{
        printf("ERROR - Invalid value: %d",value);
        return false;
    }
}

/** @brief To get current value from GPIO pin
 *  @param[in] gpio - Input GPIO struct
 *  @returns GPIO current value
 */
unsigned int GetGpioValue(GPIO* gpio){
    // To set GPIO buffer
    GetGpioPath("value",gpio);
    // To read GPIO value and save in buffer
    GpioRead(gpio->buffer,gpio);
    return atoi(gpio->buffer);
}

/** @brief To set GPIO direction and interrupt for GPIO pin
 *  @param[in] direction - Input GPIO direction
 *  @param[in] gpio - Input GPIO struct
 */
void SetGpioDirection(char direction[],GPIO* gpio){
    char* currentDirection;

    if(!CheckGpioExport()){
        printf("ERROR - Not exported!\n");
        return;
    }

    if(strcmp(direction,"in") == 0 || strcmp(direction,"out") == 0){
        memcpy(gpio->direction,direction, strlen(direction));
    }
    else if(strcmp(direction,"rising") == 0 || strcmp(direction,"falling") == 0 || strcmp(direction,"both") == 0){
        memcpy(gpio->direction,"in", strlen("in"));
        memcpy(gpio->interrupt,direction, strlen(direction));
        // To set GPIO buffer
        GetGpioPath("edge",gpio);
        GpioWrite(gpio->buffer,gpio->interrupt);
    }
    else{
        printf("ERROR - Invalid Direction\n");
        return;
    }
    // To set GPIO buffer
    GetGpioPath("direction",gpio);
    GpioRead(gpio->buffer,gpio);
    currentDirection = gpio->buffer;

    if(strcmp(currentDirection,gpio->direction) != 0){
        // To set GPIO buffer
        GetGpioPath("direction",gpio);
        GpioWrite(gpio->buffer,gpio->direction);
    }
}
