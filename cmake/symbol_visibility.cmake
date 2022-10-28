include_guard()

# Visibility controll
macro(SymbolVisibilityHidden)
    set(CMAKE_CXX_VISIBILITY_PRESET     hidden)
    set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)
endmacro()
