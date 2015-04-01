# Senior C++ Dev / Code first Audition

The objective is to write a program that can resize an JPEG image to a desired size. The size will be the larger dimension of the image. It can only use libjpeg and STL libraries. Also there's the challenge for the memory peak. It cannot exceed 30 MB, althought images can reach up to +100 MB when fully loaded in memory.

As for an effect, I bundled the libjpeg with the code, to avoid version problems and link problems. The executable won't need any additional DLL or shared libraries.

The strategy is quite simple. It chuncks the original memory in smaller pieces, and scan the image everytime to extract only one chunck per time. There are commented lines to save the chuncked pieces. It really keeps the peak memory low, althought a lot of I/O will happen. Surely, it spends more time than just using full memory size. But that's not the point.

The project is using CodeBlocks, but it's pretty straightforward to use a Makefile to build it. It's not using any special OS stuff, so it's pretty much OS independent.

It's not the best code I've even written, but I tried to spend less than 24h coding it. So I think it should be more commented and a little bit tweaked.
