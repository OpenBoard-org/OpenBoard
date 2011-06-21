#ifndef UB_ABSTRACTDRAWRULER_H_
#define UB_ABSTRACTDRAWRULER_H_

#include <QtGui>
class UBGraphicsScene;

class UBAbstractDrawRuler : public QObject
{
    Q_OBJECT;

    public:
		UBAbstractDrawRuler();
		~UBAbstractDrawRuler();

		virtual void StartLine(const QPointF& position, qreal width) = 0;
		virtual void DrawLine(const QPointF& position, qreal width) = 0;
		virtual void EndLine() = 0;

};

#endif