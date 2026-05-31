find_package(Qt6 REQUIRED COMPONENTS Test)

qt_add_executable(tst_timeline_model
    tst_timeline_model.cpp
)

target_link_libraries(tst_timeline_model PRIVATE
    QtQuickComponents
    Qt6::Test
)

add_test(NAME TimelineModel COMMAND tst_timeline_model)
