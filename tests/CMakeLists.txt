set(target_name "libkullo_tests")

# get all headers from source dir
file(GLOB_RECURSE headers
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "*.h" "*.hpp"
)

set(source_files
    api/apimodeltest.cpp
    api/apitest.cpp
    api/clientgeneratekeyslistener.cpp
    api/test_address.cpp
    api/test_addresshelpers.cpp
    api/test_client.cpp
    api/test_conversations.cpp
    api/test_datetime.cpp
    api/test_draftattachments.cpp
    api/test_drafts.cpp
    api/test_internaladdressutils.cpp
    api/test_internaldatetimeutils.cpp
    api/test_internalmasterkeyutils.cpp
    api/test_masterkey.cpp
    api/test_masterkeyhelpers.cpp
    api/test_messageattachments.cpp
    api/test_messages.cpp
    api/test_registration.cpp
    api/test_registry.cpp
    api/test_senders.cpp
    api/test_session.cpp
    api/test_syncer.cpp
    api/test_usersettings.cpp
    api_impl/test_deliveryimpl.cpp
    codec/test_attachmentsplittingsink.cpp
    codec/test_messagedecoder.cpp
    codec/test_messagedecryptor.cpp
    codec/test_messageencoder.cpp
    codec/test_sizelimitingfilter.cpp
    crypto/test_argon2.cpp
    crypto/test_decryptingfilter.cpp
    crypto/test_hasher.cpp
    crypto/test_hashverifyingfilter.cpp
    crypto/test_hkdf.cpp
    crypto/test_symmetrickey.cpp
    crypto/test_symmetrickeygenerator.cpp
    crypto/test_symmetrickeyloader.cpp
    dao/test_attachmentdao.cpp
    dao/test_draftdao.cpp
    dao/test_messageattachmentsink.cpp
    dao/test_messagedao.cpp
    dao/test_messagesearchdao.cpp
    dao/test_senderdao.cpp
    dao/test_usersettingsdao.cpp
    http/fake_httpclientfactory.cpp
    util/test_assert.cpp
    util/test_base64.cpp
    util/test_checkedconverter.cpp
    util/test_datetime.cpp
    util/test_events.cpp
    util/test_exceptions.cpp
    util/test_filesystem.cpp
    util/test_filterchain.cpp
    util/test_gzip.cpp
    util/test_gzipdecompressingfilter.cpp
    util/test_hex.cpp
    util/test_kulloaddress.cpp
    util/test_masterkey.cpp
    util/test_mimemultipart.cpp
    util/test_regex.cpp
    util/test_stacktrace.cpp
    util/test_strings.cpp
    util/test_urlencoding.cpp
    util/test_usersettings.cpp
    util/test_version.cpp
    dbtest.cpp
    kullotest.cpp
    test_nn.cpp
)

if(TESTS_AS_STATIC_LIB)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pullintests.cpp
        COMMAND
            ${CMAKE_CURRENT_SOURCE_DIR}/generate_pullintests.sh
            ${CMAKE_CURRENT_BINARY_DIR}/pullintests.cpp
        DEPENDS ${source_files}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating pullintests.cpp"
        VERBATIM
    )
    add_library(${target_name}
        ${headers}
        ${source_files}
        ${CMAKE_CURRENT_BINARY_DIR}/pullintests.cpp
    )
    target_compile_definitions(${target_name}
        PRIVATE
            KULLO_STATIC_TESTS_LIB
            GTEST_HAS_PTHREAD=0
    )
else()
    add_executable(${target_name}
        ${headers}
        ${source_files}
        main.cpp
    )
endif()

target_link_libraries(${target_name}
    # from this project
    argon2
    kulloclient

    # from other projects in workspace
    boost_program_options
    gmock
)

kullo_install_files("include/tests" ${headers})
