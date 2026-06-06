################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Device_drivers/at24c256n.c \
../Core/Src/Device_drivers/nvm_high_api.c 

OBJS += \
./Core/Src/Device_drivers/at24c256n.o \
./Core/Src/Device_drivers/nvm_high_api.o 

C_DEPS += \
./Core/Src/Device_drivers/at24c256n.d \
./Core/Src/Device_drivers/nvm_high_api.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Device_drivers/%.o Core/Src/Device_drivers/%.su Core/Src/Device_drivers/%.cyclo: ../Core/Src/Device_drivers/%.c Core/Src/Device_drivers/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -I"D:/Embedded/stm32_project/EEPROM_Driver/Core/Inc/Device_drivers" -I"D:/Embedded/stm32_project/EEPROM_Driver/Core/Src/Device_drivers" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Device_drivers

clean-Core-2f-Src-2f-Device_drivers:
	-$(RM) ./Core/Src/Device_drivers/at24c256n.cyclo ./Core/Src/Device_drivers/at24c256n.d ./Core/Src/Device_drivers/at24c256n.o ./Core/Src/Device_drivers/at24c256n.su ./Core/Src/Device_drivers/nvm_high_api.cyclo ./Core/Src/Device_drivers/nvm_high_api.d ./Core/Src/Device_drivers/nvm_high_api.o ./Core/Src/Device_drivers/nvm_high_api.su

.PHONY: clean-Core-2f-Src-2f-Device_drivers

