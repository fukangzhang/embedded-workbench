if(NOT DEFINED HOST_SIM_EXE)
    message(FATAL_ERROR "HOST_SIM_EXE is required")
endif()

if(NOT DEFINED HOST_SIM_INPUT)
    message(FATAL_ERROR "HOST_SIM_INPUT is required")
endif()

execute_process(
    COMMAND "${HOST_SIM_EXE}" --script
    INPUT_FILE "${HOST_SIM_INPUT}"
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error_output
    RESULT_VARIABLE result
)

if(NOT result EQUAL 0)
    message(FATAL_ERROR "host_sim --script failed with ${result}: ${error_output}")
endif()

string(FIND "${output}" "OK result=ok" ok_index)
if(ok_index EQUAL -1)
    message(FATAL_ERROR "host_sim --script output did not contain OK result=ok: ${output}")
endif()

string(FIND "${output}" "STATUS state=warning" status_index)
if(status_index EQUAL -1)
    message(FATAL_ERROR "host_sim --script output did not contain status response: ${output}")
endif()

string(FIND "${output}" "indicator=slow_blink" indicator_index)
if(indicator_index EQUAL -1)
    message(FATAL_ERROR "host_sim --script output did not contain alarm output summary: ${output}")
endif()
