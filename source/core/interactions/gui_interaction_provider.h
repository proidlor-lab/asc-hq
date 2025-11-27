/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GUI_INTERACTION_PROVIDER_H
#define GUI_INTERACTION_PROVIDER_H

#include "core/ui_interfaces/user_interaction_provider.h"

namespace asc {
namespace core {
namespace interactions {

/**
 * @brief GUI implementation of IUserInteractionProvider
 *
 * This implementation uses the legacy dialog system (ParaGUI-based) to
 * present actual dialogs to the user and wait for their response.
 *
 * This is the "full" implementation that provides real user interaction
 * through the graphical user interface.
 *
 * Phase 3 of GameMap refactoring: Remove guiHooked() tracking
 */
class GUIInteractionProvider : public ui_interfaces::IUserInteractionProvider {
  public:
   GUIInteractionProvider() = default;
   ~GUIInteractionProvider() override = default;

   int selectOption(const std::string& prompt, const std::vector<std::string>& options, int defaultOption) override;

   bool confirm(const std::string& message, bool defaultValue) override;

   std::string requestInput(const std::string& prompt, const std::string& defaultValue) override;

   void showMessage(const std::string& message) override;

   void showError(const std::string& error) override;

   bool isGuiAvailable() const override { return true; }
};

} // namespace interactions
} // namespace core
} // namespace asc

#endif // GUI_INTERACTION_PROVIDER_H
