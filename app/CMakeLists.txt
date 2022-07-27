add_executable(app_exe)

target_sources(app_exe PRIVATE "main.cpp")

target_compile_features(app_exe PRIVATE cxx_std_20)

target_link_libraries(app_exe PRIVATE lib::lib)

add_test(NAME app::exe COMMAND app_exe)
