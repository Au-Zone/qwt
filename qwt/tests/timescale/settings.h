#ifndef _SETTINGS_H_
#define _SETTINGS_H_ 1

#include <qdatetime.h>

class Settings
{
public:
    Settings():
        maxMajor( 10 ),
        maxMinor( 5 ),
        maxWeeks( -1 )
    {
    };

    QDateTime startDateTime;
    QDateTime endDateTime;

    int maxMajor;
    int maxMinor;

    int maxWeeks;
};

#endif
