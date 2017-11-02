# PedestrianRemoval

Interactively Remove People from Images

Requirements : OpenCV Version 3.2 or above

Code Usage
 
Compilation: g++ $(pkg-config --cflags --libs opencv) main.cpp -o main


Running: ./main \<path to img\>


Options: 

Enter 1 for automatic removal, 2 for single click removal, 3 for two click removal. [increasing levels of interactive human input]

