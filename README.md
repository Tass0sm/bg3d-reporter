# BG3D File Format Reporter #

While trying to reverse [Otto Matic](https://en.wikipedia.org/wiki/Otto_Matic),
I found a unique file format for representing 3D models in the game. This file
format isn't documented anywhere except for in Brian Greenstone's book and the
accompanying source code. (BG3D = Brian Greenstone 3D.) This repository contains
my tool for parsing the file and the lessons I learned about the format.

## Lessons ##

1. The file format is made up of tags and data. The tags mark the beginning of
   sections of data for materials, textures, OpenGL vertex arrays, normal
   arrays, UV arrays, triangle arrays, etc.
2. Each tag is 32 bits. Most other values in the file are also 32 bits, except
   for a few chars in the header.
3. The PowerPC Macintosh computers for which the Pangea Software games and the
   BG3D file format were developed were big endian, so conversion is needed in
   the code.
4. Later versions of bg3d, or at least the example models packaged with the
   book, have a slightly different format to the models packaged with Otto
   Matic.
   
## TODOs ##

1. DONE Finish the tool for reporting all the information for every tag.
2. DONE Make tool conform to the multiple bg3d versions seen so far.
3. Make a tool for converting the BG3D files to an open, more supported file
   format.
4. Maybe add tools for exporting the textures and meshes seperately.

