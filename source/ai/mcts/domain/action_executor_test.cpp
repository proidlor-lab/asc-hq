/***************************************************************************
 * action_executor_test.cpp - Unit tests for action execution
 * 
 * Part of: ASC MCTS AI (Phase 0.2 - Action Execution Interface)
 ***************************************************************************/

#include "simulation_action_executor.h"
#include "i_action_executor.h"
#include "action_types.h"
#include <iostream>
#include <cassert>
#include <iomanip>

using namespace asc::mcts;

// ========== Test Utilities ==========

class TestReporter {
public:
    void startTest(const std::string& name) {
        std::cout << "\n[TEST] " << name << "..." << std::endl;
        currentTest_ = name;
    }
    
    void pass(const std::string& detail = "") {
        std::cout << "  ✓ PASS";
        if (!detail.empty()) {
            std::cout << ": " << detail;
        }
        std::cout << std::endl;
        passCount_++;
    }
    
    void fail(const std::string& detail = "") {
        std::cout << "  ✗ FAIL";
        if (!detail.empty()) {
            std::cout << ": " << detail;
        }
        std::cout << std::endl;
        failCount_++;
    }
    
    void summary() {
        std::cout << "\n========================================" << std::endl;
        std::cout << "Test Summary:" << std::endl;
        std::cout << "  Passed: " << passCount_ << std::endl;
        std::cout << "  Failed: " << failCount_ << std::endl;
        std::cout << "  Total:  " << (passCount_ + failCount_) << std::endl;
        std::cout << "========================================" << std::endl;
        
        if (failCount_ == 0) {
            std::cout << "\n✓ ALL TESTS PASSED" << std::endl;
        } else {
            std::cout << "\n✗ SOME TESTS FAILED" << std::endl;
        }
    }
    
    bool allPassed() const { return failCount_ == 0; }
    
private:
    std::string currentTest_;
    int passCount_ = 0;
    int failCount_ = 0;
};

// ========== Test Fixture ==========

std::unique_ptr<GameStateSnapshot> createTestSnapshot() {
    auto snapshot = std::make_unique<GameStateSnapshot>();
    
    snapshot->mapWidth = 20;
    snapshot->mapHeight = 20;
    snapshot->currentPlayer = 0;
    snapshot->perspective = 0;
    
    // Add test units
    UnitSnapshot unit1;
    unit1.networkID = 1;
    unit1.x = 5;
    unit1.y = 5;
    unit1.height = 0;
    unit1.owner = 0;
    unit1.damage = 0;
    unit1.movement = 500;
    unit1.fuel = 1000;
    unit1.attacked = false;
    unit1.ammoMask = 0xFFFF;
    unit1.type = nullptr; // Will need actual VehicleType for real tests
    
    UnitSnapshot unit2;
    unit2.networkID = 2;
    unit2.x = 8;
    unit2.y = 5;
    unit2.height = 0;
    unit2.owner = 1; // Enemy
    unit2.damage = 0;
    unit2.movement = 300;
    unit2.fuel = 800;
    unit2.attacked = false;
    unit2.ammoMask = 0xFFFF;
    unit2.type = nullptr;
    
    snapshot->addUnit(unit1);
    snapshot->addUnit(unit2);
    
    return snapshot;
}

// ========== Test Cases ==========

void testActionTypeCreation(TestReporter& reporter) {
    reporter.startTest("Action Type Creation");
    
    MoveAction move{1, MapCoordinate(6, 5), 0};
    assert(move.unitID == 1);
    assert(move.destination.x == 6);
    assert(move.destination.y == 5);
    reporter.pass("MoveAction created");
    
    AttackAction attack{1, MapCoordinate(8, 5), 0};
    assert(attack.attackerID == 1);
    assert(attack.target.x == 8);
    reporter.pass("AttackAction created");
    
    WaitAction wait{1};
    assert(wait.unitID == 1);
    reporter.pass("WaitAction created");
}

void testActionVariant(TestReporter& reporter) {
    reporter.startTest("Action Variant");
    
    Action action = MoveAction{1, MapCoordinate(6, 5)};
    std::string typeName = getActionTypeName(action);
    assert(typeName == "Move");
    reporter.pass("Action variant holds MoveAction");
    
    action = AttackAction{1, MapCoordinate(8, 5)};
    typeName = getActionTypeName(action);
    assert(typeName == "Attack");
    reporter.pass("Action variant holds AttackAction");
    
    UnitID unitID = getActionUnitID(action);
    assert(unitID == 1);
    reporter.pass("getActionUnitID works");
}

void testExecutorCreation(TestReporter& reporter) {
    reporter.startTest("Executor Creation");
    
    auto snapshot = createTestSnapshot();
    auto executor = std::make_unique<SimulationActionExecutor>(std::move(snapshot));
    
    const auto& state = executor->getState();
    assert(state.units.size() == 2);
    reporter.pass("SimulationActionExecutor created with 2 units");
    
    assert(state.mapWidth == 20);
    assert(state.mapHeight == 20);
    reporter.pass("Map dimensions correct");
}

void testMoveActionLegality(TestReporter& reporter) {
    reporter.startTest("Move Action Legality");
    
    auto snapshot = createTestSnapshot();
    auto executor = std::make_unique<SimulationActionExecutor>(std::move(snapshot));
    
    // Legal move to neighbor
    MoveAction legalMove{1, MapCoordinate(6, 5)};
    ActionResult result = executor->isLegal(legalMove);
    if (result.isSuccess()) {
        reporter.pass("Legal move to neighbor");
    } else {
        reporter.fail("Legal move rejected: " + result.toString());
    }
    
    // Illegal move (occupied)
    MoveAction illegalMove{1, MapCoordinate(8, 5)};
    result = executor->isLegal(illegalMove);
    if (result.isFailure()) {
        reporter.pass("Illegal move to occupied field rejected");
    } else {
        reporter.fail("Illegal move accepted");
    }
    
    // Illegal move (nonexistent unit)
    MoveAction nonexistentUnit{999, MapCoordinate(6, 5)};
    result = executor->isLegal(nonexistentUnit);
    if (result.code == ActionResultCode::UnitNotFound) {
        reporter.pass("Nonexistent unit rejected");
    } else {
        reporter.fail("Nonexistent unit not properly rejected");
    }
}

void testMoveActionExecution(TestReporter& reporter) {
    reporter.startTest("Move Action Execution");
    
    auto snapshot = createTestSnapshot();
    auto executor = std::make_unique<SimulationActionExecutor>(std::move(snapshot));
    
    // Execute legal move
    ExecutionContext context;
    context.enableReactionFire = false; // Disable RF for simple test
    
    MoveAction move{1, MapCoordinate(6, 5)};
    ActionResult result = executor->execute(move, context);
    
    if (result.isSuccess()) {
        reporter.pass("Move executed successfully");
    } else {
        reporter.fail("Move execution failed: " + result.toString());
        return;
    }
    
    // Verify unit moved
    const auto& state = executor->getState();
    const UnitSnapshot* unit = state.findUnit(1);
    assert(unit != nullptr);
    
    if (unit->x == 6 && unit->y == 5) {
        reporter.pass("Unit position updated correctly");
    } else {
        reporter.fail("Unit position not updated (at " + 
                     std::to_string(unit->x) + "," + std::to_string(unit->y) + ")");
    }
    
    if (result.movementConsumed > 0) {
        reporter.pass("Movement consumed: " + std::to_string(result.movementConsumed));
    } else {
        reporter.fail("Movement not consumed");
    }
}

void testAttackActionLegality(TestReporter& reporter) {
    reporter.startTest("Attack Action Legality");
    
    auto snapshot = createTestSnapshot();
    auto executor = std::make_unique<SimulationActionExecutor>(std::move(snapshot));
    
    // Legal attack
    AttackAction legalAttack{1, MapCoordinate(8, 5)};
    ActionResult result = executor->isLegal(legalAttack);
    if (result.isSuccess()) {
        reporter.pass("Legal attack");
    } else {
        reporter.fail("Legal attack rejected: " + result.toString());
    }
    
    // Illegal attack (no target)
    AttackAction noTarget{1, MapCoordinate(10, 10)};
    result = executor->isLegal(noTarget);
    if (result.code == ActionResultCode::TargetNotFound) {
        reporter.pass("Attack with no target rejected");
    } else {
        reporter.fail("Attack with no target not properly rejected");
    }
}

void testAttackActionExecution(TestReporter& reporter) {
    reporter.startTest("Attack Action Execution");
    
    auto snapshot = createTestSnapshot();
    auto executor = std::make_unique<SimulationActionExecutor>(std::move(snapshot));
    
    // Get defender's initial HP
    const UnitSnapshot* defenderBefore = executor->getState().findUnit(2);
    int initialDamage = defenderBefore->damage;
    
    // Execute attack
    ExecutionContext context;
    AttackAction attack{1, MapCoordinate(8, 5), 0};
    ActionResult result = executor->execute(attack, context);
    
    if (result.isSuccess()) {
        reporter.pass("Attack executed successfully");
    } else {
        reporter.fail("Attack execution failed: " + result.toString());
        return;
    }
    
    if (result.damageDealt > 0) {
        reporter.pass("Damage dealt: " + std::to_string(result.damageDealt));
    } else {
        reporter.fail("No damage dealt");
    }
    
    // Verify defender took damage
    const UnitSnapshot* defenderAfter = executor->getState().findUnit(2);
    if (defenderAfter->damage > initialDamage) {
        reporter.pass("Defender took damage (" + 
                     std::to_string(initialDamage) + " -> " + 
                     std::to_string(defenderAfter->damage) + ")");
    } else {
        reporter.fail("Defender did not take damage");
    }
    
    // Verify attacker marked as attacked
    const UnitSnapshot* attacker = executor->getState().findUnit(1);
    if (attacker->attacked) {
        reporter.pass("Attacker marked as attacked");
    } else {
        reporter.fail("Attacker not marked as attacked");
    }
}

void testWaitActionExecution(TestReporter& reporter) {
    reporter.startTest("Wait Action Execution");
    
    auto snapshot = createTestSnapshot();
    auto executor = std::make_unique<SimulationActionExecutor>(std::move(snapshot));
    
    // Get initial movement
    const UnitSnapshot* unitBefore = executor->getState().findUnit(1);
    int initialMovement = unitBefore->movement;
    
    // Execute wait
    ExecutionContext context;
    WaitAction wait{1};
    ActionResult result = executor->execute(wait, context);
    
    if (result.isSuccess()) {
        reporter.pass("Wait executed successfully");
    } else {
        reporter.fail("Wait execution failed: " + result.toString());
    }
    
    // Verify movement consumed
    const UnitSnapshot* unitAfter = executor->getState().findUnit(1);
    if (unitAfter->movement == 0) {
        reporter.pass("Movement consumed by wait");
    } else {
        reporter.fail("Movement not consumed by wait");
    }
}

void testActionGeneration(TestReporter& reporter) {
    reporter.startTest("Action Generation");
    
    auto snapshot = createTestSnapshot();
    auto executor = std::make_unique<SimulationActionExecutor>(std::move(snapshot));
    
    // Generate actions for unit 1
    auto actions = executor->generateLegalActions(1, true);
    
    if (!actions.empty()) {
        reporter.pass("Generated " + std::to_string(actions.size()) + " actions");
    } else {
        reporter.fail("No actions generated");
    }
    
    // Check for move actions
    bool hasMoves = false;
    bool hasAttacks = false;
    bool hasWait = false;
    
    for (const auto& action : actions) {
        std::string typeName = getActionTypeName(action);
        if (typeName == "Move") hasMoves = true;
        if (typeName == "Attack") hasAttacks = true;
        if (typeName == "Wait") hasWait = true;
    }
    
    if (hasMoves) {
        reporter.pass("Move actions generated");
    } else {
        reporter.fail("No move actions generated");
    }
    
    if (hasAttacks) {
        reporter.pass("Attack actions generated");
    } else {
        reporter.fail("No attack actions generated");
    }
    
    if (hasWait) {
        reporter.pass("Wait action generated");
    } else {
        reporter.fail("No wait action generated");
    }
}

void testExecutorCloning(TestReporter& reporter) {
    reporter.startTest("Executor Cloning");
    
    auto snapshot = createTestSnapshot();
    auto executor1 = std::make_unique<SimulationActionExecutor>(std::move(snapshot));
    
    // Clone executor
    auto executor2 = executor1->clone();
    
    if (executor2) {
        reporter.pass("Executor cloned");
    } else {
        reporter.fail("Failed to clone executor");
        return;
    }
    
    // Verify state is copied
    if (executor2->getState().units.size() == 2) {
        reporter.pass("Cloned state has correct unit count");
    } else {
        reporter.fail("Cloned state has wrong unit count");
    }
    
    // Modify original
    ExecutionContext context;
    context.enableReactionFire = false;
    MoveAction move{1, MapCoordinate(6, 5)};
    executor1->execute(move, context);
    
    // Verify clone is independent
    const UnitSnapshot* unit1 = executor1->getState().findUnit(1);
    const UnitSnapshot* unit2 = executor2->getState().findUnit(1);
    
    if (unit1->x != unit2->x || unit1->y != unit2->y) {
        reporter.pass("Cloned executor is independent");
    } else {
        reporter.fail("Cloned executor shares state with original");
    }
}

void testUndoFunctionality(TestReporter& reporter) {
    reporter.startTest("Undo Functionality");
    
    auto snapshot = createTestSnapshot();
    auto executor = std::make_unique<SimulationActionExecutor>(std::move(snapshot));
    
    // Save initial position
    const UnitSnapshot* unitBefore = executor->getState().findUnit(1);
    int16_t initialX = unitBefore->x;
    int16_t initialY = unitBefore->y;
    
    // Execute move
    ExecutionContext context;
    context.enableReactionFire = false;
    MoveAction move{1, MapCoordinate(6, 5)};
    ActionResult result = executor->execute(move, context);
    
    if (!result.isSuccess()) {
        reporter.fail("Move failed: " + result.toString());
        return;
    }
    
    // Undo
    bool undone = executor->undo();
    if (undone) {
        reporter.pass("Undo succeeded");
    } else {
        reporter.fail("Undo failed");
        return;
    }
    
    // Verify position restored
    const UnitSnapshot* unitAfter = executor->getState().findUnit(1);
    if (unitAfter->x == initialX && unitAfter->y == initialY) {
        reporter.pass("Position restored after undo");
    } else {
        reporter.fail("Position not restored after undo");
    }
}

// ========== Main ==========

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Action Executor Tests (Phase 0.2)" << std::endl;
    std::cout << "========================================" << std::endl;
    
    TestReporter reporter;
    
    try {
        testActionTypeCreation(reporter);
        testActionVariant(reporter);
        testExecutorCreation(reporter);
        testMoveActionLegality(reporter);
        testMoveActionExecution(reporter);
        testAttackActionLegality(reporter);
        testAttackActionExecution(reporter);
        testWaitActionExecution(reporter);
        testActionGeneration(reporter);
        testExecutorCloning(reporter);
        testUndoFunctionality(reporter);
        
    } catch (const std::exception& e) {
        std::cerr << "\n✗ EXCEPTION: " << e.what() << std::endl;
        return 1;
    }
    
    reporter.summary();
    
    return reporter.allPassed() ? 0 : 1;
}
