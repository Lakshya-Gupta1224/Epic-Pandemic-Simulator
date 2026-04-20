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

### Compile
```bash
make
```

### Run
```bash
./pandemic_sim
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
