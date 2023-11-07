# Injector

Dynamic linked library injector with a basic Win32 GUI. Removed all injection methods besides LoadLibrary, so this can't be used against processes that actually try and block dll injections.

Once a target process and library are selected, press END to inject.
