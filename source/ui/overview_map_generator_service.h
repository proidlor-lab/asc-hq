/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef OVERVIEW_MAP_GENERATOR_SERVICE_H
#define OVERVIEW_MAP_GENERATOR_SERVICE_H

#include "core/ui_interfaces/overview_map_generator.h"
#include "game_event_dispatcher.h"
#include "graphics/surface.h"
#include <sigc++/trackable.h>
#include <sigc++/signal.h>

class GameMap;
class MapCoordinate;

namespace asc {
namespace ui {

/**
 * @brief Concrete implementation of IOverviewMapGenerator
 *
 * Generates a minimap/overview image of the game map progressively.
 * Uses an idle handler to draw fields incrementally to avoid blocking the UI.
 *
 * This class was extracted from GameMap::OverviewMapHolder during Phase 2
 * of the GameMap refactoring to decouple UI rendering from core game logic.
 *
 * Features:
 * - Progressive rendering (doesn't block UI)
 * - Handles fog of war and visibility
 * - Shows terrain, buildings, units
 * - Signals when generation is complete
 */
class OverviewMapGeneratorService : public core::ui_interfaces::IOverviewMapGenerator,
                                     public core::events::IGameEventListener,
                                     public sigc::trackable {
  private:
   GameMap& map;
   Surface overviewMapImage;
   Surface completedMapImage;
   bool initialized;
   bool secondMapReady;
   bool completed;
   bool connected;
   int x; // Current x position during progressive rendering
   int y; // Current y position during progressive rendering

   /**
    * @brief Idle handler called during GUI idle time
    * Draws several fields at a time to make progress without blocking
    */
   bool idleHandler();

   /**
    * @brief Initialize the overview map surface
    * @return true if initialization succeeded
    */
   bool init();

   /**
    * @brief Draw the next field in the progressive rendering sequence
    * @param signalOnCompletion If true, emit generationComplete signal when done
    */
   void drawNextField(bool signalOnCompletion = true);

   /**
    * @brief Create a new surface sized appropriately for the current map
    */
   Surface createNewSurface();

  public:
   /**
    * @brief Construct the overview map generator for a specific game map
    * @param gamemap The game map to generate an overview for
    */
   explicit OverviewMapGeneratorService(GameMap& gamemap);

   // IOverviewMapGenerator interface implementation
   const Surface& getOverviewMap(bool complete = true) override;
   bool updateField(const MapCoordinate& pos) override;
   void resetSize() override;
   void clear(bool allImages = false) override;
   void startUpdate() override;
   void connect() override;

   // IGameEventListener interface implementation
   void onGameEvent(const core::events::GameEvent& event) override;

   /**
    * @brief Static signal emitted when overview map generation is complete
    * Legacy UI components can connect to this signal for backward compatibility
    */
   static sigc::signal<void> generationComplete;

   /**
    * @brief Static helper to clear an overview map
    * Provided for backward compatibility with legacy code
    */
   static void clearmap(GameMap* actmap);
};

} // namespace ui
} // namespace asc

#endif // OVERVIEW_MAP_GENERATOR_SERVICE_H
