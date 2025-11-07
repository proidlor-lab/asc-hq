# MCTS AI for Advanced Strategic Command

**Next-generation AI using hierarchical Monte Carlo Tree Search for strategic and tactical decision-making.**

---

## 🚀 Quick Start

```bash
# Build
cd source/ai/mcts
make

# Run tests
./snapshot_test

# See: QUICKSTART.md for details
```

---

## 📊 Status

**Phase 0.1**: ✅ Complete (Game State Snapshots)  
**Build**: ✅ Successful  
**Tests**: ✅ All Passing  
**Performance**: ✅ Exceeds targets

See [STATUS.md](STATUS.md) for detailed progress.

---

## 📚 Documentation

### Essential
- **[QUICKSTART.md](QUICKSTART.md)** - Build, test, troubleshoot
- **[STATUS.md](STATUS.md)** - Current progress
- **[PROJECT_CHARTER.md](PROJECT_CHARTER.md)** - Goals & scope

### Implementation
- **[IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md)** - Phase 0.1 technical details
- **[CPP23_NOTES.md](docs/CPP23_NOTES.md)** - C++23 modernization notes

### Technical Design (`docs/`)
- **[hierarchical_state_design.md](docs/hierarchical_state_design.md)** - Architecture
- **[implementation_roadmap.md](docs/implementation_roadmap.md)** - Full roadmap
- **[game_description.md](docs/game_description.md)** - ASC mechanics
- **[code_structure.md](docs/code_structure.md)** - Codebase analysis

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
├── domain/          ✅ Game state snapshots (Phase 0.1 complete)
├── core/            ⏸️  MCTS engine (Phase 1)
├── agents/          ⏸️  Agent logic (Phase 2)
├── coordination/    ⏸️  Multi-agent (Phase 4)
└── infrastructure/  ⏸️  Config, logging (ongoing)
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
