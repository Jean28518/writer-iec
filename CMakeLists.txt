
set(example_SRCS
   writer_iec.c
)

IF(MSVC)
set_source_files_properties(${example_SRCS}
                                       PROPERTIES LANGUAGE CXX)
ENDIF(MSVC)

configure_file(client1-key.pem client1-key.pem COPYONLY)
configure_file(client1.cer client1.cer COPYONLY)
configure_file(root.cer root.cer COPYONLY)

add_executable(writer_iec
  ${example_SRCS}
)

target_link_libraries(writer_iec
    iec61850
)
