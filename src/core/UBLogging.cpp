#include "UBLogging.h"
#include "UBSettings.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QThread>
#include <QLoggingCategory>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUuid>
#include <QWidget>
#include <QDebug>

// ================================
// LOGGING CATEGORIES DEFINITION
// ================================

Q_LOGGING_CATEGORY(ubUserActions, "openboard.user.actions")
Q_LOGGING_CATEGORY(ubSystemEvents, "openboard.system.events")
Q_LOGGING_CATEGORY(ubPdfOperations, "openboard.pdf.operations")
Q_LOGGING_CATEGORY(ubPerformance, "openboard.performance")
Q_LOGGING_CATEGORY(ubErrors, "openboard.errors")

// ================================
// ENUM TO STRING CONVERSIONS
// ================================

namespace UBLogActions {
    QString toolToString(Tool tool) {
        switch (tool) {
            case Tool::Pen: return "Pen";
            case Tool::Eraser: return "Eraser";
            case Tool::Marker: return "Marker";
            case Tool::Selector: return "Selector";
            case Tool::Text: return "Text";
            case Tool::Shapes: return "Shapes";
            case Tool::Compass: return "Compass";
            case Tool::Ruler: return "Ruler";
            case Tool::Protractor: return "Protractor";
            case Tool::Magnifier: return "Magnifier";
            case Tool::Hand: return "Hand";
            case Tool::Laser: return "Laser";
            case Tool::Line: return "Line";
            case Tool::ArrowedLine: return "ArrowedLine";
            case Tool::Media: return "Media";
            case Tool::Play: return "Play";
            case Tool::ZoomIn: return "ZoomIn";
            case Tool::ZoomOut: return "ZoomOut";
            case Tool::Pointer: return "Pointer";
            case Tool::Capture: return "Capture";
            case Tool::Unknown: return "Unknown";
        }
        return "Unknown";
    }

    QString systemEventToString(SystemEvent event) {
        switch (event) {
            case SystemEvent::ApplicationStarted: return "ApplicationStarted";
            case SystemEvent::ApplicationShutdown: return "ApplicationShutdown";
            case SystemEvent::DocumentCreated: return "DocumentCreated";
            case SystemEvent::DocumentOpened: return "DocumentOpened";
            case SystemEvent::DocumentSaved: return "DocumentSaved";
            case SystemEvent::DocumentClosed: return "DocumentClosed";
            case SystemEvent::SessionStarted: return "SessionStarted";
            case SystemEvent::SessionEnded: return "SessionEnded";
            case SystemEvent::ConfigurationChanged: return "ConfigurationChanged";
            case SystemEvent::PluginLoaded: return "PluginLoaded";
            case SystemEvent::PluginUnloaded: return "PluginUnloaded";
            case SystemEvent::Unknown: return "Unknown";
        }
        return "Unknown";
    }

    QString userActionToString(UserAction action) {
        switch (action) {
            case UserAction::ToolSelected: return "ToolSelected";
            case UserAction::PageNavigated: return "PageNavigated";
            case UserAction::ZoomChanged: return "ZoomChanged";
            case UserAction::DocumentExported: return "DocumentExported";
            case UserAction::ItemAdded: return "ItemAdded";
            case UserAction::ItemDeleted: return "ItemDeleted";
            case UserAction::ItemMoved: return "ItemMoved";
            case UserAction::ItemRotated: return "ItemRotated";
            case UserAction::ItemResized: return "ItemResized";
            case UserAction::BoardCleared: return "BoardCleared";
            case UserAction::UndoPerformed: return "UndoPerformed";
            case UserAction::RedoPerformed: return "RedoPerformed";
            case UserAction::Unknown: return "Unknown";
        }
        return "Unknown";
    }

    QString pdfOperationToString(PdfOperation operation) {
        switch (operation) {
            case PdfOperation::RenderingStarted: return "RenderingStarted";
            case PdfOperation::RenderingCompleted: return "RenderingCompleted";
            case PdfOperation::PageRendered: return "PageRendered";
            case PdfOperation::PageCached: return "PageCached";
            case PdfOperation::ThumbnailGenerated: return "ThumbnailGenerated";
            case PdfOperation::ExportStarted: return "ExportStarted";
            case PdfOperation::ExportCompleted: return "ExportCompleted";
            case PdfOperation::Unknown: return "Unknown";
        }
        return "Unknown";
    }

    QString performanceMetricToString(PerformanceMetric metric) {
        switch (metric) {
            case PerformanceMetric::PageRenderTime: return "PageRenderTime";
            case PerformanceMetric::DocumentLoadTime: return "DocumentLoadTime";
            case PerformanceMetric::DocumentSaveTime: return "DocumentSaveTime";
            case PerformanceMetric::MemoryUsage: return "MemoryUsage";
            case PerformanceMetric::CpuUsage: return "CpuUsage";
            case PerformanceMetric::NetworkLatency: return "NetworkLatency";
            case PerformanceMetric::DiskIo: return "DiskIo";
            case PerformanceMetric::Unknown: return "Unknown";
        }
        return "Unknown";
    }
}

// ================================
// LOG ENTRY IMPLEMENTATION
// ================================

UBLogEntry::UBLogEntry(const QLoggingCategory& category,
                       QtMsgType messageType,
                       const QString& description,
                       const QVariantMap& contextData)
    : m_description(description)
    , m_contextData(contextData)
    , m_timestamp(QDateTime::currentDateTime())
    , m_categoryName(category.categoryName())
    , m_messageType(messageType)
    , m_sessionId(generateSessionId())
{
}

QString UBLogEntry::generateSessionId()
{
    static QString sessionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    return sessionId;
}

QJsonObject UBLogEntry::toJsonObject() const
{
    QJsonObject obj;
    obj["timestamp"] = m_timestamp.toString(Qt::ISODate);
    obj["session_id"] = m_sessionId;
    obj["category"] = m_categoryName;
    obj["level"] = UBLoggingUtils::messageTypeToString(m_messageType);
    obj["description"] = m_description;
    
    if (!m_contextData.isEmpty()) {
        obj["context"] = QJsonObject::fromVariantMap(m_contextData);
    }
    
    return obj;
}

QString UBLogEntry::toJsonString() const
{
    QJsonObject obj = toJsonObject();
    QJsonDocument doc(obj);
    return doc.toJson(QJsonDocument::Compact);
}

// ================================
// LOG FILE MANAGER IMPLEMENTATION
// ================================

UBLogFileManager::UBLogFileManager(const QString& logDirectory, 
                                   int retentionDays,
                                   QObject* parent)
    : QObject(parent)
    , m_logDirectory(logDirectory)
    , m_retentionDays(retentionDays)
    , m_cleanupTimer(new QTimer(this))
{
    ensureLogDirectoryExists();
    m_currentLogFile = generateLogFileName();
    
    // Set up cleanup timer (daily cleanup)
    m_cleanupTimer->setInterval(24 * 60 * 60 * 1000); // 24 hours
    m_cleanupTimer->setSingleShot(false);
    connect(m_cleanupTimer, &QTimer::timeout, this, &UBLogFileManager::cleanupOldFiles);
    m_cleanupTimer->start();
    
    // Initial cleanup after 1 second
    QTimer::singleShot(1000, this, &UBLogFileManager::cleanupOldFiles);
}

void UBLogFileManager::writeEntry(const UBLogEntry& entry)
{
    QMutexLocker locker(&m_writeMutex);
    
    // Check if we need a new log file (e.g., daily rotation)
    if (shouldCreateNewLogFile()) {
        m_currentLogFile = generateLogFileName();
    }
    
    QString jsonString = entry.toJsonString();
    
    QFile file(m_currentLogFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream(&file);
        stream << jsonString << "\n";
        file.close();
    }
}

QStringList UBLogFileManager::getAvailableLogFiles() const
{
    QDir dir(m_logDirectory);
    QStringList filters;
    filters << "openboard_*.log";
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Time);
    QStringList result;
    
    for (const QFileInfo& fileInfo : files) {
        result << fileInfo.absoluteFilePath();
    }
    
    return result;
}

QString UBLogFileManager::exportLogsAsJson(const QDateTime& since) const
{
    QJsonArray logs;
    QStringList logFiles = getAvailableLogFiles();
    
    for (const QString& logFile : logFiles) {
        QFile file(logFile);
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream stream(&file);
            while (!stream.atEnd()) {
                QString line = stream.readLine().trimmed();
                if (!line.isEmpty()) {
                    QJsonParseError error;
                    QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &error);
                    if (error.error == QJsonParseError::NoError && doc.isObject()) {
                        QJsonObject obj = doc.object();
                        if (!since.isValid() || 
                            QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate) >= since) {
                            logs.append(obj);
                        }
                    }
                }
            }
        }
    }
    
    QJsonObject result;
    result["export_timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    result["logs"] = logs;
    result["count"] = logs.size();
    
    return QJsonDocument(result).toJson(QJsonDocument::Indented);
}

void UBLogFileManager::cleanupOldFiles()
{
    if (m_retentionDays <= 0) {
        return;
    }
    
    QDateTime cutoffDate = QDateTime::currentDateTime().addDays(-m_retentionDays);
    QStringList oldFiles = UBLoggingUtils::findLogFiles(m_logDirectory, cutoffDate);
    
    for (const QString& oldFile : oldFiles) {
        QFile::remove(oldFile);
    }
}

void UBLogFileManager::flushWrites()
{
    // File system writes are handled automatically by the OS
    // This method exists for compatibility and future enhancements
}

void UBLogFileManager::ensureLogDirectoryExists()
{
    UBLoggingUtils::ensureDirectoryExists(m_logDirectory);
}

QString UBLogFileManager::generateLogFileName()
{
    // Format: openboard_2025-06-30_10-49-34_9465c5fa.log
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    QString sessionId = QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
    return QDir(m_logDirectory).absoluteFilePath(QString("openboard_%1_%2.log").arg(timestamp, sessionId));
}

bool UBLogFileManager::shouldCreateNewLogFile() const
{
    if (m_currentLogFile.isEmpty()) {
        return true;
    }
    
    QFileInfo fileInfo(m_currentLogFile);
    if (!fileInfo.exists()) {
        return true;
    }
    
    // We use one log file per session, so never create a new file during the same session
    return false;
}

// ================================
// LOGGING STATE - INTERNAL IMPLEMENTATION
// ================================

namespace {
    // Internal state structure
    struct LoggingState {
        std::unique_ptr<UBLogFileManager> fileManager;
        QtMsgType minimumLevel = QtInfoMsg;
        QString sessionId;
        bool isInitialized = false;
        QMutex stateMutex;
    };
    
    // Global state instance
    LoggingState& getLoggingState() {
        static LoggingState state;
        return state;
    }
    
    // Helper functions
    bool shouldLog(QtMsgType type) {
        LoggingState& state = getLoggingState();
        return type >= state.minimumLevel;
    }
    
    // Internal logging function that assumes mutex is already held
    void logWithCategoryInternal(const QLoggingCategory& category,
                                 QtMsgType type,
                                 const QString& description,
                                 const QVariantMap& context = {})
    {
        LoggingState& state = getLoggingState();
        
        if (!state.isInitialized || !shouldLog(type)) {
            return;
        }
        
        UBLogEntry entry(category, type, description, context);
        
        if (state.fileManager) {
            state.fileManager->writeEntry(entry);
        }
    }
    
    // Public logging function that acquires mutex
    void logWithCategory(const QLoggingCategory& category,
                        QtMsgType type,
                        const QString& description,
                        const QVariantMap& context = {})
    {
        LoggingState& state = getLoggingState();
        QMutexLocker locker(&state.stateMutex);
        
        logWithCategoryInternal(category, type, description, context);
    }
    
    QString getDefaultLogDirectory() {
        // Use OpenBoard's userDataDirectory() which resolves to ~/.local/share/OpenBoard/
        QString dataDir = UBSettings::userDataDirectory();
        return QDir(dataDir).absoluteFilePath("logs");
    }
    
    void handleQtLogMessage(QtMsgType type, const QMessageLogContext& context, const QString& message) {
        // Protect against infinite recursion
        static thread_local bool inLoggingHandler = false;
        if (inLoggingHandler) {
            return;
        }
        
        if (!shouldLog(type)) {
            return;
        }
        
        inLoggingHandler = true;
        
        QVariantMap contextData;
        contextData["file"] = context.file ? context.file : "";
        contextData["function"] = context.function ? context.function : "";
        contextData["line"] = context.line;
        contextData["qt_category"] = context.category ? context.category : "";
        contextData["source"] = "qt_logging_system";
        
        // Enhanced category detection based on context
        const QLoggingCategory* category = &ubSystemEvents(); // Default
        
        if (context.category) {
            QString categoryName = QString(context.category).toLower();
            
            // Detect OpenBoard specific categories
            if (categoryName.contains("openboard.user") || categoryName.contains("user")) {
                category = &ubUserActions();
            }
            else if (categoryName.contains("openboard.pdf") || categoryName.contains("pdf")) {
                category = &ubPdfOperations();
            }
            else if (categoryName.contains("openboard.performance") || categoryName.contains("performance")) {
                category = &ubPerformance();
            }
            else if (categoryName.contains("openboard.errors") || categoryName.contains("openboard.error") || 
                     categoryName.contains("error") || categoryName.contains("warning") || 
                     type >= QtWarningMsg) {
                category = &ubErrors();
            }
        }
        else {
            // If no category is specified, use message type to determine category
            if (type >= QtCriticalMsg) {
                category = &ubErrors();
            }
            else if (context.file) {
                QString filePath = QString(context.file).toLower();
                
                // Detect based on file path
                if (filePath.contains("pdf") || filePath.contains("document")) {
                    category = &ubPdfOperations();
                }
                else if (filePath.contains("board") || filePath.contains("tool") || 
                         filePath.contains("gui") || filePath.contains("widget")) {
                    category = &ubUserActions();
                }
                else if (filePath.contains("performance") || filePath.contains("timer")) {
                    category = &ubPerformance();
                }
            }
        }
        
        // Add Qt-specific context information
        contextData["message_type"] = UBLoggingUtils::messageTypeToString(type);
        contextData["thread_id"] = QString::number(reinterpret_cast<qintptr>(QThread::currentThread()));
        
        logWithCategory(*category, type, message, contextData);
        
        inLoggingHandler = false;
    }
} // anonymous namespace

// ================================
// UTILITY HELPERS IMPLEMENTATION
// ================================

QVariantMap UBLoggingUtils::rectToMap(const QRectF& rect)
{
    QVariantMap map;
    map["x"] = rect.x();
    map["y"] = rect.y();
    map["width"] = rect.width();
    map["height"] = rect.height();
    return map;
}

QVariantMap UBLoggingUtils::sizeToMap(const QSizeF& size)
{
    QVariantMap map;
    map["width"] = size.width();
    map["height"] = size.height();
    return map;
}

QVariantMap UBLoggingUtils::pointToMap(const QPointF& point)
{
    QVariantMap map;
    map["x"] = point.x();
    map["y"] = point.y();
    return map;
}

QVariantMap UBLoggingUtils::widgetContext(const QWidget* widget)
{
    QVariantMap context;
    if (widget) {
        context["class_name"] = widget->metaObject()->className();
        context["object_name"] = widget->objectName();
        context["geometry"] = rectToMap(widget->geometry());
        context["visible"] = widget->isVisible();
        context["enabled"] = widget->isEnabled();
    }
    return context;
}

QVariantMap UBLoggingUtils::applicationContext()
{
    QVariantMap context;
    context["app_name"] = QCoreApplication::applicationName();
    context["app_version"] = QCoreApplication::applicationVersion();
    context["qt_version"] = QT_VERSION_STR;
    context["thread_id"] = QString::number(reinterpret_cast<qintptr>(QThread::currentThread()));
    return context;
}

bool UBLoggingUtils::ensureDirectoryExists(const QString& path)
{
    QDir dir;
    return dir.mkpath(path);
}

QStringList UBLoggingUtils::findLogFiles(const QString& directory, const QDateTime& olderThan)
{
    QDir dir(directory);
    QStringList filters;
    filters << "openboard_*.log";
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Time);
    QStringList result;
    
    for (const QFileInfo& fileInfo : files) {
        if (!olderThan.isValid() || fileInfo.lastModified() < olderThan) {
            result << fileInfo.absoluteFilePath();
        }
    }
    
    return result;
}

QString UBLoggingUtils::messageTypeToString(QtMsgType type)
{
    switch (type) {
        case QtDebugMsg: return "DEBUG";
        case QtInfoMsg: return "INFO";
        case QtWarningMsg: return "WARNING";
        case QtCriticalMsg: return "ERROR";
        case QtFatalMsg: return "FATAL";
        default: return "UNKNOWN";
    }
}

QString UBLoggingUtils::formatContextData(const QVariantMap& context)
{
    if (context.isEmpty()) {
        return QString();
    }
    
    return QJsonDocument::fromVariant(context).toJson(QJsonDocument::Compact);
}

// ================================
// NAMESPACE API IMPLEMENTATION
// ================================

namespace UBLogManager {

    void initialize(const QString& logDirectory, QtMsgType minimumLevel) {
        LoggingState& state = getLoggingState();
        QMutexLocker locker(&state.stateMutex);
        
        if (state.isInitialized) {
            return;
        }
        
        QString logDir = logDirectory.isEmpty() ? getDefaultLogDirectory() : logDirectory;
        
        state.fileManager = std::make_unique<UBLogFileManager>(logDir);
        state.minimumLevel = minimumLevel;
        state.sessionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        
        // Install Qt message handler
        qInstallMessageHandler(handleQtLogMessage);
        
        state.isInitialized = true;
        
        // Log system startup
        QVariantMap context;
        context["version"] = QCoreApplication::applicationVersion();
        context["sessionId"] = state.sessionId;
        
        logWithCategoryInternal(ubSystemEvents(), QtInfoMsg, 
                               "Logging system started", context);
    }

    void shutdown() {
        LoggingState& state = getLoggingState();
        QMutexLocker locker(&state.stateMutex);
        
        if (!state.isInitialized) {
            return;
        }
        
        logWithCategoryInternal(ubSystemEvents(), QtInfoMsg, "Logging system shutdown", {});
        
        if (state.fileManager) {
            state.fileManager->flushWrites();
            state.fileManager.reset();
        }
        
        qInstallMessageHandler(nullptr);
        state.isInitialized = false;
    }

    // User Actions - Natural Language API
    void userSelectedTool(UBLogActions::Tool tool, const QVariantMap& context) {
        QVariantMap enrichedContext = context;
        enrichedContext["tool_name"] = UBLogActions::toolToString(tool);
        enrichedContext["tool_type"] = UBLogActions::toolToString(tool);
        enrichedContext["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubUserActions(), QtInfoMsg, 
                       QString("Tool selected: %1").arg(UBLogActions::toolToString(tool)), 
                       enrichedContext);
    }

    void userNavigatedToPage(int currentPage, int totalPages) {
        QVariantMap context;
        context["current_page"] = currentPage;
        context["total_pages"] = totalPages;
        context["action_type"] = UBLogActions::userActionToString(UBLogActions::UserAction::PageNavigated);
        context["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubUserActions(), QtInfoMsg, 
                       QString("Page navigated: %1/%2").arg(currentPage).arg(totalPages), 
                       context);
    }

    void userPerformedZoom(qreal newZoom, const QPointF& scenePoint) {
        QVariantMap context;
        context["new_zoom"] = newZoom;
        context["scene_point_x"] = scenePoint.x();
        context["scene_point_y"] = scenePoint.y();
        context["action_type"] = UBLogActions::userActionToString(UBLogActions::UserAction::ZoomChanged);
        context["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubUserActions(), QtInfoMsg, 
                       QString("Zoom changed to: %1 at scene point (%2, %3)")
                           .arg(newZoom)
                           .arg(scenePoint.x())
                           .arg(scenePoint.y()), 
                       context);
    }

    void userAddedItem(const QString& itemType, const QVariantMap& properties) {
        QVariantMap context = properties;
        context["item_type"] = itemType;
        context["action_type"] = UBLogActions::userActionToString(UBLogActions::UserAction::ItemAdded);
        context["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubUserActions(), QtInfoMsg, 
                       QString("Item added: %1").arg(itemType), 
                       context);
    }

    void userDeletedItem(const QString& itemType, const QVariantMap& properties) {
        QVariantMap context = properties;
        context["item_type"] = itemType;
        context["action_type"] = UBLogActions::userActionToString(UBLogActions::UserAction::ItemDeleted);
        context["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubUserActions(), QtInfoMsg, 
                       QString("Item deleted: %1").arg(itemType), 
                       context);
    }

    void userMovedItem(const QString& itemType, const QPointF& from, const QPointF& to) {
        QVariantMap context;
        context["item_type"] = itemType;
        context["from_x"] = from.x();
        context["from_y"] = from.y();
        context["to_x"] = to.x();
        context["to_y"] = to.y();
        context["distance"] = QLineF(from, to).length();
        context["action_type"] = UBLogActions::userActionToString(UBLogActions::UserAction::ItemMoved);
        context["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubUserActions(), QtInfoMsg, 
                       QString("Item moved: %1").arg(itemType), 
                       context);
    }

    void userPerformedUndo() {
        QVariantMap context;
        context["action_type"] = UBLogActions::userActionToString(UBLogActions::UserAction::UndoPerformed);
        context["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubUserActions(), QtInfoMsg, "User performed undo", context);
    }

    void userPerformedRedo() {
        QVariantMap context;
        context["action_type"] = UBLogActions::userActionToString(UBLogActions::UserAction::RedoPerformed);
        context["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubUserActions(), QtInfoMsg, "User performed redo", context);
    }

    void userClearedBoard() {
        QVariantMap context;
        context["action_type"] = UBLogActions::userActionToString(UBLogActions::UserAction::BoardCleared);
        context["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubUserActions(), QtInfoMsg, "User cleared board", context);
    }

    // Document Operations
    void documentCreated(const QString& path) {
        QVariantMap context;
        if (!path.isEmpty()) {
            context["document_path"] = path;
            context["document_name"] = QFileInfo(path).baseName();
        }
        context["event_type"] = UBLogActions::systemEventToString(UBLogActions::SystemEvent::DocumentCreated);
        context["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubSystemEvents(), QtInfoMsg, 
                       QString("Document created: %1").arg(path.isEmpty() ? "New document" : QFileInfo(path).baseName()), 
                       context);
    }

    void documentOpened(const QString& path) {
        QVariantMap context;
        context["document_path"] = path;
        context["document_name"] = QFileInfo(path).baseName();
        context["document_size"] = QFileInfo(path).size();
        context["event_type"] = UBLogActions::systemEventToString(UBLogActions::SystemEvent::DocumentOpened);
        context["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubSystemEvents(), QtInfoMsg, 
                       QString("Document opened: %1").arg(QFileInfo(path).baseName()), 
                       context);
    }

    void documentSaved(const QString& path) {
        QVariantMap context;
        context["document_path"] = path;
        context["document_name"] = QFileInfo(path).baseName();
        context["event_type"] = UBLogActions::systemEventToString(UBLogActions::SystemEvent::DocumentSaved);
        context["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubSystemEvents(), QtInfoMsg, 
                       QString("Document saved: %1").arg(QFileInfo(path).baseName()), 
                       context);
    }

    void documentExported(const QString& format, const QString& path) {
        QVariantMap context;
        context["export_format"] = format;
        context["export_path"] = path;
        context["export_name"] = QFileInfo(path).baseName();
        context["action_type"] = UBLogActions::userActionToString(UBLogActions::UserAction::DocumentExported);
        context["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubUserActions(), QtInfoMsg, 
                       QString("Document exported: %1 to %2").arg(QFileInfo(path).baseName(), format), 
                       context);
    }

    void documentClosed(const QString& path) {
        QVariantMap context;
        context["document_path"] = path;
        context["document_name"] = QFileInfo(path).baseName();
        context["event_type"] = UBLogActions::systemEventToString(UBLogActions::SystemEvent::DocumentClosed);
        context["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubSystemEvents(), QtInfoMsg, 
                       QString("Document closed: %1").arg(QFileInfo(path).baseName()), 
                       context);
    }

    // System Events
    void applicationStarted(const QString& version) {
        QVariantMap context;
        context["version"] = version;
        context["qt_version"] = QT_VERSION_STR;
        context["startup_time"] = QDateTime::currentDateTime().toSecsSinceEpoch();
        context["event_type"] = UBLogActions::systemEventToString(UBLogActions::SystemEvent::ApplicationStarted);
        context["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubSystemEvents(), QtInfoMsg, 
                       QString("Application started: v%1").arg(version), 
                       context);
    }

    void applicationShutdown() {
        QVariantMap context;
        context["shutdown_time"] = QDateTime::currentDateTime().toSecsSinceEpoch();
        context["event_type"] = UBLogActions::systemEventToString(UBLogActions::SystemEvent::ApplicationShutdown);
        context["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubSystemEvents(), QtInfoMsg, "Application shutdown", context);
    }

    void configurationChanged(const QString& setting, const QVariant& oldValue, const QVariant& newValue) {
        QVariantMap context;
        context["setting_name"] = setting;
        context["old_value"] = oldValue;
        context["new_value"] = newValue;
        context["event_type"] = UBLogActions::systemEventToString(UBLogActions::SystemEvent::ConfigurationChanged);
        context["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubSystemEvents(), QtInfoMsg, 
                       QString("Configuration changed: %1").arg(setting), 
                       context);
    }

    void pluginLoaded(const QString& pluginName, const QString& version) {
        QVariantMap context;
        context["plugin_name"] = pluginName;
        if (!version.isEmpty()) {
            context["plugin_version"] = version;
        }
        context["event_type"] = UBLogActions::systemEventToString(UBLogActions::SystemEvent::PluginLoaded);
        context["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubSystemEvents(), QtInfoMsg, 
                       QString("Plugin loaded: %1").arg(pluginName), 
                       context);
    }

    void pluginUnloaded(const QString& pluginName) {
        QVariantMap context;
        context["plugin_name"] = pluginName;
        context["event_type"] = UBLogActions::systemEventToString(UBLogActions::SystemEvent::PluginUnloaded);
        context["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubSystemEvents(), QtInfoMsg, 
                       QString("Plugin unloaded: %1").arg(pluginName), 
                       context);
    }

    // PDF Operations
    void pdfRenderingStarted(const QString& documentPath, int totalPages) {
        QVariantMap context;
        context["document_path"] = documentPath;
        context["total_pages"] = totalPages;
        context["document_name"] = QFileInfo(documentPath).baseName();
        context["operation_type"] = UBLogActions::pdfOperationToString(UBLogActions::PdfOperation::RenderingStarted);
        context["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubPdfOperations(), QtInfoMsg, 
                       QString("PDF rendering started: %1 (%2 pages)").arg(QFileInfo(documentPath).baseName()).arg(totalPages), 
                       context);
    }

    void pdfRenderingCompleted(int pagesRendered, qint64 renderingTimeMs) {
        QVariantMap context;
        context["pages_rendered"] = pagesRendered;
        context["rendering_time_ms"] = renderingTimeMs;
        context["avg_time_per_page"] = pagesRendered > 0 ? renderingTimeMs / pagesRendered : 0;
        context["operation_type"] = UBLogActions::pdfOperationToString(UBLogActions::PdfOperation::RenderingCompleted);
        context["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubPdfOperations(), QtInfoMsg, 
                       QString("PDF rendering completed: %1 pages in %2ms").arg(pagesRendered).arg(renderingTimeMs), 
                       context);
    }

    void pdfPageRendered(int pageNumber, qint64 renderingTimeMs) {
        QVariantMap context;
        context["page_number"] = pageNumber;
        context["rendering_time_ms"] = renderingTimeMs;
        context["operation_type"] = UBLogActions::pdfOperationToString(UBLogActions::PdfOperation::PageRendered);
        context["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubPdfOperations(), QtInfoMsg, 
                       QString("PDF page rendered: page %1 in %2ms").arg(pageNumber).arg(renderingTimeMs), 
                       context);
    }

    void pdfThumbnailGenerated(int pageNumber, const QSize& size, qint64 generationTimeMs) {
        QVariantMap context;
        context["page_number"] = pageNumber;
        context["thumbnail_width"] = size.width();
        context["thumbnail_height"] = size.height();
        context["generation_time_ms"] = generationTimeMs;
        context["operation_type"] = UBLogActions::pdfOperationToString(UBLogActions::PdfOperation::ThumbnailGenerated);
        context["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubPdfOperations(), QtInfoMsg, 
                       QString("PDF thumbnail generated: page %1 (%2x%3) in %4ms").arg(pageNumber).arg(size.width()).arg(size.height()).arg(generationTimeMs), 
                       context);
    }

    // Performance Metrics
    void recordPerformanceMetric(UBLogActions::PerformanceMetric metric, qint64 value, const QVariantMap& context) {
        QVariantMap enrichedContext = context;
        enrichedContext["metric_type"] = UBLogActions::performanceMetricToString(metric);
        enrichedContext["metric_value"] = value;
        enrichedContext["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubPerformance(), QtInfoMsg, 
                       QString("Performance metric: %1 = %2").arg(UBLogActions::performanceMetricToString(metric)).arg(value), 
                       enrichedContext);
    }

    void recordMemoryUsage(qint64 usedMemoryBytes, qint64 totalMemoryBytes) {
        QVariantMap context;
        context["used_memory_bytes"] = usedMemoryBytes;
        context["total_memory_bytes"] = totalMemoryBytes;
        context["memory_usage_percentage"] = totalMemoryBytes > 0 ? (usedMemoryBytes * 100.0 / totalMemoryBytes) : 0;
        
        recordPerformanceMetric(UBLogActions::PerformanceMetric::MemoryUsage, usedMemoryBytes, context);
    }

    void recordPageRenderTime(int pageNumber, qint64 renderTimeMs) {
        QVariantMap context;
        context["page_number"] = pageNumber;
        
        recordPerformanceMetric(UBLogActions::PerformanceMetric::PageRenderTime, renderTimeMs, context);
    }

    void recordDocumentLoadTime(const QString& documentPath, qint64 loadTimeMs) {
        QVariantMap context;
        context["document_path"] = documentPath;
        context["document_name"] = QFileInfo(documentPath).baseName();
        
        recordPerformanceMetric(UBLogActions::PerformanceMetric::DocumentLoadTime, loadTimeMs, context);
    }

    // Error and Diagnostic Logging
    void logError(const QString& error, const QVariantMap& context) {
        QVariantMap enrichedContext = context;
        enrichedContext["error_message"] = error;
        enrichedContext["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubErrors(), QtCriticalMsg, error, enrichedContext);
    }

    void logWarning(const QString& warning, const QVariantMap& context) {
        QVariantMap enrichedContext = context;
        enrichedContext["warning_message"] = warning;
        enrichedContext["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubErrors(), QtWarningMsg, warning, enrichedContext);
    }

    void logInfo(const QString& info, const QVariantMap& context) {
        QVariantMap enrichedContext = context;
        enrichedContext["info_message"] = info;
        enrichedContext["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        logWithCategory(ubSystemEvents(), QtInfoMsg, info, enrichedContext);
    }

    // Configuration and Export
    void setMinimumLogLevel(QtMsgType level) {
        LoggingState& state = getLoggingState();
        QMutexLocker locker(&state.stateMutex);
        state.minimumLevel = level;
    }

    void enableLoggingCategory(QLoggingCategory& category, bool enabled) {
        category.setEnabled(QtDebugMsg, enabled);
        category.setEnabled(QtInfoMsg, enabled);
        category.setEnabled(QtWarningMsg, enabled);
        category.setEnabled(QtCriticalMsg, enabled);
    }

    QString exportRecentLogs(int daysBack) {
        LoggingState& state = getLoggingState();
        QMutexLocker locker(&state.stateMutex);
        
        if (!state.fileManager) {
            return QString();
        }
        
        QDateTime since = QDateTime::currentDateTime().addDays(-daysBack);
        return state.fileManager->exportLogsAsJson(since);
    }

    void flushAllLogs() {
        LoggingState& state = getLoggingState();
        QMutexLocker locker(&state.stateMutex);
        
        if (state.fileManager) {
            state.fileManager->flushWrites();
        }
    }

    // Status
    bool isInitialized() {
        LoggingState& state = getLoggingState();
        QMutexLocker locker(&state.stateMutex);
        return state.isInitialized;
    }

    QString getCurrentSessionId() {
        LoggingState& state = getLoggingState();
        QMutexLocker locker(&state.stateMutex);
        return state.sessionId;
    }

} // namespace UBLogManager
