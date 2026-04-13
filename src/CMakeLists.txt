
# Create the executable target
add_executable(MeshProc
    MeshProc.cpp
)

# Modern compile features
target_compile_features(MeshProc PRIVATE cxx_std_20)

# Recommended warnings (portable)
if (MSVC)
    target_compile_options(MeshProc PRIVATE /W4 /permissive-)
else()
    target_compile_options(MeshProc PRIVATE -Wall -Wextra -Wpedantic)
endif()

# If you later add include dirs or libraries:
# target_include_directories(myapp PRIVATE include)
# target_link_libraries(myapp PRIVATE mylib)
