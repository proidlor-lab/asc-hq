/**
 * Game Event Dispatcher Implementation
 *
 * Created: 2025-11-20
 * Part of: GAMEMAP_REFACTORING_PLAN.md - Phase 1
 */

#include "game_event_dispatcher.h"
#include <algorithm>

namespace asc {
namespace core {
namespace events {

void GameEventDispatcher::subscribe(IGameEventListener* listener) {
   if (!listener) {
      return;  // Null check
   }

   // Check if already subscribed
   auto it = std::find(listeners.begin(), listeners.end(), listener);
   if (it != listeners.end()) {
      return;  // Already subscribed
   }

   listeners.push_back(listener);
}

void GameEventDispatcher::unsubscribe(IGameEventListener* listener) {
   auto it = std::find(listeners.begin(), listeners.end(), listener);
   if (it != listeners.end()) {
      listeners.erase(it);
   }
}

void GameEventDispatcher::dispatch(std::unique_ptr<GameEvent> event) {
   if (!event) {
      return;  // Null check
   }

   if (synchronousMode) {
      // Immediate dispatch to all listeners
      for (auto* listener : listeners) {
         if (listener) {
            listener->onGameEvent(*event);
         }
      }
   } else {
      // Queue for async processing
      eventQueue.push(std::move(event));
   }
}

void GameEventDispatcher::processQueue() {
   // Process all queued events
   while (!eventQueue.empty()) {
      auto& event = eventQueue.front();

      // Dispatch to all listeners
      for (auto* listener : listeners) {
         if (listener) {
            listener->onGameEvent(*event);
         }
      }

      eventQueue.pop();
   }
}

} // namespace events
} // namespace core
} // namespace asc
