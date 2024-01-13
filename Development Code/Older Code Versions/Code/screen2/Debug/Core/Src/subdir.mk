################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/ILI9341_GFX.c \
../Core/Src/ILI9341_STM32_Driver.c \
../Core/Src/font12.c \
../Core/Src/font16.c \
../Core/Src/font20.c \
../Core/Src/font24.c \
../Core/Src/font8.c \
../Core/Src/fonts.c \
../Core/Src/main.c \
../Core/Src/stm32l4xx_hal_msp.c \
../Core/Src/stm32l4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32l4xx.c \
../Core/Src/z_displ_ILI9XXX.c \
../Core/Src/z_displ_ILI9XXX_test.c \
../Core/Src/z_touch_XPT2046.c \
../Core/Src/z_touch_XPT2046_menu.c \
../Core/Src/z_touch_XPT2046_test.c 

OBJS += \
./Core/Src/ILI9341_GFX.o \
./Core/Src/ILI9341_STM32_Driver.o \
./Core/Src/font12.o \
./Core/Src/font16.o \
./Core/Src/font20.o \
./Core/Src/font24.o \
./Core/Src/font8.o \
./Core/Src/fonts.o \
./Core/Src/main.o \
./Core/Src/stm32l4xx_hal_msp.o \
./Core/Src/stm32l4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32l4xx.o \
./Core/Src/z_displ_ILI9XXX.o \
./Core/Src/z_displ_ILI9XXX_test.o \
./Core/Src/z_touch_XPT2046.o \
./Core/Src/z_touch_XPT2046_menu.o \
./Core/Src/z_touch_XPT2046_test.o 

C_DEPS += \
./Core/Src/ILI9341_GFX.d \
./Core/Src/ILI9341_STM32_Driver.d \
./Core/Src/font12.d \
./Core/Src/font16.d \
./Core/Src/font20.d \
./Core/Src/font24.d \
./Core/Src/font8.d \
./Core/Src/fonts.d \
./Core/Src/main.d \
./Core/Src/stm32l4xx_hal_msp.d \
./Core/Src/stm32l4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32l4xx.d \
./Core/Src/z_displ_ILI9XXX.d \
./Core/Src/z_displ_ILI9XXX_test.d \
./Core/Src/z_touch_XPT2046.d \
./Core/Src/z_touch_XPT2046_menu.d \
./Core/Src/z_touch_XPT2046_test.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L4R5xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/ILI9341_GFX.cyclo ./Core/Src/ILI9341_GFX.d ./Core/Src/ILI9341_GFX.o ./Core/Src/ILI9341_GFX.su ./Core/Src/ILI9341_STM32_Driver.cyclo ./Core/Src/ILI9341_STM32_Driver.d ./Core/Src/ILI9341_STM32_Driver.o ./Core/Src/ILI9341_STM32_Driver.su ./Core/Src/font12.cyclo ./Core/Src/font12.d ./Core/Src/font12.o ./Core/Src/font12.su ./Core/Src/font16.cyclo ./Core/Src/font16.d ./Core/Src/font16.o ./Core/Src/font16.su ./Core/Src/font20.cyclo ./Core/Src/font20.d ./Core/Src/font20.o ./Core/Src/font20.su ./Core/Src/font24.cyclo ./Core/Src/font24.d ./Core/Src/font24.o ./Core/Src/font24.su ./Core/Src/font8.cyclo ./Core/Src/font8.d ./Core/Src/font8.o ./Core/Src/font8.su ./Core/Src/fonts.cyclo ./Core/Src/fonts.d ./Core/Src/fonts.o ./Core/Src/fonts.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/stm32l4xx_hal_msp.cyclo ./Core/Src/stm32l4xx_hal_msp.d ./Core/Src/stm32l4xx_hal_msp.o ./Core/Src/stm32l4xx_hal_msp.su ./Core/Src/stm32l4xx_it.cyclo ./Core/Src/stm32l4xx_it.d ./Core/Src/stm32l4xx_it.o ./Core/Src/stm32l4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32l4xx.cyclo ./Core/Src/system_stm32l4xx.d ./Core/Src/system_stm32l4xx.o ./Core/Src/system_stm32l4xx.su ./Core/Src/z_displ_ILI9XXX.cyclo ./Core/Src/z_displ_ILI9XXX.d ./Core/Src/z_displ_ILI9XXX.o ./Core/Src/z_displ_ILI9XXX.su ./Core/Src/z_displ_ILI9XXX_test.cyclo ./Core/Src/z_displ_ILI9XXX_test.d ./Core/Src/z_displ_ILI9XXX_test.o ./Core/Src/z_displ_ILI9XXX_test.su ./Core/Src/z_touch_XPT2046.cyclo ./Core/Src/z_touch_XPT2046.d ./Core/Src/z_touch_XPT2046.o ./Core/Src/z_touch_XPT2046.su ./Core/Src/z_touch_XPT2046_menu.cyclo ./Core/Src/z_touch_XPT2046_menu.d ./Core/Src/z_touch_XPT2046_menu.o ./Core/Src/z_touch_XPT2046_menu.su ./Core/Src/z_touch_XPT2046_test.cyclo ./Core/Src/z_touch_XPT2046_test.d ./Core/Src/z_touch_XPT2046_test.o ./Core/Src/z_touch_XPT2046_test.su

.PHONY: clean-Core-2f-Src

