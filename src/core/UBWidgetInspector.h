#ifndef UBWIDGETINSPECTOR_H_
#define UBWIDGETINSPECTOR_H_

#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>

class QDialog;
class QEvent;
class QPlainTextEdit;
class QWidget;

class UBWidgetInspector : public QObject
{
    Q_OBJECT

public:
    explicit UBWidgetInspector(QObject *parent = nullptr);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    struct StyleSheetSource
    {
        QString label;
        QString styleSheet;
    };

    struct StyleRule
    {
        QString selectors;
        QString body;
    };

    void inspectWidgetUnderCursor();
    QString buildInfo(const QWidget *widget) const;
    void showInspector(const QString &info);
    void ensureInspectorDialog();
    void positionInspector();

    static QString describeWidget(const QWidget *widget);
    static QList<const QWidget*> buildWidgetHierarchy(const QWidget *widget);
    static QString buildHierarchySection(const QList<const QWidget*> &hierarchy);
    static QStringList buildSelectorTokens(const QList<const QWidget*> &hierarchy);
    static QList<StyleSheetSource> buildStyleSheetSources(const QList<const QWidget*> &hierarchy);
    static QString buildRelevantStylesSection(const QList<StyleSheetSource> &sources, const QStringList &selectorTokens);
    static QStringList filterRelevantRules(const QString &styleSheet, const QStringList &selectorTokens);
    static QList<StyleRule> parseStyleRules(const QString &styleSheet);
    static QString stripComments(const QString &styleSheet);
    static QStringList matchedTokensForSelectors(const QString &selectors, const QStringList &selectorTokens);
    static QString indentMultiline(const QString &text, const QString &indent);
    static QString normalizeStyleSheet(const QString &styleSheet);

    QPointer<QDialog> mInspectorDialog;
    QPointer<QPlainTextEdit> mInspectorView;
};

#endif /* UBWIDGETINSPECTOR_H_ */
