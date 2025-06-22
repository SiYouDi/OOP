cmake_minimum_required(VERSION 3.20)
project(ShoppingSystem)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# 设置输出路径
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# 添加源文件
file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# 指定 SQLite3 路径
#set(SQLITE3_DIR "D:/OOP_Lab/sqlite3")  # 替换为你的路径
set(SQLITE3_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sqlite3)  # 替换为你的路径


# 查找 SQLite3 动态库（MinGW 使用 .dll 和生成的 .lib）
find_library(SQLITE3_LIB
    NAMES sqlite3 libsqlite3  # 优先查找 libsqlite3.lib 或 libsqlite3.dll.a
    PATHS ${SQLITE3_DIR}
    NO_DEFAULT_PATH
)

# 如果未找到库，手动指定路径
if(NOT SQLITE3_LIB)
    message(WARNING "SQLite3 library not found, manually setting path.")
    set(SQLITE3_LIB "${SQLITE3_DIR}/libsqlite3.lib")  # 使用生成的 .lib 文件
endif()

# 创建动态库
#add_library(ShoppingSystem SHARED ${SRC})
add_executable(ShoppingSystem ${SRC})

# 链接 SQLite3 动态库
target_link_libraries(ShoppingSystem ${SQLITE3_LIB})

# 添加头文件路径
target_include_directories(ShoppingSystem PRIVATE ${SQLITE3_DIR})

# 导出宏定义
if(WIN32)
    target_compile_definitions(ShoppingSystem PRIVATE "EXPORT=__declspec(dllexport)")
else()
    target_compile_definitions(ShoppingSystem PRIVATE "EXPORT=__attribute__((visibility(\"default\")))")
    target_compile_options(ShoppingSystem PRIVATE "-fvisibility=hidden")
endif()