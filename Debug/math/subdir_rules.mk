################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
math/%.obj: ../math/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccs1260/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/bin/cl430" -vmspx --data_model=large --use_hw_mpy=F5 --include_path="C:/ti/ccs1260/ccs/ccs_base/msp430/include" --include_path="C:/Users/kooo/Documents/Koo_Intermittent/int8/Source/portable/CCS/MSP430X" --include_path="C:/Users/kooo/Documents/Koo_Intermittent/int8/driverlib/MSP430FR5xx_6xx" --include_path="C:/Users/kooo/Documents/Koo_Intermittent/int8/Source/include" --include_path="C:/Users/kooo/Documents/Koo_Intermittent/int8" --include_path="C:/Users/kooo/Documents/Koo_Intermittent/int8/layer" --include_path="C:/Users/kooo/Documents/Koo_Intermittent/int8/parameter" --include_path="C:/Users/kooo/Documents/Koo_Intermittent/int8/DSPLib/include" --include_path="C:/Users/kooo/Documents/Koo_Intermittent/int8/uart_for_msp" --include_path="C:/ti/ccs1260/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/include" --advice:power="all" --advice:hw_config=all --define=__MSP430FR5994__ -g --c99 --printf_support=full --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="math/$(basename $(<F)).d_raw" --obj_directory="math" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


