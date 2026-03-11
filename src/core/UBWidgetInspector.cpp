#include "UBWidgetInspector.h"

#include <QApplication>
#include <QCoreApplication>
#include <QCursor>
#include <QDialog>
#include <QEvent>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QScreen>
#include <QStyle>
#include <QTextCursor>
#include <QVBoxLayout>
#include <QWidget>

UBWidgetInspector::UBWidgetInspector(QObject *parent)
    : QObject(parent)
    , mInspectorDialog(nullptr)
    , mInspectorView(nullptr)
{
}

bool UBWidgetInspector::eventFilter(QObject *obj, QEvent *event)
{
    if (!event || QCoreApplication::closingDown())
        return QObject::eventFilter(obj, event);

    switch (event->type()) {
    case QEvent::KeyPress: {
        const auto *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() != Qt::Key_F12 || keyEvent->isAutoRepeat())
            return QObject::eventFilter(obj, event);

        inspectWidgetUnderCursor();
        return true;
    }
    default:
        return QObject::eventFilter(obj, event);
    }
}

void UBWidgetInspector::inspectWidgetUnderCursor()
{
    QWidget *widget = QApplication::widgetAt(QCursor::pos());
    showInspector(buildInfo(widget));
}

QString UBWidgetInspector::buildInfo(const QWidget *widget) const
{
    if (!widget) {
        return QStringLiteral("No widget under cursor.");
    }

    const QString name = widget->objectName().isEmpty()
        ? QStringLiteral("<none>")
        : widget->objectName();
    const QString styleName =
        (widget->style() && widget->style()->metaObject())
            ? QString::fromUtf8(widget->style()->metaObject()->className())
            : QStringLiteral("<none>");
    const QList<const QWidget*> hierarchy = buildWidgetHierarchy(widget);
    const QStringList selectorTokens = buildSelectorTokens(hierarchy);
    const QList<StyleSheetSource> sources = buildStyleSheetSources(hierarchy);

    QStringList reportSections;
    reportSections << QStringLiteral("Target\n%1 obj=\"%2\"\nstyle=\"%3\"")
        .arg(QString::fromUtf8(widget->metaObject()->className()),
             name,
             styleName);
    reportSections << buildHierarchySection(hierarchy);
    reportSections << QStringLiteral("Precedence\n1. widgetQss on the target widget\n2. parentQss from the nearest parent outward\n3. appQss from QApplication\nWithin one source, more specific selectors win; ties are resolved by later rules.");
    reportSections << buildRelevantStylesSection(sources, selectorTokens);

    return reportSections.join(QStringLiteral("\n\n"));
}

void UBWidgetInspector::showInspector(const QString &info)
{
    ensureInspectorDialog();

    mInspectorView->setPlainText(info);
    mInspectorView->moveCursor(QTextCursor::Start);

    if (!mInspectorDialog->isVisible())
        mInspectorDialog->show();

    positionInspector();
    mInspectorDialog->raise();
    mInspectorDialog->activateWindow();
}

void UBWidgetInspector::ensureInspectorDialog()
{
    if (mInspectorDialog && mInspectorView)
        return;

    auto *dialog = new QDialog();
    dialog->setWindowTitle(QStringLiteral("Widget Inspector"));
    dialog->setWindowFlag(Qt::Tool, true);
    dialog->resize(760, 520);

    auto *layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(8, 8, 8, 8);

    auto *view = new QPlainTextEdit(dialog);
    view->setReadOnly(true);
    view->setLineWrapMode(QPlainTextEdit::NoWrap);
    layout->addWidget(view);

    mInspectorDialog = dialog;
    mInspectorView = view;

    connect(dialog, &QObject::destroyed, this, [this]() {
        mInspectorDialog.clear();
        mInspectorView.clear();
    });
}

void UBWidgetInspector::positionInspector()
{
    if (!mInspectorDialog)
        return;

    QPoint position = QCursor::pos() + QPoint(24, 24);
    if (QScreen *screen = QGuiApplication::screenAt(QCursor::pos())) {
        const QRect availableGeometry = screen->availableGeometry();
        const QSize dialogSize = mInspectorDialog->size();

        if (position.x() + dialogSize.width() > availableGeometry.right())
            position.rx() = qMax(availableGeometry.left(), availableGeometry.right() - dialogSize.width() + 1);

        if (position.y() + dialogSize.height() > availableGeometry.bottom())
            position.ry() = qMax(availableGeometry.top(), availableGeometry.bottom() - dialogSize.height() + 1);
    }

    mInspectorDialog->move(position);
}

QString UBWidgetInspector::describeWidget(const QWidget *widget)
{
    if (!widget)
        return QStringLiteral("<null>");

    const QString objectName = widget->objectName().isEmpty()
        ? QStringLiteral("<none>")
        : widget->objectName();
    return QStringLiteral("%1 obj=\"%2\"")
        .arg(QString::fromUtf8(widget->metaObject()->className()), objectName);
}

QList<const QWidget*> UBWidgetInspector::buildWidgetHierarchy(const QWidget *widget)
{
    QList<const QWidget*> hierarchy;
    for (const QWidget *current = widget; current; current = current->parentWidget())
        hierarchy << current;
    return hierarchy;
}

QString UBWidgetInspector::buildHierarchySection(const QList<const QWidget*> &hierarchy)
{
    QStringList lines;
    lines << QStringLiteral("Hierarchy");

    for (int index = 0; index < hierarchy.size(); ++index) {
        const QWidget *widget = hierarchy.at(index);
        const QString prefix = (index == 0) ? QStringLiteral("0. target") : QStringLiteral("%1. parent").arg(index);
        lines << QStringLiteral("%1 %2").arg(prefix, describeWidget(widget));
    }

    return lines.join(QStringLiteral("\n"));
}

QStringList UBWidgetInspector::buildSelectorTokens(const QList<const QWidget*> &hierarchy)
{
    QStringList tokens;

    for (const QWidget *widget : hierarchy) {
        if (!widget)
            continue;

        if (!widget->objectName().isEmpty())
            tokens << QStringLiteral("#%1").arg(widget->objectName());

        for (const QMetaObject *metaObject = widget->metaObject(); metaObject; metaObject = metaObject->superClass()) {
            const QString className = QString::fromUtf8(metaObject->className());
            if (className.isEmpty())
                continue;

            tokens << className;

            if (className == QStringLiteral("QWidget"))
                break;
        }
    }

    tokens.removeDuplicates();
    return tokens;
}

QList<UBWidgetInspector::StyleSheetSource> UBWidgetInspector::buildStyleSheetSources(const QList<const QWidget*> &hierarchy)
{
    QList<StyleSheetSource> sources;

    if (!hierarchy.isEmpty()) {
        sources << StyleSheetSource{QStringLiteral("widgetQss %1").arg(describeWidget(hierarchy.first())),
                                    hierarchy.first()->styleSheet()};
    }

    for (int index = 1; index < hierarchy.size(); ++index) {
        sources << StyleSheetSource{QStringLiteral("parentQss %1").arg(describeWidget(hierarchy.at(index))),
                                    hierarchy.at(index)->styleSheet()};
    }

    sources << StyleSheetSource{QStringLiteral("appQss"), qApp ? qApp->styleSheet() : QString()};
    return sources;
}

QString UBWidgetInspector::buildRelevantStylesSection(const QList<StyleSheetSource> &sources, const QStringList &selectorTokens)
{
    QStringList sections;
    sections << QStringLiteral("Relevant Style Rules");

    for (const StyleSheetSource &source : sources) {
        const QStringList relevantRules = filterRelevantRules(source.styleSheet, selectorTokens);
        if (relevantRules.isEmpty())
            continue;

        sections << QStringLiteral("%1\n%2").arg(source.label, relevantRules.join(QStringLiteral("\n\n")));
    }

    if (sections.size() == 1)
        sections << QStringLiteral("No selector block referencing the target hierarchy was found.");

    return sections.join(QStringLiteral("\n\n"));
}

QStringList UBWidgetInspector::filterRelevantRules(const QString &styleSheet, const QStringList &selectorTokens)
{
    QStringList relevantRules;

    for (const StyleRule &rule : parseStyleRules(styleSheet)) {
        const QStringList matchedTokens = matchedTokensForSelectors(rule.selectors, selectorTokens);
        if (matchedTokens.isEmpty())
            continue;

        relevantRules << QStringLiteral("[matches: %1]\n%2 {\n%3\n}")
            .arg(matchedTokens.join(QStringLiteral(", ")),
                 rule.selectors.trimmed(),
                 indentMultiline(rule.body.trimmed(), QStringLiteral("    ")));
    }

    return relevantRules;
}

QList<UBWidgetInspector::StyleRule> UBWidgetInspector::parseStyleRules(const QString &styleSheet)
{
    const QString source = stripComments(styleSheet);
    QList<StyleRule> rules;
    int position = 0;

    while (position < source.size()) {
        const int openBrace = source.indexOf(QChar('{'), position);
        if (openBrace < 0)
            break;

        int depth = 1;
        int cursor = openBrace + 1;
        while (cursor < source.size() && depth > 0) {
            if (source.at(cursor) == QChar('{'))
                ++depth;
            else if (source.at(cursor) == QChar('}'))
                --depth;
            ++cursor;
        }

        if (depth != 0)
            break;

        const QString selectors = source.mid(position, openBrace - position).trimmed();
        const QString body = source.mid(openBrace + 1, cursor - openBrace - 2).trimmed();
        if (!selectors.isEmpty() && !body.isEmpty())
            rules << StyleRule{selectors, body};

        position = cursor;
    }

    return rules;
}

QString UBWidgetInspector::stripComments(const QString &styleSheet)
{
    QString stripped = styleSheet;
    static const QRegularExpression commentPattern(QStringLiteral("/\\*.*?\\*/"),
        QRegularExpression::DotMatchesEverythingOption);
    stripped.remove(commentPattern);
    return stripped;
}

QStringList UBWidgetInspector::matchedTokensForSelectors(const QString &selectors, const QStringList &selectorTokens)
{
    QStringList matchedTokens;
    const QStringList selectorParts = selectors.split(QChar(','), Qt::SkipEmptyParts);

    for (const QString &selectorPart : selectorParts) {
        const QString selector = selectorPart.trimmed();
        if (selector.isEmpty())
            continue;

        for (const QString &token : selectorTokens) {
            if (selector.contains(token))
                matchedTokens << token;
        }
    }

    matchedTokens.removeDuplicates();
    return matchedTokens;
}

QString UBWidgetInspector::indentMultiline(const QString &text, const QString &indent)
{
    QStringList lines = normalizeStyleSheet(text).split(QChar('\n'));
    for (QString &line : lines)
        line.prepend(indent);
    return lines.join(QStringLiteral("\n"));
}

QString UBWidgetInspector::normalizeStyleSheet(const QString &styleSheet)
{
    QString normalized = styleSheet.trimmed();
    normalized.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    normalized.replace(QChar('\r'), QChar('\n'));
    return normalized;
}
