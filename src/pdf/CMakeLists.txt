target_sources(${PROJECT_NAME} PRIVATE
    GraphicsPDFItem.cpp
    GraphicsPDFItem.h
    PDFRenderer.cpp
    PDFRenderer.h
)

if (USE_QT6PDF)
    target_sources(${PROJECT_NAME} PRIVATE
        QtPDFRenderer.cpp
        QtPDFRenderer.h
    )
endif()

if (USE_POPPLER)
    target_sources(${PROJECT_NAME} PRIVATE
        XPDFRenderer.cpp
        XPDFRenderer.h
    )
endif()
