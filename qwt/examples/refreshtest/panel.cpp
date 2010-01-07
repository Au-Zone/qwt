#include "panel.h"
#include <qlabel.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qwt_plot_curve.h>

Panel::Panel(QWidget *parent):
    QTabWidget(parent)
{
#if 1
    setTabPosition(QTabWidget::West);
#endif

    addTab(createPlotTab(this), "Plot");
    addTab(createCanvasTab(this), "Canvas");
    addTab(createCurveTab(this), "Curve");

    setSettings(Settings());

    connect(d_numPoints, SIGNAL(valueChanged(int)), SLOT(edited()) );
    connect(d_updateInterval, SIGNAL(valueChanged(int)), SLOT(edited()) );
    connect(d_updateType, SIGNAL(currentIndexChanged(int)), SLOT(edited()) );
    connect(d_gridStyle, SIGNAL(currentIndexChanged(int)), SLOT(edited()) );
    connect(d_paintCache, SIGNAL(stateChanged(int)), SLOT(edited()) );
    connect(d_paintOnScreen, SIGNAL(stateChanged(int)), SLOT(edited()) );
    connect(d_canvasClipping, SIGNAL(stateChanged(int)), SLOT(edited()) );
    connect(d_curveType, SIGNAL(currentIndexChanged(int)), SLOT(edited()) );
    connect(d_curveAntialiasing, SIGNAL(stateChanged(int)), SLOT(edited()) );
    connect(d_curveClipping, SIGNAL(stateChanged(int)), SLOT(edited()) );
    connect(d_curveFilter, SIGNAL(stateChanged(int)), SLOT(edited()) );
    connect(d_curveWidth, SIGNAL(valueChanged(int)), SLOT(edited()) );
    connect(d_curvePen, SIGNAL(currentIndexChanged(int)), SLOT(edited()) );
}

QWidget *Panel::createPlotTab(QWidget *parent)
{
    QWidget *page = new QWidget(parent);

    d_updateInterval = new QSpinBox(page);
    d_updateInterval->setRange(0, 1000);
    d_updateInterval->setSingleStep(10);

    d_numPoints = new QSpinBox(page);
    d_numPoints->setRange(10, 1000000);
    d_numPoints->setSingleStep(1000);

    d_updateType = new QComboBox(page);
    d_updateType->addItem("Update Canvas");
    d_updateType->addItem("Repaint Canvas");
    d_updateType->addItem("Replot");

    int row = 0;

    QGridLayout *layout = new QGridLayout(page);

    layout->addWidget(new QLabel("Updates", page), row, 0 );
    layout->addWidget(d_updateInterval, row, 1);
    layout->addWidget(new QLabel("ms", page), row++, 2 );

    layout->addWidget(new QLabel("Points", page), row, 0 );
    layout->addWidget(d_numPoints, row++, 1);

    layout->addWidget(new QLabel("Update", page), row, 0 );
    layout->addWidget(d_updateType, row++, 1);

    layout->addLayout(new QHBoxLayout(), row++, 0);

    layout->setColumnStretch(1, 10);
    layout->setRowStretch(row, 10);

    return page;
}

QWidget *Panel::createCanvasTab(QWidget *parent)
{
    QWidget *page = new QWidget(parent);

    d_gridStyle = new QComboBox(page);
    d_gridStyle->addItem("None");
    d_gridStyle->addItem("Solid");
    d_gridStyle->addItem("Dashes");

    d_paintCache = new QCheckBox("Paint Cache", page);
    d_paintOnScreen = new QCheckBox("Paint On Screen", page);
    d_canvasClipping = new QCheckBox("Canvas Clipping", page);

    int row = 0;

    QGridLayout *layout = new QGridLayout(page);
    layout->addWidget(new QLabel("Grid", page), row, 0);
    layout->addWidget(d_gridStyle, row++, 1);

    layout->addWidget(d_paintCache, row++, 0, 1, -1);
    layout->addWidget(d_paintOnScreen, row++, 0, 1, -1);
    layout->addWidget(d_canvasClipping, row++, 0, 1, -1);

    layout->addLayout(new QHBoxLayout(), row++, 0);

    layout->setColumnStretch(1, 10);
    layout->setRowStretch(row, 10);

    return page;
}

QWidget *Panel::createCurveTab(QWidget *parent)
{
    QWidget *page = new QWidget(parent);

    d_curveType = new QComboBox(page);
    d_curveType->addItem("Wave");
    d_curveType->addItem("Noise");

    d_curveAntialiasing = new QCheckBox("Antialiasing", page);
    d_curveClipping = new QCheckBox("Clipping", page);
    d_curveFilter = new QCheckBox("Filter", page);

    d_curveWidth = new QSpinBox(page);
    d_curveWidth->setRange(0, 10);

    d_curvePen = new QComboBox(page);
    d_curvePen->addItem("Solid");
    d_curvePen->addItem("Dotted");

    int row = 0;

    QGridLayout *layout = new QGridLayout(page);
    layout->addWidget(new QLabel("Type", page), row, 0 );
    layout->addWidget(d_curveType, row++, 1);

    layout->addWidget(d_curveAntialiasing, row++, 0, 1, -1);
    layout->addWidget(d_curveClipping, row++, 0, 1, -1);
    layout->addWidget(d_curveFilter, row++, 0, 1, -1);

    layout->addWidget(new QLabel("Width", page), row, 0 );
    layout->addWidget(d_curveWidth, row++, 1);

    layout->addWidget(new QLabel("Style", page), row, 0 );
    layout->addWidget(d_curvePen, row++, 1);

    layout->addLayout(new QHBoxLayout(), row++, 0);

    layout->setColumnStretch(1, 10);
    layout->setRowStretch(row, 10);

    return page;
}

void Panel::edited()
{
    const Settings s = settings();
    emit settingsChanged(s);
}


Settings Panel::settings() const
{
    Settings s;

    s.grid.pen = QPen(Qt::black);

    switch(d_gridStyle->currentIndex())
    {
        case 0:
            s.grid.pen.setStyle(Qt::NoPen);
            break;
        case 2:
            s.grid.pen.setStyle(Qt::DashLine);
            break;
    }
    
    s.curve.pen.setStyle(d_curvePen->currentIndex() == 0 ?
        Qt::SolidLine : Qt::DotLine);
    s.curve.pen.setWidth(d_curveWidth->value());
    // s.curve.brush
    s.curve.numPoints = d_numPoints->value();
    s.curve.functionType = (Settings::FunctionType)d_curveType->currentIndex();
    if ( d_curveClipping->checkState() == Qt::Checked )
        s.curve.paintAttributes |= QwtPlotCurve::ClipPolygons;
    else
        s.curve.paintAttributes &= ~QwtPlotCurve::ClipPolygons;
    if ( d_curveFilter->checkState() == Qt::Checked )
        s.curve.paintAttributes |= QwtPlotCurve::PaintFiltered;
    else
        s.curve.paintAttributes &= ~QwtPlotCurve::PaintFiltered;

    if ( d_curveAntialiasing->checkState() == Qt::Checked )
        s.curve.renderHint |= QwtPlotCurve::RenderAntialiased;
    else
        s.curve.renderHint &= ~QwtPlotCurve::RenderAntialiased;

    s.canvas.deviceClipping = (d_canvasClipping->checkState() == Qt::Checked);
    s.canvas.cached = (d_paintCache->checkState() == Qt::Checked);
    s.canvas.paintOnScreen = (d_paintOnScreen->checkState() == Qt::Checked);

    s.updateInterval = d_updateInterval->value();
    s.updateType = (Settings::UpdateType)d_updateType->currentIndex();

    return s;
}

void Panel::setSettings(const Settings &s)
{
    d_numPoints->setValue(s.curve.numPoints);
    d_updateInterval->setValue(s.updateInterval);
    d_updateType->setCurrentIndex(s.updateType);

    switch(s.grid.pen.style())
    {
        case Qt::NoPen:
            d_gridStyle->setCurrentIndex(0);
            break;
        case Qt::DashLine:
            d_gridStyle->setCurrentIndex(2);
            break;
        default:
            d_gridStyle->setCurrentIndex(1); // Solid
    }

    d_paintCache->setCheckState(s.canvas.cached ?
        Qt::Checked : Qt::Unchecked);
    d_paintOnScreen->setCheckState(s.canvas.paintOnScreen ?
        Qt::Checked : Qt::Unchecked);
    d_canvasClipping->setCheckState(s.canvas.deviceClipping ?
        Qt::Checked : Qt::Unchecked);

    d_curveType->setCurrentIndex(s.curve.functionType);
    d_curveAntialiasing->setCheckState(
        s.curve.renderHint & QwtPlotCurve::RenderAntialiased ?
        Qt::Checked : Qt::Unchecked);

    d_curveClipping->setCheckState(
        s.curve.paintAttributes & QwtPlotCurve::ClipPolygons ?
        Qt::Checked : Qt::Unchecked);

    d_curveFilter->setCheckState(
        s.curve.paintAttributes & QwtPlotCurve::PaintFiltered ?
        Qt::Checked : Qt::Unchecked);

    d_curveWidth->setValue(s.curve.pen.width());
    d_curvePen->setCurrentIndex(
        s.curve.pen.style() == Qt::SolidLine ? 0 : 1);

}
