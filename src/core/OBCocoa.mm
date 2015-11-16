#include "OBCocoa.h"
#include "UBApplicationController.h"

#include <QDebug>
#import <Cocoa/Cocoa.h>

namespace OBCocoa {

/**
 * @brief Activate the current application
 */
void setFrontProcess()
{
    NSRunningApplication* app = [NSRunningApplication currentApplication];

    // activate the application, forcing focus on it
    [app activateWithOptions: NSApplicationActivateIgnoringOtherApps];

    // other option:NSApplicationActivateAllWindows. This won't steal focus from another app, e.g
    // if the user is doing something else while waiting for OpenBoard to load
}


} // namespace OBCocoa
