#include <QApplication>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QDebug>

#include <qwt_text.h>
#include <qwt_text_label.h>
#include <qwt_mathml_text_engine.h>

#include <string>

std::string fullText = R"(
<math xmlns='http://www.w3.org/1998/Math/MathML'
    mathematica:form='StandardForm'
    xmlns:mathematica='http://www.wolfram.com/XML/'>
 <mfrac>
  <msup>
   <mi>&#8519;</mi>
   <mrow>
    <mo>-</mo>
    <mi>x</mi>
   </mrow>
  </msup>
  <msup>
   <mrow>
    <mo>(</mo>
    <mrow>
     <mn>1</mn>
     <mo>+</mo>
     <msup>
      <mi>&#8519;</mi>
      <mrow>
       <mo>-</mo>
       <mi>x</mi>
      </mrow>
     </msup>
    </mrow>
    <mo>)</mo>
   </mrow>
   <mn>2</mn>
  </msup>
 </mfrac>
</math>
)";

/*
 * FIXME: Example only scales up as QwtTextLabel seems to force a minimum size.  Needs to be fixed.
 * TODO: This code can probably be integrated right into QwtTextLabel and submitted back upstream.
 */
class QwtScalingTextLabel : public QwtTextLabel
{
public:
    explicit QwtScalingTextLabel(const QwtText& text, QWidget *parent = nullptr) :
        QwtTextLabel(text, parent) {}

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        QSizeF textSize(text().textSize());

        QSizeF scale(contentsRect().width() / textSize.width(),
                     contentsRect().height() / textSize.height());

        qDebug() << "resizeEvent -> scale: " << scale;

        if ((scale.width() >= 1.05 || scale.width() < 1.0) &&
            (scale.height() >= 1.05 || scale.height() < 1.0)) {
            QwtText t = text();
            QFont f = t.font();

            auto pointSize = f.pointSizeF();
            auto scaledPointSize = pointSize * scale.width();

            qDebug() << "resizing font: " << scaledPointSize;

            f.setPointSizeF(scaledPointSize);
            t.setFont(f);
            setText(t);
        }
    }
};

int main(int argc, char** argv)
{
    /* The following globally sets the text engine for type MathMLText to 
     * QwtMathMLTextEngine.  It should be done before creating the application
     * and live for the lifetime of the application as shown here.  We do
     * not destroy the QwtMathMLTextEngine which may cause Valgrind to show
     * it as a leak even though it is not, you can create a pointer and delete
     * it before the end of main but after QApplication::exec.
     */
    QwtText::setTextEngine(QwtText::MathMLText, new QwtMathMLTextEngine());

    QApplication a(argc, argv);

    QWidget widget;
    QVBoxLayout *layout = new QVBoxLayout;

    QwtText text(QString(fullText.c_str()), QwtText::MathMLText);
    QwtScalingTextLabel *label = new QwtScalingTextLabel(text);
    layout->addWidget(label);

    widget.setLayout(layout);
    widget.show();
    return a.exec();
}
