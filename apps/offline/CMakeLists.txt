cmake_minimum_required(VERSION 3.19)
project(offline)

# include_directories(
# 	${GTSAM_INCLUDE_DIR}
# )

# link_directories(
#   ${GTSAM_LIBRARY_DIRS}
# )

get_property(LIBS GLOBAL PROPERTY PROJECT_LIB)
aux_source_directory(src DIR_OFFLINEAPP)
foreach(item ${DIR_OFFLINEAPP})
    get_filename_component(name_we ${item} NAME_WE)
    add_executable(${name_we} ${item})
    target_link_libraries(${name_we}
    ${LIBS})
endforeach(item ${DIR_OFFLINEAPP})



