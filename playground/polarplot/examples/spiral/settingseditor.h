#ifndef _SETTINGS_EDITOR_H_
#define _SETTINGS_EDITOR_H_ 1

#include <qframe.h>
#include "plot.h"

class QCheckBox;

class SettingsEditor: public QFrame
{
    Q_OBJECT

public:
    SettingsEditor(QWidget *parent = NULL);

    void showSettings(const PlotSettings &);
    PlotSettings settings() const;
    
signals:
    void edited(const PlotSettings&);

private slots:
    void edited();

private:
    void updateEditor();

    QCheckBox *d_majorGridBox[QwtPolar::ScaleCount];
    QCheckBox *d_minorGridBox[QwtPolar::ScaleCount];
    QCheckBox *d_axisBox[QwtPolar::AxesCount];
    QCheckBox *d_antialiasing;
    QCheckBox *d_spiralData;
};

#endif
