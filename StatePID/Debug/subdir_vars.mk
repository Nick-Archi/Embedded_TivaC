################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Add inputs and outputs from these tool invocations to the build variables 
CFG_SRCS += \
../empty.cfg 

CMD_SRCS += \
../EK_TM4C123GXL.cmd 

LIB_SRCS += \
C:/ti/tirtos_tivac_2_16_01_14/products/TivaWare_C_Series-2.1.1.71b/driverlib/ccs/Debug/driverlib.lib 

C_SRCS += \
../CmdInt.c \
../RxCommands.c \
../TxResponse.c \
../data.c \
../drive.c \
../main.c 

GEN_CMDS += \
./configPkg/linker.cmd 

GEN_FILES += \
./configPkg/linker.cmd \
./configPkg/compiler.opt 

GEN_MISC_DIRS += \
./configPkg/ 

C_DEPS += \
./CmdInt.d \
./RxCommands.d \
./TxResponse.d \
./data.d \
./drive.d \
./main.d 

GEN_OPTS += \
./configPkg/compiler.opt 

OBJS += \
./CmdInt.obj \
./RxCommands.obj \
./TxResponse.obj \
./data.obj \
./drive.obj \
./main.obj 

GEN_MISC_DIRS__QUOTED += \
"configPkg\" 

OBJS__QUOTED += \
"CmdInt.obj" \
"RxCommands.obj" \
"TxResponse.obj" \
"data.obj" \
"drive.obj" \
"main.obj" 

C_DEPS__QUOTED += \
"CmdInt.d" \
"RxCommands.d" \
"TxResponse.d" \
"data.d" \
"drive.d" \
"main.d" 

GEN_FILES__QUOTED += \
"configPkg\linker.cmd" \
"configPkg\compiler.opt" 

C_SRCS__QUOTED += \
"../CmdInt.c" \
"../RxCommands.c" \
"../TxResponse.c" \
"../data.c" \
"../drive.c" \
"../main.c" 


