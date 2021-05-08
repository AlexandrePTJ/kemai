#
# Add processed qm files to qrc file.
# Files are added into 'l10n' prefix.
#
macro(add_qm_files_to_qrc qrc_file)
    set(_qm_files ${ARGN})
    set(_qrc_file ${CMAKE_CURRENT_BINARY_DIR}/l10n.qrc)

    file(WRITE ${_qrc_file} "<RCC>\n<qresource prefix=\"l10n/\">\n")
    foreach (_lang ${_qm_files})
        get_filename_component(_filename ${_lang} NAME)
        file(APPEND ${_qrc_file} "  <file>${_filename}</file>\n")
    endforeach ()
    file(APPEND ${_qrc_file} " </qresource>\n</RCC>\n")

    add_custom_target(${PROJECT_NAME}-l10n-qrc DEPENDS ${_qm_files})

    set(${qrc_file} ${_qrc_file})
endmacro()
