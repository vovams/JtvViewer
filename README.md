# JtvViewer
A simple GUI to view tv program in JTV format

This app can be useful to view content of some JTV files you are working with, but it is too simple
to be your EPG.

The JTV archive have to be manually extracted first, so that you have `*.ndx` and `*.pdt` files.
You can open files with open dialog, command line arguments or by dragging them into the main
window. You can select either `.ndx` or `.pdt` file - the second one will be guessed automatically
(if you select both, the `.ndx` one will be ignored). You can also select (or drop or pass via
command line) multiple files at once, the first one will be opened in the current window and the
rest - in new windows.

You can specify tv program time zone in the `Options` menu as a time zone offset in seconds (e.g.
UTC+2 = 7200). As for the character set, it is currently hardcoded to CP1251 in the source code.

## Compilation

You will need Qt SDK to build this program. You cat use QtCreator or:

    $ qmake JtvViewer.pro
    $ make
