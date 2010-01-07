#ifndef _PANEL_H_
#define _PANEL_H_ 1

#include "settings.h"
#include <qtabwidget.h>

class QComboBox;
class QSpinBox;
class QCheckBox;

class Panel: public QTabWidget
{
    Q_OBJECT

public:
    Panel(QWidget * = NULL);

    Settings settings() const;
    void setSettings(const Settings &);

signals:
    void settingsChanged(const Settings &);

private slots:
    void edited();

private:
    QWidget *createPlotTab(QWidget *);
    QWidget *createCanvasTab(QWidget *);
    QWidget *createCurveTab(QWidget *);

    QSpinBox *d_numPoints;
    QSpinBox *d_updateInterval;
    QComboBox *d_updateType;

    QComboBox *d_gridStyle;
    QCheckBox *d_paintCache;
    QCheckBox *d_paintOnScreen;
    QCheckBox *d_canvasClipping;

    QComboBox *d_curveType;
    QCheckBox *d_curveAntialiasing;
    QCheckBox *d_curveClipping;
    QCheckBox *d_curveFilter;
    QCheckBox *d_lineSplitting;
    QSpinBox  *d_curveWidth;
    QComboBox *d_curvePen;
    QCheckBox *d_curveFilled;
};

#endif
