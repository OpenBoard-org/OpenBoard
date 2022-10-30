target_sources(${PROJECT_NAME} PRIVATE
    UBEmbedContent.cpp
    UBEmbedContent.h
    UBEmbedController.cpp
    UBEmbedController.h
    UBEmbedParser.cpp
    UBEmbedParser.h
    UBWebController.cpp
    UBWebController.h
    simplebrowser/WBHistory.cpp
    simplebrowser/WBHistory.h
    simplebrowser/WBModelMenu.cpp
    simplebrowser/WBModelMenu.h
    simplebrowser/browserwindow.cpp
    simplebrowser/browserwindow.h
    simplebrowser/downloadmanagerwidget.cpp
    simplebrowser/downloadmanagerwidget.h
    simplebrowser/downloadwidget.cpp
    simplebrowser/downloadwidget.h
    simplebrowser/tabwidget.cpp
    simplebrowser/tabwidget.h
    simplebrowser/webpage.cpp
    simplebrowser/webpage.h
    simplebrowser/webpopupwindow.cpp
    simplebrowser/webpopupwindow.h
    simplebrowser/webview.cpp
    simplebrowser/webview.h
)

target_sources(${PROJECT_NAME} PRIVATE
    simplebrowser/certificateerrordialog.ui
    simplebrowser/downloadmanagerwidget.ui
    simplebrowser/downloadwidget.ui
    simplebrowser/passworddialog.ui
)
