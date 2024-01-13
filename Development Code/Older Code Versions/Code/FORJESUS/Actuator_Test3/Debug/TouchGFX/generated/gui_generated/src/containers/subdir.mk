################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../TouchGFX/generated/gui_generated/src/containers/CustomContainer1Base.cpp 

OBJS += \
./TouchGFX/generated/gui_generated/src/containers/CustomContainer1Base.o 

CPP_DEPS += \
./TouchGFX/generated/gui_generated/src/containers/CustomContainer1Base.d 


# Each subdirectory must supply rules for building sources it contributes
TouchGFX/generated/gui_generated/src/containers/%.o TouchGFX/generated/gui_generated/src/containers/%.su TouchGFX/generated/gui_generated/src/containers/%.cyclo: ../TouchGFX/generated/gui_generated/src/containers/%.cpp TouchGFX/generated/gui_generated/src/containers/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L4R5xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../TouchGFX/App -I../TouchGFX/target/generated -I../TouchGFX/target -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-TouchGFX-2f-generated-2f-gui_generated-2f-src-2f-containers

clean-TouchGFX-2f-generated-2f-gui_generated-2f-src-2f-containers:
	-$(RM) ./TouchGFX/generated/gui_generated/src/containers/CustomContainer1Base.cyclo ./TouchGFX/generated/gui_generated/src/containers/CustomContainer1Base.d ./TouchGFX/generated/gui_generated/src/containers/CustomContainer1Base.o ./TouchGFX/generated/gui_generated/src/containers/CustomContainer1Base.su

.PHONY: clean-TouchGFX-2f-generated-2f-gui_generated-2f-src-2f-containers

