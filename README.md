# Overview

This project simulates a a 2D eulerian fluid flow, and has a GUI for controlling parameters, boundary conditions and post processing for flow visualisations.

This project was built with SDL3 and IMGUI, they are required to build the project with the CMake file.

Built and tested with G++ on: 
- Windows
- Linux

# Examples

### Vortex shedding 

### Aerofoil lift generation
The pressure gradient caused by the aerfoil geometry can be clearly seen 




# Next steps
- Extending to 3D, this requires not insignificant rewrites but is just a level up of this project and is "simple" enough to do in the long term.
- Importing of custom geometry and BCs.
- Implementing different CFD solvers like particle based systems (LBM etc).
