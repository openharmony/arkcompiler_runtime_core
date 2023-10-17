# TODO(vpukhov): adjust es2panda path
set(ES2PANDA_PATH ${PANDA_ROOT}/plugins/ecmascript/es2panda)

if(EXISTS ${ES2PANDA_PATH})
    list(APPEND HOST_TOOLS_TARGETS es2panda)
endif()