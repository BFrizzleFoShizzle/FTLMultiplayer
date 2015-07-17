This is a work-in-progress mod/hack that aims to add multiplayer to the windows version (v1.03.3) of FTL: Faster Than Light.
It doesn't work yet, and is still months away from a semi-functional release.
#Compiling/running
This project uses VC++2010. It contains inline assembler which will likely break if compiled using a different compiler.
In order to run this, compile it into a DLL, place the fonts image in the same directory as the DLL, then inject the DLL into FTLGame.exe using your favourite DLL injector.
#Progress
The ship struct needs more work.
The biggest thing needed right now is a way of controlling ship encounters, setting an enemy ship to be one controlled by the mod.