# Epic Pandemic Simulator - Detailed Running Instructions

## Overview
This is a C/OpenGL/GLUT pandemic simulation that uses the SEIR (Susceptible → Exposed → Infected → Recovered) epidemiological model to simulate disease spread through a population. You make policy decisions that affect infection rates, economy, and mental health.

## Prerequisites

### Windows Setup
1. **MSYS2 Installation**: Ensure MSYS2 is installed on your system (typically at `C:\msys64`)
2. **Required Libraries**: The project needs FreeGLUT libraries

### One-Time Setup (if executables don't work)

#### Step 1: Install Required Dependencies
Open PowerShell as Administrator and run:
```powershell
# Install FreeGLUT using MSYS2
C:\msys64\usr\bin\bash.exe -lc "pacman -S mingw-w64-x86_64-freeglut --noconfirm"
```

#### Step 2: Copy Required DLL
```powershell
# Navigate to project directory
cd "C:\Users\LENOVO\Desktop\cgv_project\Epic-Pandemic-Simulator"

# Copy the FreeGLUT DLL to project directory
copy "C:\msys64\mingw64\bin\libfreeglut.dll" .
```

## Running the Application

### Method 1: Use Pre-compiled Executable (Recommended)
```powershell
# Navigate to project directory
cd "C:\Users\LENOVO\Desktop\cgv_project\Epic-Pandemic-Simulator"

# Run the simulation
.\pandemic_sim2.exe
```

### Method 2: Compile from Source
If you want to compile the project yourself:

#### Step 1: Install Build Tools
```powershell
# Install make and build tools using MSYS2
C:\msys64\usr\bin\bash.exe -lc "pacman -S make --noconfirm"
```

#### Option A: Using MSYS2 Make (Recommended)
```powershell
# Use MSYS2 environment to compile - this is the method that successfully built pandemic_sim2.exe
C:\msys64\usr\bin\bash.exe -lc "cd /c/Users/LENOVO/Desktop/cgv_project/Epic-Pandemic-Simulator && PATH=/mingw64/bin:$PATH make"
```

#### Option B: Direct GCC Compilation (Manual)
```powershell
# Compile using MSYS2 GCC directly (same command used by the Makefile)
C:\msys64\mingw64\bin\gcc.exe -Wall -Wextra -std=c99 -Isrc -Isrc/shared -Isrc/backend -Isrc/frontend -Isrc/controller src/main.c src/backend/seir.c src/backend/timer.c src/backend/economy.c src/backend/mental_health.c src/backend/hospital.c src/backend/decisions.c src/backend/end_conditions.c src/backend/debriefing.c src/backend/simulation.c src/frontend/renderer.c src/frontend/camera.c src/frontend/city_renderer.c src/frontend/entity_renderer.c src/frontend/hud.c src/frontend/menu.c src/frontend/input.c src/controller/controller.c src/controller/world_gen.c -o pandemic_sim -lfreeglut -lopengl32 -lglu32 -lm

# Copy the required DLL after compilation
copy "C:\msys64\mingw64\bin\libfreeglut.dll" .

# Run the compiled executable
.\pandemic_sim.exe
```

#### Step 3: Copy Required DLL (After Compilation)
```powershell
# Always copy the FreeGLUT DLL after successful compilation
copy "C:\msys64\mingw64\bin\libfreeglut.dll" .
```

## Game Controls

| Key | Action | Description |
|-----|--------|-------------|
| **ENTER** | Start Simulation | Begin the pandemic simulation |
| **1-9, 0, -, =** | Toggle School Grades | Close/open specific school grades (1-12) |
| **G** | Toggle Going Out | Control public movement restrictions |
| **S** | Toggle Outdoor Sports | Allow/prohibit outdoor sports activities |
| **+ / Backspace** | Sanitization Level | Increase/decrease public sanitization efforts |
| **[ / ]** | Simulation Speed | Decrease/increase simulation speed |
| **F** | Fast Forward | Skip ahead 5 days in simulation |
| **P / ESC** | Pause/Resume | Pause or resume the simulation |
| **H** | Help Overlay | Toggle help information display |
| **D** | Debug Mode | Show debug output in console |
| **R** | Restart | Restart the simulation from beginning |
| **Q** | Quit | Exit the application |

## Game Mechanics

### SEIR Model
The simulation uses a compartmental epidemiological model:
- **S**usceptible: People who can catch the disease
- **E**xposed: People infected but not yet contagious
- **I**nfected: People who are contagious
- **R**ecovered: People who have recovered and are immune

### Policy Trade-offs
Every decision creates consequences:
- **Closing Schools**: Reduces infection spread but hurts economy and mental health
- **Movement Restrictions**: Slows disease spread but impacts economic activity
- **Sanitization**: Helps prevent spread but costs resources
- **Activity Limitations**: Protects health but affects mental well-being

### End Conditions
The simulation ends when one of these conditions is met:
1. **Pandemic Ended**: Infection rate drops below 1%
2. **Budget Depleted**: Economy reaches 0
3. **Max Days Reached**: 100 days of simulation complete
4. **Mental Health Crisis**: Population mental health drops to 0

## Troubleshooting

### Common Issues and Solutions

#### Issue: "libfreeglut.dll was not found"
**Solution**: Copy the DLL from MSYS2 to project directory
```powershell
copy "C:\msys64\mingw64\bin\libfreeglut.dll" .
```

#### Issue: "GL/glut.h: No such file or directory"
**Solution**: Install FreeGLUT development libraries
```powershell
C:\msys64\usr\bin\bash.exe -lc "pacman -S mingw-w64-x86_64-freeglut --noconfirm"
```

#### Issue: Application window doesn't appear
**Solution**: Try running with Start-Process
```powershell
Start-Process -FilePath ".\pandemic_sim2.exe"
```

#### Issue: Compilation fails with linking errors
**Solution**: Ensure you're using MSYS2's GCC and proper library paths
```powershell
C:\msys64\mingw64\bin\gcc.exe [compilation flags] -LC:\msys64\mingw64\lib -lfreeglut
```

## Project Structure

```
Epic-Pandemic-Simulator/
├── README.md                 # Project documentation
├── Makefile                 # Build configuration
├── pandemic_sim.exe         # Compiled executable (older version)
├── pandemic_sim2.exe        # Compiled executable (recommended)
├── libfreeglut.dll          # Required DLL (copy this if missing)
├── src/                     # Source code directory
│   ├── main.c              # Entry point
│   ├── backend/            # Simulation logic (no OpenGL)
│   ├── frontend/           # OpenGL visualization
│   ├── controller/         # Mediator between backend/frontend
│   └── shared/             # Shared data structures
└── assets/                 # Game assets and resources
```

## Tips for Success

1. **Start with ENTER**: Press ENTER to begin the simulation immediately
2. **Monitor All Metrics**: Keep an eye on infection rate, economy, and mental health
3. **Balance Decisions**: Each policy has trade-offs - find the right balance
4. **Use Help**: Press H to toggle the help overlay if you forget controls
5. **Experiment**: Try different strategies to see what works best
6. **Watch the Clock**: You have 100 days to successfully manage the pandemic

## Development Notes

- **Language**: Pure C with OpenGL/GLUT for graphics
- **Architecture**: Modular design separating simulation logic from visualization
- **Platform**: Windows (with MinGW/MSYS2), Linux, macOS support planned
- **Dependencies**: FreeGLUT, OpenGL, GLU

For more technical details, see the main README.md file in the project directory.
