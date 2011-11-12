#include "legend.h"
#include <qwt_legend_data.h>
#include <qwt_text.h>
#include <qtreeview.h>
#include <qlayout.h>
#include <qstyle.h>
#include <qwindowsstyle.h>
#include <qstandarditemmodel.h>
#include <qitemdelegate.h>
#include <qdebug.h>

class LegendTreeView: public QTreeView
{
public:
    LegendTreeView( Legend * );

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
};

LegendTreeView::LegendTreeView( Legend *legend ):
    QTreeView( legend )
{
    setFrameStyle( NoFrame );
    viewport()->setBackgroundRole(QPalette::Background);
    viewport()->setAutoFillBackground( false );

    setRootIsDecorated( false );
    setHeaderHidden( true );

    QStandardItemModel *model = new QStandardItemModel();

    QStandardItem *rootItem = new QStandardItem( "Stocks" );
    rootItem->setEditable( false );

    model->appendRow( rootItem );

    setModel( model );
    setExpanded( model->index( 0, 0 ), true );

    // we want unstyled items
    setItemDelegate( new QItemDelegate( this ) );

}

QSize LegendTreeView::minimumSizeHint() const
{
    return QSize( -1, -1 );
}

QSize LegendTreeView::sizeHint() const
{
    QStyleOptionViewItem styleOption;
    styleOption.initFrom( this );

    const QStandardItem *rootItem = 
        qobject_cast<QStandardItemModel *>( model() )->item( 0 );

    QAbstractItemDelegate *delegate = itemDelegate();
    
    const QSize rootHint = delegate->sizeHint( 
        styleOption, rootItem->index() );

    int w0 = rootHint.width();
    int h = rootHint.height();

    int w = 0;
    for ( int i = 0; i < rootItem->rowCount(); i++ )
    {
        const QSize hint = delegate->sizeHint( styleOption, 
            rootItem->child( i )->index() );

        w = qMax( w, hint.width() );
        h += hint.height();
    }
    w = qMax( w + indentation(), w0 );

    int left, right, top, bottom;
    getContentsMargins( &left, &top, &right, &bottom );

    w += left + right;
    h += top + bottom;

    return QSize( w, h );
}

Legend::Legend( QWidget *parent ):
    QwtAbstractLegend( parent )
{
    d_treeView = new LegendTreeView( this );

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->addWidget( d_treeView );

    connect( d_treeView, SIGNAL( clicked( const QModelIndex & ) ),
        this, SLOT( handleClick( const QModelIndex & ) ) );
}

Legend::~Legend()
{
}

void Legend::renderLegend( QPainter *painter,
    const QRectF &rect, bool fillBackground ) const
{
    Q_UNUSED( painter );
    Q_UNUSED( rect );
    Q_UNUSED( fillBackground );
}

bool Legend::isEmpty() const
{
    const QStandardItemModel *model =
        qobject_cast<QStandardItemModel *>( d_treeView->model() );

    const QStandardItem *rootItem = model->item( 0 );

    return rootItem->rowCount() == 0;
}

int Legend::scrollExtent( Qt::Orientation ) const
{
    return style()->pixelMetric( QStyle::PM_ScrollBarExtent );
}

void Legend::updateLegend( const QwtPlotItem *plotItem,
    const QList<QwtLegendData> &data )
{
    QStandardItemModel *model = 
        qobject_cast<QStandardItemModel *>( d_treeView->model() );

    QStandardItem *rootItem = model->item( 0 );

    QList<QStandardItem *> itemList;
    for ( int i = 0; i < rootItem->rowCount(); i++ )
    {
        QStandardItem *item = rootItem->child( i );
        
        const QVariant key = item->data();

        if ( qVariantCanConvert<qlonglong>( key ) )
        {
            const qlonglong ptr = qVariantValue<qlonglong>( key );
            if ( ptr == qlonglong( plotItem ) )
                itemList += item;
        }
    }

    while ( itemList.size() < data.size() )
    {
        QStandardItem *item = new QStandardItem();
        item->setEditable( false );
        item->setData( qlonglong( plotItem ) );
        item->setCheckable( true );
        item->setCheckState( Qt::Checked );

        itemList += item;
        rootItem->appendRow( item );
    }
        
    while ( itemList.size() > data.size() )
    {
        QStandardItem *item = itemList.takeLast();
        rootItem->removeRow( item->row() );
    }

    for ( int i = 0; i < itemList.size(); i++ )
        updateItem( itemList[i], data[i] );

    d_treeView->updateGeometry();
}

void Legend::updateItem( QStandardItem *item, const QwtLegendData &data )
{
    const QVariant titleValue = data.value( QwtLegendData::TitleRole );

    QwtText title;
    if ( qVariantCanConvert<QwtText>( titleValue ) )
    {
        item->setText( title.text() );
        title = qVariantValue<QwtText>( titleValue );
    }
    else if ( qVariantCanConvert<QString>( titleValue ) )
    {
        title.setText( qVariantValue<QString>( titleValue ) );
    }
    item->setText( title.text() );

    const QVariant iconValue = data.value( QwtLegendData::IconRole );

    QPixmap pm;
    if ( qVariantCanConvert<QPixmap>( iconValue ) )
        pm = qVariantValue<QPixmap>( iconValue );

    item->setData(pm, Qt::DecorationRole);
}

void Legend::handleClick( const QModelIndex &index )
{
    const QStandardItemModel *model =
        qobject_cast<QStandardItemModel *>( d_treeView->model() );

    const QStandardItem *item = model->itemFromIndex( index );
    const qlonglong ptr = qVariantValue<qlonglong>( item->data() );
    
    Q_EMIT checked( (QwtPlotItem *)ptr, 
        item->checkState() == Qt::Checked, 0 );
}
