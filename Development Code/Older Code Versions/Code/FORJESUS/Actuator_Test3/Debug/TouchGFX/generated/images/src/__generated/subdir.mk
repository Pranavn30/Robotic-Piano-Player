################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../TouchGFX/generated/images/src/__generated/image_alternate_theme_images_backgrounds_320x240_poly.cpp \
../TouchGFX/generated/images/src/__generated/image_alternate_theme_images_widgets_togglebutton_medium_rounded_text_off_normal.cpp \
../TouchGFX/generated/images/src/__generated/image_alternate_theme_images_widgets_togglebutton_medium_rounded_text_on_action.cpp 

OBJS += \
./TouchGFX/generated/images/src/__generated/image_alternate_theme_images_backgrounds_320x240_poly.o \
./TouchGFX/generated/images/src/__generated/image_alternate_theme_images_widgets_togglebutton_medium_rounded_text_off_normal.o \
./TouchGFX/generated/images/src/__generated/image_alternate_theme_images_widgets_togglebutton_medium_rounded_text_on_action.o 

CPP_DEPS += \
./TouchGFX/generated/images/src/__generated/image_alternate_theme_images_backgrounds_320x240_poly.d \
./TouchGFX/generated/images/src/__generated/image_alternate_theme_images_widgets_togglebutton_medium_rounded_text_off_normal.d \
./TouchGFX/generated/images/src/__generated/image_alternate_theme_images_widgets_togglebutton_medium_rounded_text_on_action.d 


# Each subdirectory must supply rules for building sources it contributes
TouchGFX/generated/images/src/__generated/%.o TouchGFX/generated/images/src/__generated/%.su TouchGFX/generated/images/src/__generated/%.cyclo: ../TouchGFX/generated/images/src/__generated/%.cpp TouchGFX/generated/images/src/__generated/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L4R5xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../TouchGFX/App -I../TouchGFX/target/generated -I../TouchGFX/target -I../Middlewares/ST/touchgfx/framework/include -I../TouchGFX/generated/fonts/include -I../TouchGFX/generated/gui_generated/include -I../TouchGFX/generated/images/include -I../TouchGFX/generated/texts/include -I../TouchGFX/generated/videos/include -I../TouchGFX/gui/include -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -femit-class-debug-always -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-TouchGFX-2f-generated-2f-images-2f-src-2f-__generated

clean-TouchGFX-2f-generated-2f-images-2f-src-2f-__generated:
	-$(RM) ./TouchGFX/generated/images/src/__generated/image_alternate_theme_images_backgrounds_320x240_poly.cyclo ./TouchGFX/generated/images/src/__generated/image_alternate_theme_images_backgrounds_320x240_poly.d ./TouchGFX/generated/images/src/__generated/image_alternate_theme_images_backgrounds_320x240_poly.o ./TouchGFX/generated/images/src/__generated/image_alternate_theme_images_backgrounds_320x240_poly.su ./TouchGFX/generated/images/src/__generated/image_alternate_theme_images_widgets_togglebutton_medium_rounded_text_off_normal.cyclo ./TouchGFX/generated/images/src/__generated/image_alternate_theme_images_widgets_togglebutton_medium_rounded_text_off_normal.d ./TouchGFX/generated/images/src/__generated/image_alternate_theme_images_widgets_togglebutton_medium_rounded_text_off_normal.o ./TouchGFX/generated/images/src/__generated/image_alternate_theme_images_widgets_togglebutton_medium_rounded_text_off_normal.su ./TouchGFX/generated/images/src/__generated/image_alternate_theme_images_widgets_togglebutton_medium_rounded_text_on_action.cyclo ./TouchGFX/generated/images/src/__generated/image_alternate_theme_images_widgets_togglebutton_medium_rounded_text_on_action.d ./TouchGFX/generated/images/src/__generated/image_alternate_theme_images_widgets_togglebutton_medium_rounded_text_on_action.o ./TouchGFX/generated/images/src/__generated/image_alternate_theme_images_widgets_togglebutton_medium_rounded_text_on_action.su

.PHONY: clean-TouchGFX-2f-generated-2f-images-2f-src-2f-__generated

