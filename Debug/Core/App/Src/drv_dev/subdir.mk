################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../Core/App/Src/drv_dev/acs712_driver.c \
../Core/App/Src/drv_dev/esp8266_driver.c \
../Core/App/Src/drv_dev/lps22df_driver.c \
../Core/App/Src/drv_dev/lsm6dsv16x_driver.c \
../Core/App/Src/drv_dev/sht30_driver.c \
../Core/App/Src/drv_dev/ssd1306_driver.c

OBJS += \
./Core/App/Src/drv_dev/acs712_driver.o \
./Core/App/Src/drv_dev/esp8266_driver.o \
./Core/App/Src/drv_dev/lps22df_driver.o \
./Core/App/Src/drv_dev/lsm6dsv16x_driver.o \
./Core/App/Src/drv_dev/sht30_driver.o \
./Core/App/Src/drv_dev/ssd1306_driver.o

C_DEPS += \
./Core/App/Src/drv_dev/acs712_driver.d \
./Core/App/Src/drv_dev/esp8266_driver.d \
./Core/App/Src/drv_dev/lps22df_driver.d \
./Core/App/Src/drv_dev/lsm6dsv16x_driver.d \
./Core/App/Src/drv_dev/sht30_driver.d \
./Core/App/Src/drv_dev/ssd1306_driver.d


# Each subdirectory must supply rules for building sources it contributes
Core/App/Src/drv_dev/%.o Core/App/Src/drv_dev/%.su Core/App/Src/drv_dev/%.cyclo: ../Core/App/Src/drv_dev/%.c Core/App/Src/drv_dev/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H750xx -c -I../Core/Inc -I../Core/App/Inc -I../Core/App -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage  -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-App-2f-Src-2f-drv_dev

clean-Core-2f-App-2f-Src-2f-drv_dev:
	-$(RM) ./Core/App/Src/drv_dev/acs712_driver.cyclo ./Core/App/Src/drv_dev/acs712_driver.d ./Core/App/Src/drv_dev/acs712_driver.o ./Core/App/Src/drv_dev/acs712_driver.su ./Core/App/Src/drv_dev/esp8266_driver.cyclo ./Core/App/Src/drv_dev/esp8266_driver.d ./Core/App/Src/drv_dev/esp8266_driver.o ./Core/App/Src/drv_dev/esp8266_driver.su ./Core/App/Src/drv_dev/lps22df_driver.cyclo ./Core/App/Src/drv_dev/lps22df_driver.d ./Core/App/Src/drv_dev/lps22df_driver.o ./Core/App/Src/drv_dev/lps22df_driver.su ./Core/App/Src/drv_dev/lsm6dsv16x_driver.cyclo ./Core/App/Src/drv_dev/lsm6dsv16x_driver.d ./Core/App/Src/drv_dev/lsm6dsv16x_driver.o ./Core/App/Src/drv_dev/lsm6dsv16x_driver.su ./Core/App/Src/drv_dev/sht30_driver.cyclo ./Core/App/Src/drv_dev/sht30_driver.d ./Core/App/Src/drv_dev/sht30_driver.o ./Core/App/Src/drv_dev/sht30_driver.su ./Core/App/Src/drv_dev/ssd1306_driver.cyclo ./Core/App/Src/drv_dev/ssd1306_driver.d ./Core/App/Src/drv_dev/ssd1306_driver.o ./Core/App/Src/drv_dev/ssd1306_driver.su

.PHONY: clean-Core-2f-App-2f-Src-2f-drv_dev
