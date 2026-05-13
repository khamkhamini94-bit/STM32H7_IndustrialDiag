################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../Core/App/Src/drv_hal/adc_driver.c \
../Core/App/Src/drv_hal/dma_driver.c \
../Core/App/Src/drv_hal/i2c_driver.c \
../Core/App/Src/drv_hal/tim_driver.c \
../Core/App/Src/drv_hal/usart_driver.c

OBJS += \
./Core/App/Src/drv_hal/adc_driver.o \
./Core/App/Src/drv_hal/dma_driver.o \
./Core/App/Src/drv_hal/i2c_driver.o \
./Core/App/Src/drv_hal/tim_driver.o \
./Core/App/Src/drv_hal/usart_driver.o

C_DEPS += \
./Core/App/Src/drv_hal/adc_driver.d \
./Core/App/Src/drv_hal/dma_driver.d \
./Core/App/Src/drv_hal/i2c_driver.d \
./Core/App/Src/drv_hal/tim_driver.d \
./Core/App/Src/drv_hal/usart_driver.d


# Each subdirectory must supply rules for building sources it contributes
Core/App/Src/drv_hal/%.o Core/App/Src/drv_hal/%.su Core/App/Src/drv_hal/%.cyclo: ../Core/App/Src/drv_hal/%.c Core/App/Src/drv_hal/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H750xx -c -I../Core/Inc -I../Core/App/Inc -I../Core/App -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage  -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-App-2f-Src-2f-drv_hal

clean-Core-2f-App-2f-Src-2f-drv_hal:
	-$(RM) ./Core/App/Src/drv_hal/adc_driver.cyclo ./Core/App/Src/drv_hal/adc_driver.d ./Core/App/Src/drv_hal/adc_driver.o ./Core/App/Src/drv_hal/adc_driver.su ./Core/App/Src/drv_hal/dma_driver.cyclo ./Core/App/Src/drv_hal/dma_driver.d ./Core/App/Src/drv_hal/dma_driver.o ./Core/App/Src/drv_hal/dma_driver.su ./Core/App/Src/drv_hal/i2c_driver.cyclo ./Core/App/Src/drv_hal/i2c_driver.d ./Core/App/Src/drv_hal/i2c_driver.o ./Core/App/Src/drv_hal/i2c_driver.su ./Core/App/Src/drv_hal/tim_driver.cyclo ./Core/App/Src/drv_hal/tim_driver.d ./Core/App/Src/drv_hal/tim_driver.o ./Core/App/Src/drv_hal/tim_driver.su ./Core/App/Src/drv_hal/usart_driver.cyclo ./Core/App/Src/drv_hal/usart_driver.d ./Core/App/Src/drv_hal/usart_driver.o ./Core/App/Src/drv_hal/usart_driver.su

.PHONY: clean-Core-2f-App-2f-Src-2f-drv_hal
