file(GLOB HEADERS "include/graphene/debug_blockproducer/*.hpp")

add_library( graphene_debug_blockproducer
             debug_api.cpp
             debug_blockproducer.cpp
           )

target_link_libraries( graphene_debug_blockproducer graphene_chain graphene_app )
target_include_directories( graphene_debug_blockproducer
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   graphene_debug_blockproducer

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
INSTALL( FILES ${HEADERS} DESTINATION "include/graphene/debug_blockproducer" )
