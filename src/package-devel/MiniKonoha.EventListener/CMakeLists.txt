FIND_PACKAGE_EXTRA_LIBRARY(event jansson)
set(PACKAGE_SOURCE_CODE EventListener_glue.c)
set(PACKAGE_SCRIPT_CODE EventListener_glue.k)
add_konoha_package(Konoha.EventListener)
