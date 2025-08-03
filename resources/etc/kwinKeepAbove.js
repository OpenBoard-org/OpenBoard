/*
 * Set keepAbove for OpenBoard DesktopView
 *
 * Works with KDE 5 and KDE 6
 */

function keepOnTop() {
    // KDE 6 property
    var allClients = workspace.stackingOrder;

    if (!allClients) {
        // KDE 5 function
        allClients = workspace.clientList();
    }
    for (var i = 0; i < allClients.length; ++i) {
        var client = allClients[i];
        if (client.resourceClass == "org.oe-f.openboard" && client.caption == "DesktopView") {
            client.keepAbove=true;
        }
    }
}

keepOnTop();
