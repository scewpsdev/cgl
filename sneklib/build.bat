@echo off

snekc build src/main.src -o bin/sneklib.exe -gcc -debug
cv2pdb bin/sneklib.exe
