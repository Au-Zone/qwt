#ifndef _PANEL_H_
#define _PANEL_H_ 1

#include "settings.h"
#include <qtabwidget.h>

class ComboBox;
class SpinBox;
class CheckBox;

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

    SpinBox *d_numPoints;
    SpinBox *d_updateInterval;
    ComboBox *d_updateType;

    ComboBox *d_gridStyle;
    CheckBox *d_paintCache;
#if QT_VERSION >= 0x040000
    CheckBox *d_paintOnScreen;
#endif
    CheckBox *d_canvasClipping;

    ComboBox *d_curveType;
#if QT_VERSION >= 0x040000
    CheckBox *d_curveAntialiasing;
#endif
    CheckBox *d_curveClipping;
    CheckBox *d_curveFilter;
    CheckBox *d_lineSplitting;
    SpinBox  *d_curveWidth;
    ComboBox *d_curvePen;
    CheckBox *d_curveFilled;
};

#endif
