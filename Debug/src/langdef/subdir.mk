################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/langdef/langdef.c \
../src/langdef/nd.c \
../src/langdef/translate.c 

OBJS += \
./src/langdef/langdef.o \
./src/langdef/nd.o \
./src/langdef/translate.o 

C_DEPS += \
./src/langdef/langdef.d \
./src/langdef/nd.d \
./src/langdef/translate.d 


# Each subdirectory must supply rules for building sources it contributes
src/langdef/%.o: ../src/langdef/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I../inc -I/usr/include/python3.6m/ -O0 -g3 -Wall -Wextra -Winline -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

