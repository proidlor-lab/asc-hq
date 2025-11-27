/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef USER_INTERACTION_PROVIDER_H
#define USER_INTERACTION_PROVIDER_H

#include <string>
#include <vector>

namespace asc {
namespace core {
namespace ui_interfaces {

/**
 * @brief Interface for requesting user input and showing notifications
 *
 * This interface abstracts the UI concern of interacting with the user.
 * Implementations are responsible for:
 * - Requesting user decisions (select option, confirm, text input)
 * - Showing notifications (messages, errors)
 * - Handling the mechanics of presentation (GUI dialogs, console, defaults, etc.)
 *
 * The interface allows the core game logic to remain independent of
 * UI frameworks (ParaGUI, wxWidgets, etc.), enabling headless operation.
 *
 * Phase 3 of GameMap refactoring: Remove guiHooked() tracking via dependency injection
 */
class IUserInteractionProvider {
  public:
   virtual ~IUserInteractionProvider() = default;

   /**
    * @brief Request user to select one option from a list
    * @param prompt Description of what is being selected
    * @param options List of available options
    * @param defaultOption Index of default option (used in headless mode or if user cancels)
    * @return Index of selected option (0-based)
    */
   virtual int selectOption(const std::string& prompt, const std::vector<std::string>& options, int defaultOption = 0) = 0;

   /**
    * @brief Request user confirmation (yes/no decision)
    * @param message Question or statement to confirm
    * @param defaultValue Default answer (used in headless mode or if user cancels)
    * @return true if confirmed, false otherwise
    */
   virtual bool confirm(const std::string& message, bool defaultValue = true) = 0;

   /**
    * @brief Request text input from user
    * @param prompt Description of what input is needed
    * @param defaultValue Default text (used in headless mode or if user cancels)
    * @return Text entered by user, or defaultValue
    */
   virtual std::string requestInput(const std::string& prompt, const std::string& defaultValue = "") = 0;

   /**
    * @brief Show informational message to user
    * @param message Text to display
    *
    * In GUI mode: Shows dialog box
    * In headless mode: May log to file or ignore
    */
   virtual void showMessage(const std::string& message) = 0;

   /**
    * @brief Show error message to user
    * @param error Error description
    *
    * In GUI mode: Shows error dialog
    * In headless mode: Logs to stderr
    */
   virtual void showError(const std::string& error) = 0;

   /**
    * @brief Check if GUI is available for interactions
    * @return true if GUI interactions are available, false for headless mode
    *
    * This allows code to adapt behavior when GUI is not available,
    * without directly checking for GUI hooks in game logic.
    */
   virtual bool isGuiAvailable() const = 0;
};

} // namespace ui_interfaces
} // namespace core
} // namespace asc

#endif // USER_INTERACTION_PROVIDER_H
