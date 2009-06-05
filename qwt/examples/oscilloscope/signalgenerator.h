#include <qobject.h>
#include <qdatetime.h>

class SignalGenerator: public QObject
{
	Q_OBJECT
public:
	SignalGenerator(QObject *parent = NULL);

signals:
	void value(double);

protected:
	virtual void timerEvent(QTimerEvent *);
	QTime d_startTime;
};
