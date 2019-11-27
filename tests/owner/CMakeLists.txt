add_executable( owner_test $ENV{FAS_TESTING_CPP} owner_test.cpp  )
target_link_libraries(owner_test PUBLIC iow)
wci_test( TARGET owner_test COMMAND owner_test )
