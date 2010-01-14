#include "panel.h"
#include <qlabel.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qwt_plot_curve.h>
#include <iostream>

class GridLayout: public QGridLayout
{
public:
	GridLayout(QWidget *parent):
		QGridLayout(parent)
	{
#if QT_VERSION < 0x040000
		setMargin(10);
		setSpacing(5);
#endif
	}
#if QT_VERSION < 0x040000
	void setColumnStretch ( int column, int stretch )
	{
		setColStretch(column, stretch);
	}

	void addWidget(QWidget * w, int row, int col) 
	{
		QGridLayout::addWidget(w, row, col);
	}

	void addWidget(QWidget *w, int fromRow, int fromColumn, 
		int rowSpan, int columnSpan)
	{
		int toRow = ( rowSpan == -1 ) ? numRows() : fromRow + rowSpan - 1;
		int toCol = ( columnSpan == -1 ) ? numCols() : fromColumn + columnSpan -1;

		addMultiCellWidget(w, fromRow, toRow, fromColumn, toCol);
	}
#endif
};

class ComboBox: public QComboBox
{
public:
	ComboBox(QWidget *parent):
		QComboBox(parent)
	{
	}

#if QT_VERSION < 0x040000
	void setCurrentIndex(int idx)
	{
		setCurrentItem(idx);
	}

	int currentIndex() const
	{
		return currentItem();
	}

	void addItem(const QString &item)
	{
		insertItem(item);
	}
#endif
};

class SpinBox: public QSpinBox
{
public:
	SpinBox(int min, int max, int step, QWidget *parent):
		QSpinBox(parent)
	{
#if QT_VERSION < 0x040000
		setMinValue(min);
		setMaxValue(max);
		setLineStep(step);
#else
		setRange(min, max);
		setSingleStep(step);
#endif
	}
};

class CheckBox: public QCheckBox
{
public:
	CheckBox(const QString &title, QWidget *parent):
		QCheckBox(title, parent)
	{
	}

#if QT_VERSION >= 0x040000
	void setChecked(bool)
	{
		setCheckState(Qt::Checked);
	}

	bool isChecked() const
	{
		return checkState() == Qt::Checked;
	}
#endif
};

Panel::Panel(QWidget *parent):
    QTabWidget(parent)
{
#if QT_VERSION >= 0x040000
    setTabPosition(QTabWidget::West);
#endif

    addTab(createPlotTab(this), "Plot");
    addTab(createCanvasTab(this), "Canvas");
    addTab(createCurveTab(this), "Curve");

    setSettings(Settings());

    connect(d_numPoints, SIGNAL(valueChanged(int)), SLOT(edited()) );
    connect(d_updateInterval, SIGNAL(valueChanged(int)), SLOT(edited()) );
    connect(d_curveWidth, SIGNAL(valueChanged(int)), SLOT(edited()) );
    connect(d_paintCache, SIGNAL(stateChanged(int)), SLOT(edited()) );
    connect(d_canvasClipping, SIGNAL(stateChanged(int)), SLOT(edited()) );
#if QT_VERSION >= 0x040000
    connect(d_paintOnScreen, SIGNAL(stateChanged(int)), SLOT(edited()) );
    connect(d_curveAntialiasing, SIGNAL(stateChanged(int)), SLOT(edited()) );
#endif
    connect(d_curveClipping, SIGNAL(stateChanged(int)), SLOT(edited()) );
    connect(d_curveFilter, SIGNAL(stateChanged(int)), SLOT(edited()) );
    connect(d_lineSplitting, SIGNAL(stateChanged(int)), SLOT(edited()) );
    connect(d_curveFilled, SIGNAL(stateChanged(int)), SLOT(edited()) );

#if QT_VERSION < 0x040000
    connect(d_updateType, SIGNAL(activated(int)), SLOT(edited()) );
    connect(d_gridStyle, SIGNAL(activated(int)), SLOT(edited()) );
    connect(d_curveType, SIGNAL(activated(int)), SLOT(edited()) );
    connect(d_curvePen, SIGNAL(activated(int)), SLOT(edited()) );
#else
    connect(d_updateType, SIGNAL(currentIndexChanged(int)), SLOT(edited()) );
    connect(d_gridStyle, SIGNAL(currentIndexChanged(int)), SLOT(edited()) );
    connect(d_curveType, SIGNAL(currentIndexChanged(int)), SLOT(edited()) );
    connect(d_curvePen, SIGNAL(currentIndexChanged(int)), SLOT(edited()) );
#endif
}

QWidget *Panel::createPlotTab(QWidget *parent)
{
    QWidget *page = new QWidget(parent);

    d_updateInterval = new SpinBox(0, 1000, 10, page);
    d_numPoints = new SpinBox(10, 1000000, 1000, page);

    d_updateType = new ComboBox(page);
    d_updateType->addItem("Update");
    d_updateType->addItem("Repaint");
    d_updateType->addItem("Replot");

    int row = 0;

    GridLayout *layout = new GridLayout(page);

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

    d_gridStyle = new ComboBox(page);
    d_gridStyle->addItem("None");
    d_gridStyle->addItem("Solid");
    d_gridStyle->addItem("Dashes");

    d_paintCache = new CheckBox("Paint Cache", page);
#if QT_VERSION >= 0x040000
    d_paintOnScreen = new CheckBox("Paint On Screen", page);
#endif
    d_canvasClipping = new CheckBox("Canvas Clipping", page);

    int row = 0;

    GridLayout *layout = new GridLayout(page);
    layout->addWidget(new QLabel("Grid", page), row, 0);
    layout->addWidget(d_gridStyle, row++, 1);

    layout->addWidget(d_paintCache, row++, 0, 1, -1);
#if QT_VERSION >= 0x040000
    layout->addWidget(d_paintOnScreen, row++, 0, 1, -1);
#endif
    layout->addWidget(d_canvasClipping, row++, 0, 1, -1);

    layout->addLayout(new QHBoxLayout(), row++, 0);

    layout->setColumnStretch(1, 10);
    layout->setRowStretch(row, 10);

    return page;
}

QWidget *Panel::createCurveTab(QWidget *parent)
{
    QWidget *page = new QWidget(parent);

    d_curveType = new ComboBox(page);
    d_curveType->addItem("Wave");
    d_curveType->addItem("Noise");

#if QT_VERSION >= 0x040000
    d_curveAntialiasing = new CheckBox("Antialiasing", page);
#endif
    d_curveClipping = new CheckBox("Clipping", page);
    d_curveFilter = new CheckBox("Filter", page);
    d_lineSplitting = new CheckBox("Split Lines", page);

    d_curveWidth = new SpinBox(0, 10, 1, page);

    d_curvePen = new ComboBox(page);
    d_curvePen->addItem("Solid");
    d_curvePen->addItem("Dotted");

    d_curveFilled = new CheckBox("Filled", page);

    int row = 0;

    GridLayout *layout = new GridLayout(page);
    layout->addWidget(new QLabel("Type", page), row, 0 );
    layout->addWidget(d_curveType, row++, 1);

#if QT_VERSION >= 0x040000
    layout->addWidget(d_curveAntialiasing, row++, 0, 1, -1);
#endif
    layout->addWidget(d_curveClipping, row++, 0, 1, -1);
    layout->addWidget(d_curveFilter, row++, 0, 1, -1);
    layout->addWidget(d_lineSplitting, row++, 0, 1, -1);

    layout->addWidget(new QLabel("Width", page), row, 0 );
    layout->addWidget(d_curveWidth, row++, 1);

    layout->addWidget(new QLabel("Style", page), row, 0 );
    layout->addWidget(d_curvePen, row++, 1);

    layout->addWidget(d_curveFilled, row++, 0, 1, -1);

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
    s.curve.brush.setStyle((d_curveFilled->isChecked()) ?
        Qt::SolidPattern : Qt::NoBrush);
    s.curve.numPoints = d_numPoints->value();
    s.curve.functionType = (Settings::FunctionType)d_curveType->currentIndex();
    if ( d_curveClipping->isChecked() )
        s.curve.paintAttributes |= QwtPlotCurve::ClipPolygons;
    else
        s.curve.paintAttributes &= ~QwtPlotCurve::ClipPolygons;
    if ( d_curveFilter->isChecked() )
        s.curve.paintAttributes |= QwtPlotCurve::PaintFiltered;
    else
        s.curve.paintAttributes &= ~QwtPlotCurve::PaintFiltered;

#if QT_VERSION >= 0x040000
    if ( d_curveAntialiasing->isChecked() )
        s.curve.renderHint |= QwtPlotCurve::RenderAntialiased;
    else
        s.curve.renderHint &= ~QwtPlotCurve::RenderAntialiased;
#endif

    s.curve.lineSplitting = (d_lineSplitting->isChecked() );

    s.canvas.deviceClipping = (d_canvasClipping->isChecked() );
    s.canvas.cached = (d_paintCache->isChecked() );
#if QT_VERSION >= 0x040000
    s.canvas.paintOnScreen = (d_paintOnScreen->isChecked() );
#endif

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

    d_paintCache->setChecked(s.canvas.cached );
#if QT_VERSION >= 0x040000
    d_paintOnScreen->setChecked(s.canvas.paintOnScreen);
#endif
    d_canvasClipping->setChecked(s.canvas.deviceClipping);

    d_curveType->setCurrentIndex(s.curve.functionType);
#if QT_VERSION >= 0x040000
    d_curveAntialiasing->setChecked(
        s.curve.renderHint & QwtPlotCurve::RenderAntialiased );
#endif

    d_curveClipping->setChecked(
        s.curve.paintAttributes & QwtPlotCurve::ClipPolygons);

    d_curveFilter->setChecked(
        s.curve.paintAttributes & QwtPlotCurve::PaintFiltered);

    d_lineSplitting->setChecked(s.curve.lineSplitting );

    d_curveWidth->setValue(s.curve.pen.width());
    d_curvePen->setCurrentIndex(
        s.curve.pen.style() == Qt::SolidLine ? 0 : 1);
    d_curveFilled->setChecked(s.curve.brush.style() != Qt::NoBrush);
}
