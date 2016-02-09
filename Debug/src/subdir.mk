################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/btree.c \
../src/btree_insert.c \
../src/btree_node.c \
../src/btree_print.c \
../src/lab.c 

OBJS += \
./src/btree.o \
./src/btree_insert.o \
./src/btree_node.o \
./src/btree_print.o \
./src/lab.o 

C_DEPS += \
./src/btree.d \
./src/btree_insert.d \
./src/btree_node.d \
./src/btree_print.d \
./src/lab.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


