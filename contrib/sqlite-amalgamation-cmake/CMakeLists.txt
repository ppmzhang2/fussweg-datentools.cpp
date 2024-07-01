set(SQLITE_SRC_DIR "${FusswegDatentools_SOURCE_DIR}/contrib/sqlite-amalgamation")

add_library(_sqlite "${SQLITE_SRC_DIR}/sqlite3.c")
target_include_directories(_sqlite SYSTEM PUBLIC "${SQLITE_SRC_DIR}")
