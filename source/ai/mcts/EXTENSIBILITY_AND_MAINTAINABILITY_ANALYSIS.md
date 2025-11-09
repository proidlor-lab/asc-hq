# MCTS AI - Extensibility and Maintainability Analysis

**Date**: 2025-11-09
**Scope**: Critical analysis of source/ai/mcts codebase
**Focus**: Architecture extensibility, code maintainability, technical debt

---

## Executive Summary

The MCTS AI implementation demonstrates **strong architectural foundations** with excellent use of modern C++23 patterns and clean separation of concerns. However, several **critical extensibility gaps** and **maintainability concerns** exist that could impede future development and integration with the broader ASC codebase.

**Overall Assessment**:
- Architecture Quality: **8/10** (Strong interfaces, good separation)
- Extensibility: **6/10** (Limited by concrete dependencies and missing abstractions)
- Maintainability: **7/10** (Good documentation, but complexity concerns)
- Production Readiness: **5/10** (MVP quality, significant gaps remain)

---

## Critical Findings by Priority

### 🔴 PRIORITY 1: Critical Architectural Issues

#### 1.1 Hard-Coded Action Space Limitation

**Location**: `mcts_search.cpp:335-369`, `simulation_action_executor.h:141-143`

**Issue**: Action generation is **tightly coupled** to the concrete `Action` variant type. Adding new action types requires modifications to multiple files across the codebase.

**Impact**:
- **High barrier to extension**: Adding service actions, building, or cargo operations requires changes to 6+ files
- **Maintenance burden**: Every new action type ripples through selection, expansion, simulation, and backpropagation
- **Testing complexity**: Each action type addition requires comprehensive integration testing

**Current Code**:
```cpp
// action_types.h:99
using Action = std::variant<MoveAction, AttackAction, WaitAction>;
```

**Problem**: No abstraction layer for action generation. The `getUnexpandedActions()` method must collect all action types manually.

**Recommended Solution**:
```cpp
// NEW: Action generator registry pattern
class IActionGenerator {
public:
    virtual ~IActionGenerator() = default;
    virtual std::vector<Action> generateActions(
        const GameStateSnapshot& state,
        UnitID unitID) const = 0;
    virtual int getPriority() const = 0;  // For ordering
};

class ActionGeneratorRegistry {
    std::vector<std::unique_ptr<IActionGenerator>> generators;
public:
    void registerGenerator(std::unique_ptr<IActionGenerator> gen);
    std::vector<Action> generateAllActions(
        const GameStateSnapshot& state,
        UnitID unitID) const;
};
```

**Benefits**:
- New action types can be added without modifying core MCTS code
- Easier to test action generators in isolation
- Supports dynamic action type registration (e.g., map-specific actions)

**Effort**: Medium (2-3 days refactoring)

---

#### 1.2 Missing Abstraction for State Representation

**Location**: `game_state_snapshot.h`, `mcts_search.cpp:189-277`

**Issue**: The `GameStateSnapshot` is a **concrete class** rather than an interface, making it impossible to swap alternative state representations without rewriting core algorithms.

**Impact**:
- Cannot experiment with different state representations (e.g., bitboard, compressed state)
- Difficult to add hierarchical state abstractions for strategic planning
- Memory profiling and optimization limited to single implementation

**Current Design**:
```cpp
class GameStateSnapshot {  // Concrete class
    std::vector<UnitSnapshot> units;
    fast_map<MapCoordinate, FieldSnapshot> terrain;
    // ...
};
```

**Problem**: All MCTS code directly references `GameStateSnapshot` concrete type.

**Recommended Solution**:
```cpp
// NEW: State interface abstraction
class IGameState {
public:
    virtual ~IGameState() = default;
    virtual std::unique_ptr<IGameState> clone() const = 0;
    virtual const UnitSnapshot* findUnit(UnitID id) const = 0;
    virtual std::vector<const UnitSnapshot*> getPlayerUnits(PlayerID) const = 0;
    // ... other query methods
};

// Concrete implementations can vary
class SnapshotGameState : public IGameState { /* current impl */ };
class CompressedGameState : public IGameState { /* memory-efficient */ };
class HierarchicalGameState : public IGameState { /* strategic layer */ };
```

**Benefits**:
- Enables experimentation with state representations
- Supports multi-level abstraction (tactical + strategic)
- Better memory/performance profiling

**Effort**: High (4-5 days refactoring, breaking change)

**Trade-off**: Adds virtual function call overhead (acceptable for clean architecture)

---

#### 1.3 Executor Factory Pattern Incomplete

**Location**: `i_action_executor.h:131-152`, `action_executor_factory.cpp`

**Issue**: The `ActionExecutorFactory` creates executors but provides **no mechanism for executor customization or dependency injection** beyond basic snapshot vs. real game distinction.

**Impact**:
- Cannot create custom executors with different simulation fidelity levels
- No way to inject mock executors for testing
- Difficult to add executor variants (e.g., probabilistic, deterministic, fuzzy)

**Current Code**:
```cpp
class ActionExecutorFactory {
    static std::unique_ptr<IActionExecutor> createSimulationExecutor(...);
    static std::unique_ptr<IActionExecutor> createRealGameExecutor(...);
};
```

**Problem**: Factory is static with no extension points.

**Recommended Solution**:
```cpp
// NEW: Configurable factory with builder pattern
class ExecutorFactory {
public:
    class Builder {
        ExecutionContext defaultContext;
        std::unique_ptr<IActionCostCalculator> costCalc;
        std::unique_ptr<ICombatResolver> combatResolver;
    public:
        Builder& withContext(ExecutionContext ctx);
        Builder& withCostCalculator(std::unique_ptr<IActionCostCalculator> calc);
        Builder& withCombatResolver(std::unique_ptr<ICombatResolver> resolver);
        std::unique_ptr<IActionExecutor> build();
    };

    static Builder simulation(std::unique_ptr<GameStateSnapshot> state);
    static Builder realGame(GameMap* map);
};

// Usage:
auto executor = ExecutorFactory::simulation(std::move(snapshot))
    .withCombatResolver(std::make_unique<SimpleCombatResolver>())
    .withCostCalculator(std::make_unique<ApproximateCostCalculator>())
    .build();
```

**Benefits**:
- Supports varied executor configurations
- Easier testing with mock components
- Enables progressive complexity scaling

**Effort**: Medium (2-3 days)

---

#### 1.4 Rollout Policy Hard-Coded

**Location**: `mcts_search.cpp:185-277` (simulate method)

**Issue**: Rollout simulation uses a **monolithic method** with if-else branching for random vs. heuristic rollout. No abstraction for pluggable rollout policies.

**Impact**:
- Cannot experiment with different rollout strategies (e.g., ε-greedy, UCB-based, learned)
- Difficult to implement domain-specific rollout heuristics
- Testing rollout policies requires modifying MCTSSearch class

**Current Code**:
```cpp
double MCTSSearch::simulate(MCTSNode* node) {
    // 90 lines of rollout logic hard-coded in method
    if (config_.useRandomRollout || actions.size() == 1) {
        // Random rollout
    } else {
        // Heuristic rollout
    }
}
```

**Recommended Solution**:
```cpp
// NEW: Rollout policy interface
class IRolloutPolicy {
public:
    virtual ~IRolloutPolicy() = default;
    virtual Action selectAction(
        const std::vector<Action>& legalActions,
        const GameStateSnapshot& state,
        const ITacticalEvaluator& evaluator) const = 0;
};

class RandomRolloutPolicy : public IRolloutPolicy { /* ... */ };
class HeuristicRolloutPolicy : public IRolloutPolicy { /* ... */ };
class MixedRolloutPolicy : public IRolloutPolicy { /* ε-greedy */ };

// In MCTSSearch:
std::unique_ptr<IRolloutPolicy> rolloutPolicy_;
```

**Benefits**:
- Easy to test rollout policies independently
- Enables ML-based rollout policies
- Cleaner code separation

**Effort**: Medium (2 days)

---

### 🟡 PRIORITY 2: Maintainability Concerns

#### 2.1 Caching Logic in GameStateSnapshot Prone to Bugs

**Location**: `game_state_snapshot.h:268-290`

**Issue**: Mutable caching with manual dirty flag management is **error-prone** and violates const-correctness principles.

**Current Code**:
```cpp
mutable std::unordered_map<UnitID, size_t> unitIndexById;
mutable bool unitIndexesDirty;

void ensureUnitIndexes() const { /* rebuilds if dirty */ }
```

**Problems**:
- Manual dirty flag management scattered across 5+ methods
- Const methods mutate state (violates logical constness)
- Easy to forget invalidation (leads to stale cache bugs)
- No thread-safety (future parallelization blocked)

**Recommended Solution**:
```cpp
// Option 1: Lazy evaluation with std::optional
std::optional<std::unordered_map<UnitID, size_t>> cachedIndexById_;

const auto& getUnitIndexById() const {
    if (!cachedIndexById_) {
        cachedIndexById_ = buildUnitIndex();
    }
    return *cachedIndexById_;
}

// Option 2: Separate read-only view class
class GameStateSnapshotView {
    const GameStateSnapshot& snapshot;
    mutable LazyIndex<UnitID, size_t> unitIndex;  // RAII-managed cache
};
```

**Benefits**:
- Eliminates manual dirty flag management
- Clearer const-correctness
- Easier to reason about cache lifetime

**Effort**: Low-Medium (1-2 days)

---

#### 2.2 Missing Error Handling Strategy

**Location**: Multiple files - no consistent error handling pattern

**Issue**: Mix of **nullptr returns**, **empty optionals**, and **silent failures** with no unified error handling strategy.

**Examples**:
```cpp
// mcts_search.cpp:174
auto child = createChildNode(node, actionToTry);
if (!child) {
    return node;  // Silent failure - child creation failed, but why?
}

// simulation_action_executor.cpp
ActionResult execute(...) {
    return ActionResult::failure("generic message");  // Lost context
}
```

**Problems**:
- Debugging is difficult (no error context propagation)
- Silent failures mask bugs
- No logging of error conditions
- Cannot distinguish between different failure modes

**Recommended Solution**:
```cpp
// NEW: Result type with error context
template<typename T>
class Result {
    std::variant<T, Error> value_;
public:
    bool isOk() const;
    bool isErr() const;
    T& unwrap();
    const Error& error() const;
    T unwrapOr(T defaultValue);
};

struct Error {
    ErrorCode code;
    std::string message;
    std::string file;
    int line;
    std::vector<Error> causes;  // Error chain
};

// Usage:
Result<MCTSNode*> expand(MCTSNode* node) {
    auto result = createChildNode(node, action);
    if (result.isErr()) {
        return Error{ErrorCode::ExpansionFailed, "...", __FILE__, __LINE__}
            .withCause(result.error());
    }
    return result.unwrap();
}
```

**Benefits**:
- Full error context for debugging
- Explicit error propagation
- Better testability
- Logging integration

**Effort**: Medium-High (3-4 days, pervasive change)

---

#### 2.3 Configuration Management Fragmented

**Location**: `mcts_search.h:40-61`, `i_tactical_evaluator.h:31-48`, `MCTS_AI.h:68-84`

**Issue**: Configuration is split across **multiple struct types** with no centralized management or validation.

**Current State**:
- `MCTSConfig` (search parameters)
- `EvaluationContext` (evaluator weights)
- `ExecutionContext` (executor flags)
- `MCTS_AI::Profile` (high-level settings)

**Problems**:
- No single source of truth
- Duplicate parameters (e.g., logging flags in multiple places)
- No validation (can set invalid values)
- Difficult to serialize/deserialize
- No configuration versioning for save compatibility

**Recommended Solution**:
```cpp
// NEW: Unified configuration system
class MCTSConfiguration {
    ConfigSchema schema_;
    std::unordered_map<std::string, ConfigValue> values_;

public:
    // Type-safe getters
    template<typename T>
    T get(const std::string& key) const;

    // Validation
    bool validate() const;
    std::vector<ConfigError> getErrors() const;

    // Serialization
    void loadFromFile(const std::string& path);
    void saveToFile(const std::string& path) const;
    void loadFromIni(const std::string& iniContent);

    // Versioning
    int getVersion() const;
    void migrate(int fromVersion, int toVersion);
};

// Define schema once
ConfigSchema createMCTSSchema() {
    return ConfigSchema()
        .addInt("maxIterations", 1000, 1, 100000)
        .addDouble("explorationConstant", 1.414, 0.0, 10.0)
        .addBool("enableLogging", false)
        .addEnum("rolloutPolicy", "random", {"random", "heuristic", "mixed"});
}
```

**Benefits**:
- Single configuration interface
- Built-in validation
- Easy serialization
- Version migration support

**Effort**: Medium-High (3-4 days)

---

#### 2.4 Excessive Use of Raw Pointers in Node Tree

**Location**: `mcts_node.h:337`, `mcts_search.cpp`

**Issue**: Mix of **raw pointers** (parent) and **unique_ptr** (children) creates confusion and potential lifetime issues.

**Current Code**:
```cpp
class MCTSNode {
    MCTSNode* parent_;  // Raw pointer (parent owns us)
    std::vector<std::unique_ptr<MCTSNode>> children_;  // We own children
};
```

**Problems**:
- Asymmetric ownership model
- Comments needed to explain ownership ("parent owns us")
- Easy to dangle parent pointer if tree restructured
- No compile-time safety

**Recommended Solution**:
```cpp
// Option 1: Use observer_ptr or non_null<T*> from GSL
class MCTSNode {
    gsl::not_null<MCTSNode*> parent_;  // Non-owning, never null
    std::vector<std::unique_ptr<MCTSNode>> children_;
};

// Option 2: Intrusive tree structure (more cache-friendly)
class MCTSNode {
    size_t parentIndex_;  // Index into tree's flat array
    std::vector<size_t> childrenIndices_;
};

class MCTSTree {
    std::vector<MCTSNode> nodes_;  // Flat array, cache-friendly
};
```

**Benefits**:
- Clearer ownership semantics
- Better cache locality (flat array)
- Safer pointer handling

**Effort**: Medium (2-3 days, requires careful testing)

---

### 🟢 PRIORITY 3: Extensibility Enhancements

#### 3.1 No Plugin Architecture for Evaluators

**Location**: `evaluator_factory.cpp`, `i_tactical_evaluator.h`

**Issue**: Evaluators can only be added by modifying `EvaluatorFactory` source code. No dynamic plugin loading.

**Impact**:
- Users cannot add custom evaluators without recompiling
- Difficult to A/B test evaluator variants
- No support for domain-specific evaluators

**Recommended Solution**:
```cpp
// NEW: Evaluator plugin system
class EvaluatorRegistry {
    std::unordered_map<std::string, EvaluatorFactory> factories_;
public:
    void registerEvaluator(
        const std::string& name,
        std::function<std::unique_ptr<ITacticalEvaluator>()> factory);

    std::unique_ptr<ITacticalEvaluator> create(const std::string& name) const;
    std::vector<std::string> getAvailableEvaluators() const;
};

// User can add evaluator:
EvaluatorRegistry::global().registerEvaluator(
    "my_custom_evaluator",
    []() { return std::make_unique<MyCustomEvaluator>(); }
);
```

**Benefits**:
- User extensibility
- Dynamic evaluator selection
- Easy A/B testing

**Effort**: Low-Medium (1-2 days)

---

#### 3.2 Limited Observability and Instrumentation

**Location**: Entire codebase - minimal logging/metrics

**Issue**: No structured logging, metrics, or telemetry for debugging or performance analysis.

**Current State**:
- Occasional debug messages
- No performance counters
- No structured event logging
- No profiling hooks

**Recommended Solution**:
```cpp
// NEW: Instrumentation layer
class MCTSInstrumentation {
public:
    virtual void onIterationStart(int iteration) {}
    virtual void onIterationEnd(int iteration, double value) {}
    virtual void onNodeExpanded(const MCTSNode& node) {}
    virtual void onRolloutStart(const GameStateSnapshot& state) {}
    virtual void onRolloutEnd(double value, int depth) {}
    virtual void onActionEvaluated(const Action& action, double score) {}
};

// Implementations:
class LoggingInstrumentation : public MCTSInstrumentation { /* logs */ };
class MetricsInstrumentation : public MCTSInstrumentation { /* metrics */ };
class ProfilingInstrumentation : public MCTSInstrumentation { /* timings */ };

// In MCTSSearch:
std::vector<std::shared_ptr<MCTSInstrumentation>> instruments_;
```

**Benefits**:
- Deep visibility into MCTS behavior
- Performance profiling
- Debugging support
- Analytics for tuning

**Effort**: Medium (2-3 days)

---

#### 3.3 No Multi-Level Abstraction Support

**Location**: Architecture-wide

**Issue**: MCTS operates at single abstraction level (tactical). No support for **hierarchical planning** (strategic + tactical).

**Impact**:
- Cannot plan long-term strategies (base building, resource management)
- AI limited to immediate tactical decisions
- Difficult to add strategic objectives

**Recommended Solution**:
```cpp
// NEW: Hierarchical state and action abstraction
class IAbstractionLevel {
public:
    virtual ~IAbstractionLevel() = default;
    virtual std::vector<AbstractAction> getAbstractActions(
        const IGameState& state) const = 0;
    virtual std::vector<Action> refineAction(
        const AbstractAction& abstractAction,
        const IGameState& state) const = 0;
};

// Two levels:
class StrategicLevel : public IAbstractionLevel {
    // Actions: "capture base", "defend sector", "attack player"
};

class TacticalLevel : public IAbstractionLevel {
    // Actions: move, attack, wait (current)
};

// Hierarchical MCTS:
class HierarchicalMCTS {
    std::vector<std::unique_ptr<IAbstractionLevel>> levels_;
    std::vector<std::unique_ptr<MCTSSearch>> searches_;
};
```

**Benefits**:
- Long-term planning capability
- Strategic + tactical AI
- Better scalability to complex scenarios

**Effort**: High (1-2 weeks, new feature)

---

#### 3.4 Tree Persistence Not Implemented

**Location**: `mcts_search.h:159-162` (reset method discards tree)

**Issue**: Search tree is **discarded between turns**, losing valuable search information.

**Impact**:
- Wasted computation (tree could be reused)
- No learning across turns
- Cannot implement tree reuse optimization

**Recommended Solution**:
```cpp
// NEW: Tree persistence and reuse
class MCTSSearch {
    std::unique_ptr<MCTSNode> root_;

public:
    // Reuse tree from previous search
    void reuseTree(const GameStateSnapshot& newState) {
        if (!root_) return;

        // Find child node matching new state
        auto* matchingChild = findMatchingChild(newState);
        if (matchingChild) {
            // Promote child to new root
            promoteChild(matchingChild);
        } else {
            // State diverged, start fresh
            reset();
        }
    }

    // Serialize tree to disk
    void saveTree(const std::string& path) const;
    void loadTree(const std::string& path);
};
```

**Benefits**:
- Faster subsequent searches
- Better long-term planning
- Tree can be analyzed offline

**Effort**: Medium-High (3-4 days)

---

### 🟣 PRIORITY 4: Code Quality Issues

#### 4.1 Inconsistent Naming Conventions

**Issue**: Mix of **camelCase**, **snake_case**, and **PascalCase** across codebase.

**Examples**:
- `networkID` (camelCase) vs. `owner` (snake_case) in `UnitSnapshot`
- `getUCB1Value` (mixed) vs. `selectBestChildByVisits` (camelCase)
- `MapCoordinate` (PascalCase) vs. `unitIndexById` (camelCase)

**Impact**: Reduced readability, cognitive load

**Recommendation**: Adopt consistent naming convention (suggest: camelCase for methods, snake_case for members)

**Effort**: Low (1 day, automated refactoring)

---

#### 4.2 Magic Numbers Throughout Code

**Location**: Multiple files

**Examples**:
```cpp
// simple_combat_evaluator.cpp:296
const int RF_RANGE = 10;  // Why 10?

// game_state_snapshot.h:324
constexpr float MAX_HEIGHT = 7.0f;  // Why 7?

// simulation_action_executor.h:114
static constexpr size_t MAX_UNDO_DEPTH = 10;  // Why 10?
```

**Impact**: Hard to tune, unclear rationale

**Recommendation**: Move constants to configuration or named constants with documentation

**Effort**: Low (1 day)

---

#### 4.3 Missing Unit Tests for Core Components

**Location**: Limited test coverage

**Current State**:
- Some test executables (`action_executor_test`, `evaluator_test`, `snapshot_test`, `mcts_test`)
- No coverage metrics
- No CI/CD integration
- Manual test execution

**Impact**: Refactoring risk, regression potential

**Recommendation**:
- Add comprehensive unit tests (target 80% coverage)
- Add integration tests
- Set up CI/CD with automated testing

**Effort**: High (1-2 weeks)

---

#### 4.4 Documentation Inconsistency

**Issue**: Mix of excellent header documentation and sparse inline comments.

**Examples**:
- Headers: Excellent documentation (e.g., `mcts_search.h`)
- Implementation files: Sparse comments (e.g., `simple_combat_evaluator.cpp`)
- Design decisions not documented inline

**Recommendation**:
- Add inline comments for complex logic
- Document "why" not just "what"
- Add complexity analysis comments

**Effort**: Low-Medium (2-3 days)

---

## Cross-Cutting Concerns

### Performance Considerations

1. **Memory Allocation**: Heavy use of `std::unique_ptr` and `std::vector` causes frequent allocations. Consider object pooling for nodes.

2. **Cache Locality**: Tree traversal uses pointer chasing (poor cache performance). Consider flat array representation.

3. **Virtual Function Overhead**: Interfaces use virtual functions (acceptable, but measurable overhead).

4. **Copy Performance**: `GameStateSnapshot::clone()` is fast but called frequently. Profile and optimize hot path.

### Thread Safety

**Current State**: No thread safety guarantees.

**Concerns**:
- MCTS is embarrassingly parallel (could run multiple rollouts in parallel)
- No synchronization primitives
- Mutable caching in `GameStateSnapshot` not thread-safe

**Recommendation**: Add thread-safety analysis and parallel MCTS support.

**Effort**: High (1 week)

---

### Testing Strategy Gaps

1. **Property-Based Testing**: No property-based tests (e.g., "all actions should be reversible")
2. **Fuzz Testing**: No fuzz testing for action parsing/execution
3. **Performance Benchmarks**: No benchmarking suite
4. **Integration Tests**: Limited integration testing with real game

**Recommendation**: Comprehensive testing strategy.

**Effort**: High (2 weeks)

---

## Technical Debt Assessment

### Debt Categories

| Category | Amount | Interest Rate | Priority |
|----------|--------|---------------|----------|
| Action space abstraction | High | High | P1 |
| State abstraction | Medium | Medium | P1 |
| Error handling | Medium | High | P2 |
| Configuration management | Medium | Medium | P2 |
| Testing infrastructure | High | Medium | P3 |
| Documentation | Low | Low | P4 |
| Performance optimization | Medium | Low | P4 |

### Technical Debt Cost

**Total Effort to Address Critical Issues**: ~15-20 developer-days

**Total Effort to Address All Issues**: ~40-50 developer-days

**Interest Cost** (if not addressed):
- P1 issues: +50% development time for each new feature
- P2 issues: +20% debugging/maintenance time
- P3 issues: Occasional production issues
- P4 issues: Developer frustration, onboarding difficulty

---

## Recommendations

### Immediate Actions (Next Sprint)

1. **Add action generator abstraction** (P1.1) - Unblocks extensibility
2. **Implement Result<T> error handling** (P2.2) - Improves debugging
3. **Fix caching in GameStateSnapshot** (P2.1) - Prevents bugs
4. **Add basic instrumentation** (P3.2) - Enables profiling

### Short-Term (1-2 Months)

1. **Refactor to IGameState interface** (P1.2) - Major architectural improvement
2. **Implement configuration management** (P2.3) - Improves usability
3. **Add rollout policy abstraction** (P1.4) - Enables experimentation
4. **Build test infrastructure** (P4.3) - Reduces regression risk

### Long-Term (3-6 Months)

1. **Hierarchical planning support** (P3.3) - Strategic AI capability
2. **Tree persistence and reuse** (P3.4) - Performance optimization
3. **Parallel MCTS** (Thread Safety) - Scalability
4. **Plugin architecture** (P3.1) - User extensibility

---

## Conclusion

The MCTS AI codebase demonstrates **strong design principles** and **good C++23 usage**, but suffers from **limited extensibility** due to concrete dependencies and **maintainability concerns** around error handling and configuration.

**Key Strengths**:
- Clean interface separation (IActionExecutor, ITacticalEvaluator)
- Modern C++23 usage
- Well-documented headers
- Sound MCTS algorithm implementation

**Key Weaknesses**:
- Hard-coded action space (biggest extensibility blocker)
- No state abstraction (limits experimentation)
- Fragmented configuration
- Missing error context
- Limited testing infrastructure

**Bottom Line**: The architecture is solid but needs **strategic refactoring** to unlock full extensibility potential. Priority 1 issues must be addressed before adding significant new features to avoid accumulating technical debt.

---

**Document Version**: 1.0
**Review Date**: 2025-11-09
**Next Review**: After P1 issues addressed
