################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../LIB_LED/lib_led.c 

OBJS += \
./LIB_LED/lib_led.o 

C_DEPS += \
./LIB_LED/lib_led.d 


# Each subdirectory must supply rules for building sources it contributes
LIB_LED/lib_led.o: C:/Users/BochniakF/Desktop/GitHub/playground_FilipBochniak/playground/LIB_LED/lib_led.c LIB_LED/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32L562xx -c -I../Core/Inc -I../Drivers/STM32L5xx_HAL_Driver/Inc -I../Drivers/STM32L5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L5xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/BochniakF/Desktop/GitHub/playground_FilipBochniak/playground/LIB_LED" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-LIB_LED

clean-LIB_LED:
	-$(RM) ./LIB_LED/lib_led.d ./LIB_LED/lib_led.o ./LIB_LED/lib_led.su

.PHONY: clean-LIB_LED

