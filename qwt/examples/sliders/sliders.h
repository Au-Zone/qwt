#include <qwidget.h>

class QLabel;
class QLayout;
class QwtSlider;

class SliderBox: public QWidget
{
    Q_OBJECT
public:
    SliderBox( QWidget *parent, int sliderType );

private Q_SLOTS:
    void setNum( double v );

private:
    QwtSlider *createSlider( int sliderType ) const;

    QwtSlider *d_slider;
    QLabel *d_label;
};

class SliderDemo : public QWidget
{
public:
    SliderDemo( QWidget *p = NULL );
};
