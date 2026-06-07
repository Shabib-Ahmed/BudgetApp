cmake_minimum_required(VERSION 3.10)

project(BudgetAPP VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(MyApp 
    main.cpp 
    src/BudgetManager.cpp
    libs/csvLibrary/CSVReader.cpp
)

target_include_directories(MyApp PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
)

configure_file(data.csv data.csv COPYONLY)