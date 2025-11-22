/**
 * Game Event Dispatcher - Phase 1 of GameMap Refactoring
 *
 * Purpose: Abstract event notification system to replace sigc::signal
 * Goal: Decouple game logic from UI notification mechanism
 *
 * This replaces the direct sigc++ dependency in GameMap with a clean
 * abstraction that can work with:
 * - Legacy UI (via LegacyUIEventAdapter)
 * - Headless server (via HeadlessEventDispatcher)
 * - Network clients (via WebSocketEventBroadcaster)
 * - Test mocks (via MockEventListener)
 *
 * Created: 2025-11-20
 * Part of: GAMEMAP_REFACTORING_PLAN.md - Phase 1
 */

#ifndef GAME_EVENT_DISPATCHER_H
#define GAME_EVENT_DISPATCHER_H

#include <memory>
#include <vector>
#include <queue>
#include <string>
#include <ctime>

// Forward declarations to avoid including heavy headers
class Player;
class GameMap;

namespace asc {
namespace core {
namespace events {

/**
 * Game Event Types
 *
 * These correspond to the existing sigc::signals in GameMap
 * (see gamemap.h:466-481)
 */
enum class GameEventType {
   // Player turn events
   PlayerTurnBegins,           // sigc::signal<void, Player&> sigPlayerTurnBegins
   PlayerTurnEnds,             // sigc::signal<void, Player&> sigPlayerTurnEnds
   PlayerTurnHasEnded,         // sigc::signal<void, Player&> sigPlayerTurnHasEnded

   // Player interaction events
   PlayerUserInteractionBegins, // sigc::signal<void, Player&> sigPlayerUserInteractionBegins
   PlayerUserInteractionEnds,   // sigc::signal<void, Player&> sigPlayerUserInteractionEnds

   // Game state events
   MapWon,                     // sigc::signal<void, Player&> sigMapWon
   RoundStarts,                // sigc::signal<void> newRound
   CoordinateShift,            // sigc::signal<void, const MapCoodinateVector&> sigCoordinateShift

   // Map lifecycle events (static signals)
   MapCreated,                 // static sigc::signal<void, GameMap&> sigMapCreation
   MapDestroyed                // static sigc::signal<void, GameMap&> sigMapDeletion
};

/**
 * Base Game Event
 *
 * Polymorphic event base class. Events are value types that can be
 * serialized, logged, and transmitted over the network.
 */
struct GameEvent {
   GameEventType type;
   time_t timestamp;

   explicit GameEvent(GameEventType eventType)
      : type(eventType), timestamp(std::time(nullptr)) {}

   virtual ~GameEvent() = default;

   // Event must be copyable for queuing
   GameEvent(const GameEvent&) = default;
   GameEvent& operator=(const GameEvent&) = default;
};

/**
 * Player-related Event
 *
 * Most game events involve a player (turn changes, victory, etc.)
 */
struct PlayerEvent : public GameEvent {
   int playerId;  // Player index (0-8)

   PlayerEvent(GameEventType eventType, int player)
      : GameEvent(eventType), playerId(player) {}
};

/**
 * Coordinate Shift Event
 *
 * Fired when map coordinates change (for minimap updates, etc.)
 */
struct CoordinateShiftEvent : public GameEvent {
   // In future: store coordinate vector here
   // For now, listeners query GameMap directly

   CoordinateShiftEvent()
      : GameEvent(GameEventType::CoordinateShift) {}
};

/**
 * Map Lifecycle Event
 *
 * Fired when a GameMap is created or destroyed
 */
struct MapLifecycleEvent : public GameEvent {
   GameMap* map;  // Non-owning pointer

   MapLifecycleEvent(GameEventType eventType, GameMap* gameMap)
      : GameEvent(eventType), map(gameMap) {}
};

/**
 * Event Listener Interface
 *
 * Implement this to receive game events. Can filter by type.
 */
class IGameEventListener {
public:
   virtual ~IGameEventListener() = default;

   /**
    * Called when an event occurs
    * @param event The event (use dynamic_cast to get specific type)
    */
   virtual void onGameEvent(const GameEvent& event) = 0;
};

/**
 * Game Event Dispatcher
 *
 * Manages event listeners and dispatches events.
 *
 * Usage:
 *   GameEventDispatcher dispatcher;
 *   dispatcher.subscribe(myListener);
 *
 *   auto event = std::make_unique<PlayerEvent>(
 *      GameEventType::PlayerTurnBegins, playerId);
 *   dispatcher.dispatch(std::move(event));
 */
class GameEventDispatcher {
private:
   // Registered listeners
   std::vector<IGameEventListener*> listeners;

   // Event queue for async processing (future enhancement)
   std::queue<std::unique_ptr<GameEvent>> eventQueue;

   // Synchronous vs asynchronous dispatch
   bool synchronousMode;

public:
   GameEventDispatcher() : synchronousMode(true) {}

   ~GameEventDispatcher() = default;

   // Non-copyable (manages listener lifetimes)
   GameEventDispatcher(const GameEventDispatcher&) = delete;
   GameEventDispatcher& operator=(const GameEventDispatcher&) = delete;

   /**
    * Subscribe to all events
    * @param listener Listener to add (must outlive dispatcher)
    */
   void subscribe(IGameEventListener* listener);

   /**
    * Unsubscribe from events
    * @param listener Listener to remove
    */
   void unsubscribe(IGameEventListener* listener);

   /**
    * Dispatch an event to all listeners
    * @param event Event to dispatch (ownership transferred)
    *
    * In synchronous mode (default), calls listeners immediately.
    * In async mode, queues for later processing.
    */
   void dispatch(std::unique_ptr<GameEvent> event);

   /**
    * Process queued events (async mode only)
    * Call this each game loop iteration
    */
   void processQueue();

   /**
    * Enable/disable synchronous mode
    * @param sync true for immediate dispatch, false for queued
    */
   void setSynchronousMode(bool sync) { synchronousMode = sync; }

   /**
    * Check if any listeners are registered
    */
   bool hasListeners() const { return !listeners.empty(); }

   /**
    * Get number of registered listeners
    */
   size_t getListenerCount() const { return listeners.size(); }
};

} // namespace events
} // namespace core
} // namespace asc

#endif // GAME_EVENT_DISPATCHER_H
