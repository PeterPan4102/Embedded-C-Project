################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/src/Bootloader.c \
../src/src/Driver_GPIO.c \
../src/src/Driver_USART.c \
../src/src/FLASH.c \
../src/src/HAL_GPIO.c \
../src/src/HAL_USART.c \
../src/src/Queue.c \
../src/src/app.c \
../src/src/clocks_and_modes.c \
../src/src/srec.c 

OBJS += \
./src/src/Bootloader.o \
./src/src/Driver_GPIO.o \
./src/src/Driver_USART.o \
./src/src/FLASH.o \
./src/src/HAL_GPIO.o \
./src/src/HAL_USART.o \
./src/src/Queue.o \
./src/src/app.o \
./src/src/clocks_and_modes.o \
./src/src/srec.o 

C_DEPS += \
./src/src/Bootloader.d \
./src/src/Driver_GPIO.d \
./src/src/Driver_USART.d \
./src/src/FLASH.d \
./src/src/HAL_GPIO.d \
./src/src/HAL_USART.d \
./src/src/Queue.d \
./src/src/app.d \
./src/src/clocks_and_modes.d \
./src/src/srec.d 


# Each subdirectory must supply rules for building sources it contributes
src/src/%.o: ../src/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@src/src/Bootloader.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


