file(GLOB HEADERS "include/graphene/blockproducer/*.hpp")

add_library( graphene_blockproducer 
             blockproducer.cpp
           )

target_link_libraries( graphene_blockproducer graphene_chain graphene_app )
target_include_directories( graphene_blockproducer
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

if (MSVC)
    set_target_properties( graphene_blockproducer PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

install( TARGETS
   graphene_blockproducer

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
