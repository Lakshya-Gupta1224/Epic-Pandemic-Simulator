# Epidemic Choices — Pandemic Simulation

A modular C/OpenGL/GLUT pandemic simulation integrating the SEIR epidemiological model with 3D visualization.

## Architecture

```
Backend (Pure C, no GL)  →  Controller (Mediator)  →  Frontend (OpenGL/GLUT)
├── seir.c                  ├── controller.c           ├── renderer.c
├── timer.c                 └── world_gen.c            ├── camera.c
├── economy.c                                          ├── city_renderer.c
├── mental_health.c                                    ├── entity_renderer.c
├── hospital.c                                         ├── hud.c
├── decisions.c                                        ├── menu.c
├── end_conditions.c                                   └── input.c
├── debriefing.c
└── simulation.c
```

## Building

### Prerequisites
- GCC (MinGW on Windows)
- FreeGLUT development libraries
- OpenGL & GLU

### Option 1 (with make tools)
**First you have to download make**
```
Install make via MSYS2 (recommended)
Open an MSYS2 MINGW64 terminal and run:

pacman -S make
```
### Compile
```bash
make
```

### Run
```bash
./pandemic_sim
```
### Option 2 (Compile directly from PowerShell, no make needed)
### Compile
```
Run this one-liner that does the same thing as the Makefile:
powershell 

gcc -Wall -Wextra -std=c99 -Isrc -Isrc/shared -Isrc/backend -Isrc/frontend -Isrc/controller src/main.c src/backend/seir.c src/backend/timer.c src/backend/economy.c src/backend/mental_health.c src/backend/hospital.c src/backend/decisions.c src/backend/end_conditions.c src/backend/debriefing.c src/backend/simulation.c src/frontend/renderer.c src/frontend/camera.c src/frontend/city_renderer.c src/frontend/entity_renderer.c src/frontend/hud.c src/frontend/menu.c src/frontend/input.c src/controller/controller.c src/controller/world_gen.c -o pandemic_sim -lfreeglut -lopengl32 -lglu32 -lm
```

### Run
```
Then run:
powershell

.\pandemic_sim.exe
```

## Controls

| Key | Action |
|-----|--------|
| ENTER | Start simulation |
| 1-9, 0, -, = | Toggle school grades 1-12 |
| G | Toggle going out |
| S | Toggle outdoor sports |
| + / Backspace | Increase / decrease sanitization |
| [ / ] | Decrease / increase speed |
| F | Fast forward 5 days |
| P / ESC | Pause / Resume |
| H | Toggle help overlay |
| D | Debug output to console |
| R (results) | Restart |
| Q | Quit |

## Game Mechanics

The simulation uses the **SEIR compartmental model**:
- **S**usceptible → **E**xposed → **I**nfected → **R**ecovered

Every intervention creates a trade-off:
- Closing schools **reduces** infection but **hurts** economy and mental health
- Restricting activities **slows** spread but **costs** resources

### End Conditions
1. **Pandemic Ended** — Infection drops below 1%
2. **Budget Depleted** — Economy reaches 0
3. **Max Days Reached** — 100 days pass
4. **Mental Health Crisis** — Mental health drops to 0
