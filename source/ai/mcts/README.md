# MCTS AI for Advanced Strategic Command

**Next-generation AI using hierarchical Monte Carlo Tree Search for strategic and tactical decision-making.**

## Project Status

🚧 **Current Phase**: [See STATUS.md for implementation progress]

See [PROJECT_CHARTER.md](PROJECT_CHARTER.md) for goals, success criteria, and project scope.

## Documentation

### Core Documents
- **[PROJECT_CHARTER.md](PROJECT_CHARTER.md)** - Project goals, success criteria, key decisions
- **[STATUS.md](STATUS.md)** - Current implementation status, completed work, next steps

### Technical Documentation (`docs/`)
- **[game_description.md](docs/game_description.md)** - ASC game mechanics, AI requirements, domain analysis
- **[hierarchical_state_design.md](docs/hierarchical_state_design.md)** - Architecture design: groups, memory, multi-level MCTS
- **[implementation_roadmap.md](docs/implementation_roadmap.md)** - Phase-by-phase implementation plan, risks, milestones
- **[code_structure.md](docs/code_structure.md)** - ASC codebase analysis, integration points, existing APIs

## Architecture Overview

```
Strategic Layer (Groups + Sectors)
    ↓ objectives
Tactical Layer (Individual Units)
    ↓ execution
ASC Game Integration
```

**Key Concepts:**
- **Hierarchical MCTS**: Strategic decisions on group/sector level, tactical execution at unit level
- **Persistent Memory**: Multi-turn goals, research queues, fog-of-war tracking
- **Lightweight State**: Snapshots instead of full GameMap clones for performance

See [docs/hierarchical_state_design.md](docs/hierarchical_state_design.md) for detailed architecture.

## Directory Structure

```
source/ai/mcts/
├── README.md                  # This file
├── PROJECT_CHARTER.md         # Project scope and goals
├── STATUS.md                  # Current implementation status
├── docs/                      # Technical documentation
│   ├── game_description.md
│   ├── hierarchical_state_design.md
│   ├── implementation_roadmap.md
│   └── code_structure.md
├── core/                      # Domain-agnostic MCTS engine
├── domain/                    # ASC game state adapters
├── agents/                    # Agent logic (UnitAgent, SupportAgent, etc.)
├── coordination/              # Multi-agent coordination
└── infrastructure/            # Config, logging, utilities
```

## Quick Start for Contributors

1. **Understand the problem**: Read [docs/game_description.md](docs/game_description.md)
2. **Review the architecture**: Read [docs/hierarchical_state_design.md](docs/hierarchical_state_design.md)
3. **Check current status**: Read [STATUS.md](STATUS.md) to see what's implemented
4. **Follow the roadmap**: See [docs/implementation_roadmap.md](docs/implementation_roadmap.md) for phase plan

## Contact & Contribution

- **Repository**: https://github.com/ValHaris/asc-hq
- **ASC Project**: Advanced Strategic Command (open-source TBS game)
- **License**: GPL (matching ASC)
