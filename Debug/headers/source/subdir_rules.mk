################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
headers/source/%.obj: ../headers/source/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs1100/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --idiv_support=idiv0 --tmu_support=tmu0 --include_path="C:/Talha Turac Turk/GitHub Repository/uart_communication_f280025c/uart_communication" --include_path="C:/ti/ccs1100/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --include_path="C:/Talha Turac Turk/GitHub Repository/uart_communication_f280025c/uart_communication/driverlib" --include_path="C:/Talha Turac Turk/GitHub Repository/uart_communication_f280025c/uart_communication/common/include" --include_path="C:/Talha Turac Turk/GitHub Repository/uart_communication_f280025c/uart_communication/headers/include" --advice:performance=all --define=CPU1 --define=_DUAL_HEADERS -g --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="headers/source/$(basename $(<F)).d_raw" --obj_directory="headers/source" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


