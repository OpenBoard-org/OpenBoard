/*
 * OPENBOARD QT-BASED LOGGING SYSTEM
 * 
 * This file defines the new Qt-native logging system for OpenBoard.
 * It leverages Qt 6.7.2's built-in logging infrastructure while providing
 * a namespace-based API that avoids singletons and uses type-safe enums.
 * 
 * Key Features:
 * - Namespace-based API (no singletons in public interface)
 * - Type-safe enums for all log actions
 * - Asynchronous JSON logging
 * - Per-session log files
 * - Automatic log retention
 * - Runtime filtering via Qt logging categories
 * - Thread-safe operations
 * - Natural language API
 */

#ifndef UB_LOGGING_H
#define UB_LOGGING_H

#include <QObject>
#include <QLoggingCategory>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QVariantMap>
#include <QTimer>
#include <QTextStream>
#include <QMutex>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QPointF>
#include <memory>

// ================================
// LOGGING CATEGORIES
// ================================

Q_DECLARE_LOGGING_CATEGORY(ubUserActions)
Q_DECLARE_LOGGING_CATEGORY(ubSystemEvents)
Q_DECLARE_LOGGING_CATEGORY(ubPdfOperations)
Q_DECLARE_LOGGING_CATEGORY(ubPerformance)
Q_DECLARE_LOGGING_CATEGORY(ubErrors)

// ================================
// TYPE-SAFE ACTION ENUMS
// ================================

namespace UBLogActions {
    enum class Tool {
        Pen,
        Eraser,
        Marker,
        Selector,
        Text,
        Shapes,
        Compass,
        Ruler,
        Protractor,
        Magnifier,
        Hand,
        Laser,
        Line,
        ArrowedLine,
        Media,
        Play,        // OpenBoard specific
        ZoomIn,      // OpenBoard specific
        ZoomOut,     // OpenBoard specific
        Pointer,     // OpenBoard specific
        Capture,     // OpenBoard specific
        Unknown
    };

    enum class SystemEvent {
        ApplicationStarted,
        ApplicationShutdown,
        DocumentCreated,
        DocumentOpened,
        DocumentSaved,
        DocumentClosed,
        SessionStarted,
        SessionEnded,
        ConfigurationChanged,
        PluginLoaded,
        PluginUnloaded,
        Unknown
    };

    enum class UserAction {
        ToolSelected,
        PageNavigated,
        ZoomChanged,
        DocumentExported,
        ItemAdded,
        ItemDeleted,
        ItemMoved,
        ItemRotated,
        ItemResized,
        BoardCleared,
        UndoPerformed,
        RedoPerformed,
        Unknown
    };

    enum class PdfOperation {
        RenderingStarted,
        RenderingCompleted,
        PageRendered,
        PageCached,
        ThumbnailGenerated,
        ExportStarted,
        ExportCompleted,
        Unknown
    };

    enum class PerformanceMetric {
        PageRenderTime,
        DocumentLoadTime,
        DocumentSaveTime,
        MemoryUsage,
        CpuUsage,
        NetworkLatency,
        DiskIo,
        Unknown
    };

    // Utility functions to convert enums to readable strings
    QString toolToString(Tool tool);
    QString systemEventToString(SystemEvent event);
    QString userActionToString(UserAction action);
    QString pdfOperationToString(PdfOperation operation);
    QString performanceMetricToString(PerformanceMetric metric);
}

// ================================
// LOG ENTRY
// ================================

class UBLogEntry
{
public:
    explicit UBLogEntry(const QLoggingCategory& category,
                       QtMsgType messageType,
                       const QString& description,
                       const QVariantMap& contextData = {});

    // Accessors
    QString getDescription() const { return m_description; }
    QVariantMap getContextData() const { return m_contextData; }
    QDateTime getTimestamp() const { return m_timestamp; }
    QString getCategoryName() const { return m_categoryName; }
    QtMsgType getMessageType() const { return m_messageType; }
    QString getSessionId() const { return m_sessionId; }
    
    // Serialization
    QJsonObject toJsonObject() const;
    QString toJsonString() const;

private:
    QString m_description;
    QVariantMap m_contextData;
    QDateTime m_timestamp;
    QString m_categoryName;
    QtMsgType m_messageType;
    QString m_sessionId;
    
    static QString generateSessionId();
};

// ================================
// LOG FILE MANAGER
// ================================

class UBLogFileManager : public QObject
{
    Q_OBJECT

public:
    explicit UBLogFileManager(const QString& logDirectory, 
                             int retentionDays = 7,
                             QObject* parent = nullptr);

    void writeEntry(const UBLogEntry& entry);
    QString getCurrentLogFile() const { return m_currentLogFile; }
    QStringList getAvailableLogFiles() const;
    QString exportLogsAsJson(const QDateTime& since = QDateTime()) const;

public slots:
    void cleanupOldFiles();
    void flushWrites();

private:
    QString m_logDirectory;
    QString m_currentLogFile;
    int m_retentionDays;
    QTimer* m_cleanupTimer;
    mutable QMutex m_writeMutex;
    
    void ensureLogDirectoryExists();
    QString generateLogFileName();
    bool shouldCreateNewLogFile() const;
};

// ================================
// INTERNAL LOGGING IMPLEMENTATION (moved to UBLogManager namespace)
// ================================

// ================================
// PUBLIC NAMESPACE API
// ================================

namespace UBLogManager {
    // Initialization
    void initialize(const QString& logDirectory = QString(), QtMsgType minimumLevel = QtInfoMsg);
    void shutdown();
    
    // User Actions - Natural Language API
    void userSelectedTool(UBLogActions::Tool tool, const QVariantMap& context = {});
    void userNavigatedToPage(int currentPage, int totalPages);
    void userPerformedZoom(qreal newZoom, const QPointF& scenePoint = QPointF(0, 0));
    void userAddedItem(const QString& itemType, const QVariantMap& properties = {});
    void userDeletedItem(const QString& itemType, const QVariantMap& properties = {});
    void userMovedItem(const QString& itemType, const QPointF& from, const QPointF& to);
    void userPerformedUndo();
    void userPerformedRedo();
    void userClearedBoard();
    
    // Document Operations
    void documentCreated(const QString& path = {});
    void documentOpened(const QString& path);
    void documentSaved(const QString& path);
    void documentExported(const QString& format, const QString& path);
    void documentClosed(const QString& path);
    
    // System Events
    void applicationStarted(const QString& version);
    void applicationShutdown();
    void configurationChanged(const QString& setting, const QVariant& oldValue, const QVariant& newValue);
    void pluginLoaded(const QString& pluginName, const QString& version = {});
    void pluginUnloaded(const QString& pluginName);
    
    // PDF Operations
    void pdfRenderingStarted(const QString& documentPath, int totalPages);
    void pdfRenderingCompleted(int pagesRendered, qint64 renderingTimeMs);
    void pdfPageRendered(int pageNumber, qint64 renderingTimeMs);
    void pdfThumbnailGenerated(int pageNumber, const QSize& size, qint64 generationTimeMs);
    
    // Performance Metrics
    void recordPerformanceMetric(UBLogActions::PerformanceMetric metric, qint64 value, const QVariantMap& context = {});
    void recordMemoryUsage(qint64 usedMemoryBytes, qint64 totalMemoryBytes);
    void recordPageRenderTime(int pageNumber, qint64 renderTimeMs);
    void recordDocumentLoadTime(const QString& documentPath, qint64 loadTimeMs);
    
    // Error and Diagnostic Logging
    void logError(const QString& error, const QVariantMap& context = {});
    void logWarning(const QString& warning, const QVariantMap& context = {});
    void logInfo(const QString& info, const QVariantMap& context = {});
    
    // Configuration and Export
    void setMinimumLogLevel(QtMsgType level);
    void enableLoggingCategory(QLoggingCategory& category, bool enabled = true);
    QString exportRecentLogs(int daysBack = 7);
    void flushAllLogs();
    
    // Status
    bool isInitialized();
    QString getCurrentSessionId();
}

// ================================
// UTILITY HELPERS
// ================================

class UBLoggingUtils
{
public:
    // Qt type conversion helpers
    static QVariantMap rectToMap(const QRectF& rect);
    static QVariantMap sizeToMap(const QSizeF& size);  
    static QVariantMap pointToMap(const QPointF& point);
    
    // Widget context extraction
    static QVariantMap widgetContext(const QWidget* widget);
    static QVariantMap applicationContext();
    
    // File system helpers
    static bool ensureDirectoryExists(const QString& path);
    static QStringList findLogFiles(const QString& directory, const QDateTime& olderThan = QDateTime());
    
    // Message type helpers
    static QString messageTypeToString(QtMsgType type);
    static QString formatContextData(const QVariantMap& context);
};

#endif // UB_LOGGING_H
