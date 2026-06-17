files in this dir are used to generate font files for the ui.
if you have a .ttf file, you can use fontforge to convert it to a .bdf file:

- element -> bitmap strikes available...
- type the desired size
- element -> regenerate bitmap glyphs
- file -> generate fonts
- select BDF as the output format & set the filename to be whatever.bdf
- then you can use convert.py
