# QtCub3d
Cub3d 42 project using Qt as a third party library

## Controls

- Click into the renderer to lock mouse and keyboard focus 
- WASD for movement 
- ESC to unlock focus 

## Install

- cmake, Qt6 and Qt5 is required as a dependency
- [installer](https://github.com/Gilqamesh/QtCub3d/releases)

## Features

- In the installed directory change maps in maps directory using the extended BNF format found in cub_format_bnf.txt
- To provide your own texture files please put them in the assets folder of the installed directory
- The file icon on the top left allows you to:
  - create a New map of size 3x3 to 100x100
  - Open editor: let's you add or remove walls
  - Load map from a file located in the maps folder
  - Save the current map of the renderer out to a file
  - Quit the application
