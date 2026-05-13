################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../Core/App/Src/mgr/analog_manager.c \
../Core/App/Src/mgr/comm_manager.c \
../Core/App/Src/mgr/output_manager.c \
../Core/App/Src/mgr/sensor_manager.c

OBJS += \
./Core/App/Src/mgr/analog_manager.o \
./Core/App/Src/mgr/comm_manager.o \
./Core/App/Src/mgr/output_manager.o \
./Core/App/Src/mgr/sensor_manager.o

C_DEPS += \
./Core/App/Src/mgr/analog_manager.d \
./Core/App/Src/mgr/comm_manager.d \
./Core/App/Src/mgr/output_manager.d \
./Core/App/Src/mgr/sensor_manager.d


# Each subdirectory must supply rules for building sources it contributes
Core/App/Src/mgr/%.o Core/App/Src/mgr/%.su Core/App/Src/mgr/%.cyclo: ../Core/App/Src/mgr/%.c Core/App/Src/mgr/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H750xx -c -I../Core/Inc -I../Core/App/Inc -I../Core/App -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage  -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-App-2f-Src-2f-mgr

clean-Core-2f-App-2f-Src-2f-mgr:
	-$(RM) ./Core/App/Src/mgr/analog_manager.cyclo ./Core/App/Src/mgr/analog_manager.d ./Core/App/Src/mgr/analog_manager.o ./Core/App/Src/mgr/analog_manager.su ./Core/App/Src/mgr/comm_manager.cyclo ./Core/App/Src/mgr/comm_manager.d ./Core/App/Src/mgr/comm_manager.o ./Core/App/Src/mgr/comm_manager.su ./Core/App/Src/mgr/output_manager.cyclo ./Core/App/Src/mgr/output_manager.d ./Core/App/Src/mgr/output_manager.o ./Core/App/Src/mgr/output_manager.su ./Core/App/Src/mgr/sensor_manager.cyclo ./Core/App/Src/mgr/sensor_manager.d ./Core/App/Src/mgr/sensor_manager.o ./Core/App/Src/mgr/sensor_manager.su

.PHONY: clean-Core-2f-App-2f-Src-2f-mgr
