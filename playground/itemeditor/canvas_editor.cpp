#include "canvas_editor.h"
#include "overlay.h"
#include <qwt_plot.h>
#include <qevent.h>

class CanvasEditorOverlay: public Overlay
{
public:
    CanvasEditorOverlay( 
            QWidget *parent, CanvasEditor *editor ):
        Overlay( parent ),
        d_editor( editor )
    {
    }

protected:
    virtual void drawOverlay( QPainter *painter ) const
    {
        d_editor->drawOverlay( painter );
    }

    virtual QRegion maskHint() const
    {
        return d_editor->maskHint();
    }

private:
    CanvasEditor *d_editor;
};

CanvasEditor::CanvasEditor( QwtPlot* plot ):
    QObject( plot ),
    m_isEnabled( false ),
    m_overlay( NULL )
{
}


CanvasEditor::~CanvasEditor()
{
    delete m_overlay;
}


void CanvasEditor::setEnabled( bool on )
{
    if ( on == m_isEnabled )
        return;

    QwtPlot *plot = qobject_cast<QwtPlot *>( parent() );
    if ( plot )
    {
        m_isEnabled = on;

        if ( on )
        {
            plot->canvas()->installEventFilter( this );
        }
        else
        {
            plot->canvas()->removeEventFilter( this );

            delete m_overlay;
            m_overlay = NULL;
        }
    }
}

bool CanvasEditor::isEnabled() const
{
    return m_isEnabled;
}

bool CanvasEditor::eventFilter( QObject* object, QEvent* event )
{
    QwtPlot *plot = qobject_cast<QwtPlot *>( parent() );
    if ( plot && object == plot->canvas() )
    {
        switch( event->type() )
        {
            case QEvent::MouseButtonPress:
            {
                const QMouseEvent* mouseEvent =
                    dynamic_cast<QMouseEvent* >( event );

                if ( m_overlay == NULL && 
                    mouseEvent->button() == Qt::LeftButton  )
                {
                    const bool accepted = pressed( mouseEvent->pos() );
                    if ( accepted )
                    {
                        m_overlay = new CanvasEditorOverlay( plot->canvas(), this );
                        m_overlay->resize( plot->canvas()->size() );

                        connect( m_overlay, SIGNAL( destroyed ( QObject* ) ),
                           this, SLOT( overlayDestroyed() ) );

                        updateOverlay();

                        m_overlay->show();
                    }
                }

                break;
            }
            case QEvent::MouseMove:
            {
                if ( m_overlay )
                {
                    const QMouseEvent* mouseEvent =
                        dynamic_cast< QMouseEvent* >( event );

                    const bool accepted = moved( mouseEvent->pos() );
                    if ( accepted )
                        updateOverlay();
                }

                break;
            }
            case QEvent::MouseButtonRelease:
            {
                const QMouseEvent* mouseEvent =
                    static_cast<QMouseEvent* >( event );

                if ( m_overlay && mouseEvent->button() == Qt::LeftButton )
                {
                    released( mouseEvent->pos() );

                    delete m_overlay;
                    m_overlay = NULL;
                }

                break;
            }
            case QEvent::Resize:
            {
                if ( m_overlay )
                {
                    const QResizeEvent* resizeEvent =
                        static_cast< const QResizeEvent* >( event );
                    if ( resizeEvent )
                        m_overlay->resize( resizeEvent->size() );
                }
                break;
            }
            default:
                break;
        }

        return false;
    }

    return QObject::eventFilter( object, event );
}

void CanvasEditor::overlayDestroyed()
{
    // The overlay widget is a child of the plot canvas
    // and might be destroyed together with it.

    m_overlay = NULL;
}


void CanvasEditor::updateOverlay()
{
    if ( m_overlay )
    {
        m_overlay->updateMask();
        m_overlay->update();
    }
}
