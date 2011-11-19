################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CameraCapture.cpp \
../src/CameraCaptureEntry.cpp \
../src/CameraCaptureMainForm.cpp \
../src/CameraCapturePreview.cpp \
../src/CameraCapturePreviewForm.cpp \
../src/CameraRecorder.cpp \
../src/CameraRecorderForm.cpp 

OBJS += \
./src/CameraCapture.o \
./src/CameraCaptureEntry.o \
./src/CameraCaptureMainForm.o \
./src/CameraCapturePreview.o \
./src/CameraCapturePreviewForm.o \
./src/CameraRecorder.o \
./src/CameraRecorderForm.o 

CPP_DEPS += \
./src/CameraCapture.d \
./src/CameraCaptureEntry.d \
./src/CameraCaptureMainForm.d \
./src/CameraCapturePreview.d \
./src/CameraCapturePreviewForm.d \
./src/CameraRecorder.d \
./src/CameraRecorderForm.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: bada C++ Compiler'
	arm-samsung-nucleuseabi-g++ -D_DEBUG -DSHP -I"C:/bada/sdk/include" -I"C:/bada/sdk/PhotoClown/inc" -O0 -g -Wall -c -funsigned-char -fshort-wchar -fpic -mthumb -mthumb-interwork -mfpu=vfp -mfloat-abi=softfp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	arm-samsung-nucleuseabi-g++ -D_DEBUG -DSHP -I"C:/bada/sdk/include" -I"C:/bada/sdk/PhotoClown/inc" -O0 -g -Wall -E -funsigned-char -fshort-wchar -fpic -mthumb -mthumb-interwork -mfpu=vfp -mfloat-abi=softfp -o"C:/bada/sdk/repository/PhotoClown/Target-Debug/$(notdir $(basename $@).i)" "$<"
	@echo 'Finished building: $<'
	@echo ' '


