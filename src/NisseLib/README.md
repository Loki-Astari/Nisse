# NisseLib

This makefile builds the other directories into a single shared library.

The directory NisseServer contains all the code in the namespace `ThorsAnvil::NisseServer`. The code is compiler into a standard library `libNisseServer.lib`.

The directory NisseHTTP contains all the code in the namespace `ThorsAnvil::NisseHTTP`. The code is compiler into a standard library `libNisseHTTP.lib`.

This makefile compiles the libraries `libNisseServer.lib` and `libNisseHTTP.lib` into the shared library `libNisse.so` (or `libNisse.dll`).

