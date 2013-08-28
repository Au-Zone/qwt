#include "mainwindow.h"
#include "formulaview.h"
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qfiledialog.h>
#include <qbuffer.h>
#include <qstatusbar.h>
#include <qdebug.h>

MainWindow::MainWindow()
{
    d_view = new FormulaView( this );
    setCentralWidget( d_view );

    QToolBar *toolBar = new QToolBar( this );

    QToolButton *btnLoad = new QToolButton( toolBar );

    btnLoad->setText( "Load" );
    btnLoad->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    toolBar->addWidget( btnLoad );

    addToolBar( toolBar );

    connect( btnLoad, SIGNAL( clicked() ), this, SLOT( load() ) );
};

void MainWindow::load()
{
    const QString fileName = QFileDialog::getOpenFileName( NULL,
        "Load a Scaleable Vector Graphic (SVG) Document",
        QString::null, "MathML Files (*.mml)" );

    if ( !fileName.isEmpty() )
        loadFormula( fileName );

    statusBar()->showMessage( fileName );
}

void MainWindow::loadFormula( const QString &fileName )
{
    QFile file( fileName );
    if ( !file.open(QIODevice::ReadOnly | QIODevice::Text) )
        return;

    const QByteArray document = file.readAll();
    file.close();

    d_view->setFormula( document );
}
