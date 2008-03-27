#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include "settingseditor.h"

SettingsEditor::SettingsEditor(QWidget *parent):
    QFrame(parent)
{
    QGroupBox *axesBox = new QGroupBox("Axes", this);
    QVBoxLayout* axesBoxLayout = new QVBoxLayout(axesBox);
    for ( int axisId = 0; axisId < QwtRadialPlotGrid::AxesCount; axisId++ )
    {
        d_axisBox[axisId] = new QCheckBox(axesBox);
        axesBoxLayout->addWidget(d_axisBox[axisId]);
        connect(d_axisBox[axisId], SIGNAL(clicked()), this, SLOT(edited()) );
    }
    axesBoxLayout->addStretch(10);

    QGroupBox *gridBox = new QGroupBox("Grids", this);
    QVBoxLayout* gridBoxLayout = new QVBoxLayout(gridBox);
    
    for ( int scaleId = 0; scaleId < QwtRadialPlot::ScaleCount; scaleId++ )
    {
        d_majorGridBox[scaleId] = new QCheckBox(gridBox);
        gridBoxLayout->addWidget(d_majorGridBox[scaleId]);
        connect(d_majorGridBox[scaleId], SIGNAL(clicked()), this, SLOT(edited()) );

        d_minorGridBox[scaleId] = new QCheckBox(gridBox);
        gridBoxLayout->addWidget(d_minorGridBox[scaleId]);
        connect(d_minorGridBox[scaleId], SIGNAL(clicked()), this, SLOT(edited()) );
    }
    gridBoxLayout->addStretch(10);

    QGroupBox *otherBox = new QGroupBox("Other", this);
    QVBoxLayout* otherBoxLayout = new QVBoxLayout(otherBox);
    d_antialiasing = new QCheckBox(otherBox);
    connect(d_antialiasing, SIGNAL(clicked()), this, SLOT(edited()) );
    otherBoxLayout->addWidget(d_antialiasing);
    otherBoxLayout->addStretch(10);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(axesBox);
    layout->addWidget(gridBox);
    layout->addWidget(otherBox);
    layout->addStretch(10);

    d_majorGridBox[QwtRadialPlot::AngleScale]->setText("Angle");
    d_majorGridBox[QwtRadialPlot::DistanceScale]->setText("Distance");
    d_minorGridBox[QwtRadialPlot::AngleScale]->setText("Angle Minor");
    d_minorGridBox[QwtRadialPlot::DistanceScale]->setText("Distance Minor");
    
    d_axisBox[QwtRadialPlotGrid::AngleAxis]->setText("Angle");
    d_axisBox[QwtRadialPlotGrid::LeftAxis]->setText("Left");
    d_axisBox[QwtRadialPlotGrid::RightAxis]->setText("Right");
    d_axisBox[QwtRadialPlotGrid::TopAxis]->setText("Top");
    d_axisBox[QwtRadialPlotGrid::BottomAxis]->setText("Bottom");

    d_antialiasing->setText("Antialiased Scales/Grids");
}

void SettingsEditor::showSettings(const PlotSettings &settings)
{
    blockSignals(true);
    for ( int scaleId = 0; scaleId < QwtRadialPlot::ScaleCount; scaleId++ )
    {
        d_majorGridBox[scaleId]->setChecked(settings.majorGrid[scaleId]);
        d_minorGridBox[scaleId]->setChecked(settings.minorGrid[scaleId]);
    }

    for ( int axisId = 0; axisId < QwtRadialPlotGrid::AxesCount; axisId++ )
        d_axisBox[axisId]->setChecked(settings.axis[axisId]);

    d_antialiasing->setChecked(settings.antialiasing);

    blockSignals(false);
    updateEditor();
}

PlotSettings SettingsEditor::settings() const
{
    PlotSettings settings;
    for ( int scaleId = 0; scaleId < QwtRadialPlot::ScaleCount; scaleId++ )
    {
        settings.majorGrid[scaleId] = d_majorGridBox[scaleId]->isChecked();
        settings.minorGrid[scaleId] = d_minorGridBox[scaleId]->isChecked();
    }
    
    for ( int axisId = 0; axisId < QwtRadialPlotGrid::AxesCount; axisId++ )
        settings.axis[axisId] = d_axisBox[axisId]->isChecked();
        
    settings.antialiasing = d_antialiasing->isChecked();

    return settings;
}

void SettingsEditor::edited()
{
    updateEditor();

    const PlotSettings s = settings();
    emit edited(s);
}

void SettingsEditor::updateEditor()
{
    for ( int scaleId = 0; scaleId < QwtRadialPlot::ScaleCount; scaleId++ )
    {
        d_minorGridBox[scaleId]->setEnabled(
            d_majorGridBox[scaleId]->isChecked());
    }
}
