update_ogenc(
  TARGETS chat_server chat_client
  WARNINGS -Wnull-dereference OFF)

add_executable(chat_server chat_server.cpp)
target_link_libraries(chat_server PRIVATE iow)

add_executable(chat_client chat_client.cpp)
target_link_libraries(chat_client PRIVATE iow)

wci_utils(chat_server chat_client)
