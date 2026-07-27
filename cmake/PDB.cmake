function(_makepri_get_target_pdb_name target result)
    set(
        ${result}
        "$<TARGET_FILE_DIR:${target}>/$<TARGET_FILE_BASE_NAME:${target}>.pdb"
        PARENT_SCOPE
    )
endfunction()

function(makepri_target_add_pdb target)
    if(WIN32)
        _makepri_get_target_pdb_name(${target} pdb_name)
        target_compile_options(${target} PRIVATE -gcodeview)
        target_link_options(${target} PRIVATE "-Wl,--pdb=${pdb_name}" -Wl,-s)
        set_property(TARGET ${target} APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${pdb_name}")
    endif()
endfunction()

if(WIN32)
    list(APPEND MAKEPRI_THIRD_PARTY_COMPILE_FLAGS -gcodeview)
endif()
