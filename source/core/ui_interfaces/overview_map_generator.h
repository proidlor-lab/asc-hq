/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef OVERVIEW_MAP_GENERATOR_H
#define OVERVIEW_MAP_GENERATOR_H

// Forward declarations - no need to include full headers in interface
class Surface;
class GameMap;
class MapCoordinate;

namespace asc {
namespace core {
namespace ui_interfaces {

/**
 * @brief Interface for generating overview/minimap representations of the game map
 *
 * This interface abstracts the UI concern of generating a visual overview
 * of the game map. Implementations are responsible for:
 * - Creating visual representations of the map state
 * - Updating the overview when the map changes
 * - Handling fog of war and player visibility
 *
 * The interface allows the core game logic to remain independent of
 * graphics libraries (SDL, Surface, etc.), enabling headless operation.
 *
 * Phase 2 of GameMap refactoring: Extract UI rendering from core game logic
 */
class IOverviewMapGenerator {
  public:
   virtual ~IOverviewMapGenerator() = default;

   /**
    * @brief Generate or retrieve the current overview map surface
    * @param complete If true, complete the image immediately (may take several seconds).
    *                 If false, return the current partial image.
    * @return Reference to the overview map surface
    */
   virtual const Surface& getOverviewMap(bool complete = true) = 0;

   /**
    * @brief Update a specific field on the overview map
    * @param pos The map coordinate to update
    * @return true if successful, false otherwise
    */
   virtual bool updateField(const MapCoordinate& pos) = 0;

   /**
    * @brief Notify that the map has been resized
    * Implementations should reinitialize internal buffers
    */
   virtual void resetSize() = 0;

   /**
    * @brief Clear the overview map and restart generation
    * @param allImages If true, clear all cached images. If false, clear only the working image.
    */
   virtual void clear(bool allImages = false) = 0;

   /**
    * @brief Start updating the overview map from scratch
    * Typically called after map modifications
    */
   virtual void startUpdate() = 0;

   /**
    * @brief Connect the generator to event sources
    * Implementations should hook into appropriate events (idle, map changes, etc.)
    */
   virtual void connect() = 0;
};

} // namespace ui_interfaces
} // namespace core
} // namespace asc

#endif // OVERVIEW_MAP_GENERATOR_H
