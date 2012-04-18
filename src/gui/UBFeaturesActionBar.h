#ifndef UBFEATURESACTIONBAR_H
#define UBFEATURESACTIONBAR_H

#include <QWidget>
#include <QToolButton>
#include "UBLibActionBar.h"

class UBFeaturesActionBar : public QWidget
{
	Q_OBJECT
public:
	UBFeaturesActionBar(QWidget* parent=0, const char* name="UBFeaturesActionBar");
    ~UBFeaturesActionBar();
};


#endif