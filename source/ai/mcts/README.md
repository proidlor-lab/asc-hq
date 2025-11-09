# MCTS AI for Advanced Strategic Command

**Next-generation AI using hierarchical Monte Carlo Tree Search for strategic and tactical decision-making.**

---

## 🚀 Quick Start

```bash
# Build ASC with MCTS AI
cd /path/to/asc-hq
./bootstrap && ./configure && make

# Test AI types in headless mode
./source/unix/asc/asc --headless \
    --mapfile data/maps/tutorial.ascmap \
    --player1 mcts_balanced \
    --player2 classic \
    --turnlimit 10

# All AI types available:
# classic, mcts_balanced, mcts_aggressive, mcts_defensive, mcts_fast, mcts_deep

# See: QUICKSTART.md, HEADLESS_MODE_INTEGRATION.md for details
```

---

## 📊 Status

**Phase 1.1b**: ✅ **COMPLETE** (Integration Skeleton)  
**Runtime**: ✅ **FULLY FUNCTIONAL** (all bugs fixed)  
**Build**: ✅ Successful  
**Tests**: ✅ All Passing  
**AI Types**: ✅ 6 variants working (Classic + 5 MCTS profiles)

**Latest**: All critical bugs fixed (2025-11-08 21:20 UTC)
- ✅ Player::swapPlayers() preserves aiType
- ✅ AIFactory correctly creates MCTS AI types
- ✅ Log output readable with newlines
- ✅ All 6 AI types verified in headless mode

See [STATUS.md](STATUS.md) for detailed progress.

---

## 📚 Documentation

### Essential (Start Here)
- **[README.md](README.md)** - This file (quick overview)
- **[QUICKSTART.md](QUICKSTART.md)** - Build, test, troubleshoot
- **[STATUS.md](STATUS.md)** - Current progress & next steps
- **[PROJECT_CHARTER.md](PROJECT_CHARTER.md)** - Goals & scope

### Implementation Guides
- **[INTEGRATION_NOTES.md](INTEGRATION_NOTES.md)** - Legacy code integration status
- **[HEADLESS_MODE_INTEGRATION.md](HEADLESS_MODE_INTEGRATION.md)** - Command-line AI selection
- **[VERIFICATION_GUIDE.md](VERIFICATION_GUIDE.md)** - How to verify MCTS is working
- **[SPECIFICATION_GAPS.md](SPECIFICATION_GAPS.md)** - Known limitations & future work (TODOs)

### Technical Design (`docs/`)
- **[hierarchical_state_design.md](docs/hierarchical_state_design.md)** - Architecture overview
- **[implementation_roadmap.md](docs/implementation_roadmap.md)** - Full roadmap
- **[game_description.md](docs/game_description.md)** - ASC mechanics
- **[code_structure.md](docs/code_structure.md)** - Codebase analysis
- **[CPP23_NOTES.md](docs/CPP23_NOTES.md)** - C++23 modernization notes

### Historical Documentation (`docs/history/`)
- **[INDEX.md](docs/history/INDEX.md)** - Index of all historical docs
- Phase completion reports (0.1, 0.2, 0.3, 1.1b)
- Bug fix documentation
- Build milestone reports
- See `docs/history/` for complete archive

---

## 🏗️ Architecture

```
Strategic Layer (Groups + Sectors)
    ↓ objectives
Tactical Layer (Individual Units)
    ↓ execution
ASC Game Integration
```

**Key Features:**
- **Lightweight Snapshots**: 4.6 KB for 20 units (vs 10-50 MB GameMap)
- **Fast Cloning**: 0.019ms per clone (50× better than target!)
- **Dependency Injection**: Clean interfaces, testable design
- **C++23 Ready**: Modern features, future-proof

---

## 📁 Directory Structure

```
source/ai/mcts/
├── domain/          ✅ Game state snapshots, actions, evaluation
├── core/            ✅ MCTS engine (Phase 1.1 complete)
├── infrastructure/  ✅ Legacy game interface, config
├── agents/          ⏸️  Agent logic (Phase 2)
└── coordination/    ⏸️  Multi-agent (Phase 4)

source/ai/
├── ai_factory.h/cpp ✅ Factory pattern for AI creation
├── mcts_ai.h/cpp    ✅ MCTS AI wrapper (implements BaseAI)
└── ai_config.h      ✅ Configuration system
```

---

## 🎯 Performance

| Metric | Target | Actual | Status |
|--------|--------|--------|--------|
| Clone time | <1ms | **0.019ms** | ✅✅ |
| Memory (20 units) | <20 KB | **4.6 KB** | ✅✅ |
| 100 clones | <100ms | **3.3ms** | ✅✅ |

---

## 📝 License

GPL (matching ASC)  
Repository: https://github.com/ValHaris/asc-hq
