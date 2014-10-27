/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_color_map.h"
#include "qwt_math.h"
#include "qwt_interval.h"
#include <qnumeric.h>

static inline QRgb qwtHsvToRgb( int h, int s, int v, int a )
{
#if 0
    return QColor::fromHsv( h, s, v, a ).rgb();
#else

    const double vs = v * s / 255.0;
    const int p = v - qRound( vs );

    switch( h / 60 ) 
    {
        case 0:
        {
            const double r = ( 60 - h ) / 60.0;
            return qRgba( v, v - qRound( r * vs ), p, a );
        }
        case 1:
        {
            const double r = ( h - 60 ) / 60.0;
            return qRgba( v - qRound( r * vs ), v, p, a );
        }
        case 2:
        {
            const double r = ( 180 - h ) / 60.0;
            return qRgba( p, v, v - qRound( r * vs ), a );
        }
        case 3:
        {
            const double r = ( h - 180 ) / 60.0;
            return qRgba( p, v - qRound( r * vs ), v, a );
        }
        case 4:
        {
            const double r = ( 300 - h ) / 60.0;
            return qRgba( v - qRound( r * vs ), p, v, a );
        }
        case 5:
        default:
        {
            const double r = ( h - 300 ) / 60.0;
            return qRgba( v, p, v - qRound( r * vs ), a );
        }
    }
#endif
}

class QwtLinearColorMap::ColorStops
{
public:
    ColorStops():
        d_doAlpha( false )
    {
        d_stops.reserve( 256 );
    }

    void insert( double pos, const QColor &color );
    QRgb rgb( QwtLinearColorMap::Mode, double pos ) const;

    QVector<double> stops() const;

private:

    class ColorStop
    {
    public:
        ColorStop():
            pos( 0.0 ),
            rgb( 0 )
        {
        };

        ColorStop( double p, const QColor &c ):
            pos( p ),
            rgb( c.rgba() )
        {
            r = qRed( rgb );
            g = qGreen( rgb );
            b = qBlue( rgb );
            a = qAlpha( rgb );

            /* 
                when mapping a value to rgb we will have to calcualate: 
                   - const int v = int( ( s1.v0 + ratio * s1.vStep ) + 0.5 );

                Thus adding 0.5 ( for rounding ) can be done in advance
             */
            r0 = r + 0.5;
            g0 = g + 0.5;
            b0 = b + 0.5;
            a0 = a + 0.5;

            rStep = gStep = bStep = aStep = 0.0;
            posStep = 0.0;
        }

        void updateSteps( const ColorStop &nextStop )
        {
            rStep = nextStop.r - r;
            gStep = nextStop.g - g;
            bStep = nextStop.b - b;
            aStep = nextStop.a - a;
            posStep = nextStop.pos - pos;
        }

        double pos;
        QRgb rgb;
        int r, g, b, a;

        // precalculated values
        double rStep, gStep, bStep, aStep;
        double r0, g0, b0, a0;
        double posStep;
    };

    inline int findUpper( double pos ) const;
    QVector<ColorStop> d_stops;
    bool d_doAlpha;
};

void QwtLinearColorMap::ColorStops::insert( double pos, const QColor &color )
{
    // Lookups need to be very fast, insertions are not so important.
    // Anyway, a balanced tree is what we need here. TODO ...

    if ( pos < 0.0 || pos > 1.0 )
        return;

    int index;
    if ( d_stops.size() == 0 )
    {
        index = 0;
        d_stops.resize( 1 );
    }
    else
    {
        index = findUpper( pos );
        if ( index == d_stops.size() ||
                qAbs( d_stops[index].pos - pos ) >= 0.001 )
        {
            d_stops.resize( d_stops.size() + 1 );
            for ( int i = d_stops.size() - 1; i > index; i-- )
                d_stops[i] = d_stops[i-1];
        }
    }

    d_stops[index] = ColorStop( pos, color );
    if ( color.alpha() != 255 )
        d_doAlpha = true;

    if ( index > 0 )
        d_stops[index-1].updateSteps( d_stops[index] );

    if ( index < d_stops.size() - 1 )
        d_stops[index].updateSteps( d_stops[index+1] );
}

inline QVector<double> QwtLinearColorMap::ColorStops::stops() const
{
    QVector<double> positions( d_stops.size() );
    for ( int i = 0; i < d_stops.size(); i++ )
        positions[i] = d_stops[i].pos;
    return positions;
}

inline int QwtLinearColorMap::ColorStops::findUpper( double pos ) const
{
    int index = 0;
    int n = d_stops.size();

    const ColorStop *stops = d_stops.data();

    while ( n > 0 )
    {
        const int half = n >> 1;
        const int middle = index + half;

        if ( stops[middle].pos <= pos )
        {
            index = middle + 1;
            n -= half + 1;
        }
        else
            n = half;
    }

    return index;
}

inline QRgb QwtLinearColorMap::ColorStops::rgb(
    QwtLinearColorMap::Mode mode, double pos ) const
{
    if ( pos <= 0.0 )
        return d_stops[0].rgb;
    if ( pos >= 1.0 )
        return d_stops[ d_stops.size() - 1 ].rgb;

    const int index = findUpper( pos );
    if ( mode == FixedColors )
    {
        return d_stops[index-1].rgb;
    }
    else
    {
        const ColorStop &s1 = d_stops[index-1];

        const double ratio = ( pos - s1.pos ) / ( s1.posStep );

        const int r = int( s1.r0 + ratio * s1.rStep );
        const int g = int( s1.g0 + ratio * s1.gStep );
        const int b = int( s1.b0 + ratio * s1.bStep );

        if ( d_doAlpha )
        {
            if ( s1.aStep )
            {
                const int a = int( s1.a0 + ratio * s1.aStep );
                return qRgba( r, g, b, a );
            }
            else
            {
                return qRgba( r, g, b, s1.a );
            }
        }
        else
        {
            return qRgb( r, g, b );
        }
    }
}

/*!
   Constructor
   \param format Format of the color map
*/
QwtColorMap::QwtColorMap( Format format ):
    d_format( format )
{
}

//! Destructor
QwtColorMap::~QwtColorMap()
{
}

/*!
   Set the format of the color map

   \param format Format of the color map
*/
void QwtColorMap::setFormat( Format format )
{
    d_format = format;
}

/*!
  \brief Map a value of a given interval into a color index

  \param numColors Number of colors
  \param interval Range for all values
  \param value Value to map into a color index

  \return Index, between 0 and numColors - 1, or -1 for an invalid value
  \note NaN values are mapped to 0
*/
int QwtColorMap::colorIndex( int numColors,
    const QwtInterval &interval, double value ) const
{
    if ( qIsNaN( value ) )
        return -1;

    const double width = interval.width();
    if ( width <= 0.0 )
        return -1;

    if ( value <= interval.minValue() )
        return 0;

    if ( value >= interval.maxValue() )
        return numColors - 1;

    const double v = ( numColors - 1 ) * ( value - interval.minValue() ) / width;
    return static_cast<unsigned int>( v + 0.5 );
}

/*!
   Build and return a color map of 256 colors

   The color table is needed for rendering indexed images in combination
   with using colorIndex().

   \param interval Range for the values
   \return A color table, that can be used for a QImage
*/
QVector<QRgb> QwtColorMap::colorTable256() const
{
    QVector<QRgb> table( 256 );

    const QwtInterval interval( 0, 256 );

    for ( int i = 0; i < 256; i++ )
        table[i] = rgb( interval, i );

    return table;
}

QVector<QRgb> QwtColorMap::colorTable( int numColors ) const
{
    QVector<QRgb> table( numColors );

    const QwtInterval interval( 0.0, 1.0 );

    const double step = 1.0 / ( numColors - 1 );
    for ( int i = 0; i < numColors; i++ )
        table[i] = rgb( interval, step * i );

    return table;
}

class QwtLinearColorMap::PrivateData
{
public:
    ColorStops colorStops;
    QwtLinearColorMap::Mode mode;
};

/*!
   Build a color map with two stops at 0.0 and 1.0. The color
   at 0.0 is Qt::blue, at 1.0 it is Qt::yellow.

   \param format Preferred format of the color map
*/
QwtLinearColorMap::QwtLinearColorMap( QwtColorMap::Format format ):
    QwtColorMap( format )
{
    d_data = new PrivateData;
    d_data->mode = ScaledColors;

    setColorInterval( Qt::blue, Qt::yellow );
}

/*!
   Build a color map with two stops at 0.0 and 1.0.

   \param color1 Color used for the minimum value of the value interval
   \param color2 Color used for the maximum value of the value interval
   \param format Preferred format for the color map
*/
QwtLinearColorMap::QwtLinearColorMap( const QColor &color1,
        const QColor &color2, QwtColorMap::Format format ):
    QwtColorMap( format )
{
    d_data = new PrivateData;
    d_data->mode = ScaledColors;
    setColorInterval( color1, color2 );
}

//! Destructor
QwtLinearColorMap::~QwtLinearColorMap()
{
    delete d_data;
}

/*!
   \brief Set the mode of the color map

   FixedColors means the color is calculated from the next lower
   color stop. ScaledColors means the color is calculated
   by interpolating the colors of the adjacent stops.

   \sa mode()
*/
void QwtLinearColorMap::setMode( Mode mode )
{
    d_data->mode = mode;
}

/*!
   \return Mode of the color map
   \sa setMode()
*/
QwtLinearColorMap::Mode QwtLinearColorMap::mode() const
{
    return d_data->mode;
}

/*!
   Set the color range

   Add stops at 0.0 and 1.0.

   \param color1 Color used for the minimum value of the value interval
   \param color2 Color used for the maximum value of the value interval

   \sa color1(), color2()
*/
void QwtLinearColorMap::setColorInterval(
    const QColor &color1, const QColor &color2 )
{
    d_data->colorStops = ColorStops();
    d_data->colorStops.insert( 0.0, color1 );
    d_data->colorStops.insert( 1.0, color2 );
}

/*!
   Add a color stop

   The value has to be in the range [0.0, 1.0].
   F.e. a stop at position 17.0 for a range [10.0,20.0] must be
   passed as: (17.0 - 10.0) / (20.0 - 10.0)

   \param value Value between [0.0, 1.0]
   \param color Color stop
*/
void QwtLinearColorMap::addColorStop( double value, const QColor& color )
{
    if ( value >= 0.0 && value <= 1.0 )
        d_data->colorStops.insert( value, color );
}

/*!
   \return Positions of color stops in increasing order
*/
QVector<double> QwtLinearColorMap::colorStops() const
{
    return d_data->colorStops.stops();
}

/*!
  \return the first color of the color range
  \sa setColorInterval()
*/
QColor QwtLinearColorMap::color1() const
{
    return QColor( d_data->colorStops.rgb( d_data->mode, 0.0 ) );
}

/*!
  \return the second color of the color range
  \sa setColorInterval()
*/
QColor QwtLinearColorMap::color2() const
{
    return QColor( d_data->colorStops.rgb( d_data->mode, 1.0 ) );
}

/*!
  Map a value of a given interval into a RGB value

  \param interval Range for all values
  \param value Value to map into a RGB value

  \return RGB value for value
*/
QRgb QwtLinearColorMap::rgb(
    const QwtInterval &interval, double value ) const
{
    if ( qIsNaN(value) )
        return 0u;

    const double width = interval.width();
    if ( width <= 0.0 )
        return 0u;

    const double ratio = ( value - interval.minValue() ) / width;
    return d_data->colorStops.rgb( d_data->mode, ratio );
}

/*!
  \brief Map a value of a given interval into a color index

  \param numColors Size of the color table
  \param interval Range for all values
  \param value Value to map into a color index

  \return Index, between 0 and 255
  \note NaN values are mapped to 0
*/
int QwtLinearColorMap::colorIndex( int numColors,
    const QwtInterval &interval, double value ) const
{
    if ( qIsNaN( value ) )
        return -1;

    const double width = interval.width();
    if ( width <= 0.0 )
        return -1;

    if ( value <= interval.minValue() )
        return 0;

    if ( value >= interval.maxValue() )
        return numColors - 1;

    const double v = ( numColors - 1 ) * ( value - interval.minValue() ) / width;
    return static_cast<unsigned int>( ( d_data->mode == FixedColors ) ? v : v + 0.5 );
}

class QwtAlphaColorMap::PrivateData
{
public:
    PrivateData():
        alpha1(0),
        alpha2(255)
    {
    }

    int alpha1, alpha2;

    QColor color;
    QRgb rgb;

    QRgb rgbMin;
    QRgb rgbMax;
};


/*!
   Constructor
   \param color Color of the map
*/
QwtAlphaColorMap::QwtAlphaColorMap( const QColor &color ):
    QwtColorMap( QwtColorMap::RGB )
{
    d_data = new PrivateData;
    setColor( color );
}

//! Destructor
QwtAlphaColorMap::~QwtAlphaColorMap()
{
    delete d_data;
}

/*!
   Set the color

   \param color Color
   \sa color()
*/
void QwtAlphaColorMap::setColor( const QColor &color )
{
    d_data->color = color;
    d_data->rgb = color.rgb() & qRgba( 255, 255, 255, 0 );

    d_data->rgbMin = d_data->rgb | ( d_data->alpha1 << 24 );
    d_data->rgbMax = d_data->rgb | ( d_data->alpha2 << 24 );
}

/*!
  \return the color
  \sa setColor()
*/
QColor QwtAlphaColorMap::color() const
{
    return d_data->color;
}

void QwtAlphaColorMap::setAlphaInterval( int alpha1, int alpha2 )
{
    d_data->alpha1 = qBound( 0, alpha1, 255 );
    d_data->alpha2 = qBound( 0, alpha2, 255 );

    d_data->rgbMin = d_data->rgb | ( alpha1 << 24 );
    d_data->rgbMax = d_data->rgb | ( alpha2 << 24 );
}

int QwtAlphaColorMap::alpha1() const
{
    return d_data->alpha1;
}

int QwtAlphaColorMap::alpha2() const
{
    return d_data->alpha2;
}

/*!
  \brief Map a value of a given interval into a alpha value

  alpha := (value - interval.minValue()) / interval.width();

  \param interval Range for all values
  \param value Value to map into a RGB value
  \return RGB value, with an alpha value
*/
QRgb QwtAlphaColorMap::rgb( const QwtInterval &interval, double value ) const
{
    if ( qIsNaN(value) )
        return 0u;

    const double width = interval.width();
    if ( width <= 0.0 )
        return 0u;

    if ( value <= interval.minValue() )
        return d_data->rgb;

    if ( value >= interval.maxValue() )
        return d_data->rgbMax;

    const double ratio = ( value - interval.minValue() ) / width;
    const int alpha = d_data->alpha1 + qRound( ratio * ( d_data->alpha2 - d_data->alpha1 ) );

    return d_data->rgb | ( alpha << 24 );
}

class QwtHueColorMap::PrivateData
{
public:
    PrivateData();

    void updateTable();

    int hue1, hue2;
    int saturation;
    int value;
    int alpha;

    QRgb rgbMin;
    QRgb rgbMax;

    QRgb rgbTable[360];
};

QwtHueColorMap::PrivateData::PrivateData():
    hue1(0),
    hue2(359),
    saturation(255),
    value(255),
    alpha(255)
{
    updateTable();
}

void QwtHueColorMap::PrivateData::updateTable()
{
    const int p = qRound( value * ( 255 - saturation ) / 255.0 );
    const double vs = value * saturation / 255.0;

    for ( int i = 0; i < 60; i++ )
    {
        const double r = ( 60 - i ) / 60.0;
        rgbTable[i] = qRgba( value, qRound( value - r * vs ), p, alpha );
    }

    for ( int i = 60; i < 120; i++ )
    {
        const double r = ( i - 60 ) / 60.0;
        rgbTable[i] = qRgba( qRound( value - r * vs ), value, p, alpha );
    }

    for ( int i = 120; i < 180; i++ )
    {
        const double r = ( 180 - i ) / 60.0;
        rgbTable[i] = qRgba( p, value, qRound( value - r * vs ), alpha );
    }

    for ( int i = 180; i < 240; i++ )
    {
        const double r = ( i - 180 ) / 60.0;
        rgbTable[i] = qRgba( p, qRound( value - r * vs ), value, alpha );
    }

    for ( int i = 240; i < 300; i++ )
    {
        const double r = ( 300 - i ) / 60.0;
        rgbTable[i] = qRgba( qRound( value - r * vs ), p, value, alpha );
    }

    for ( int i = 300; i < 360; i++ )
    {
        const double r = ( i - 300 ) / 60.0;
        rgbTable[i] = qRgba( value, p, qRound( value - r * vs ), alpha );
    }

    rgbMin = rgbTable[ hue1 % 360 ];
    rgbMax = rgbTable[ hue2 % 360 ];
}

QwtHueColorMap::QwtHueColorMap( QwtColorMap::Format format ):
    QwtColorMap( format )
{
    d_data = new PrivateData;
}

QwtHueColorMap::~QwtHueColorMap()
{
    delete d_data;
}

void QwtHueColorMap::setHueInterval( int hue1, int hue2 )
{
    d_data->hue1 = qMax( hue1, 0 );
    d_data->hue2 = qMax( hue2, 0 );

    d_data->rgbMin = d_data->rgbTable[ hue1 % 360 ];
    d_data->rgbMax = d_data->rgbTable[ hue2 % 360 ];
}

void QwtHueColorMap::setSaturation( int saturation )
{
    saturation = qBound( 0, saturation, 255 );

    if ( saturation != d_data->saturation )
    {
        d_data->saturation = saturation;
        d_data->updateTable();
    }
}

void QwtHueColorMap::setValue( int value )
{
    value = qBound( 0, value, 255 );

    if ( value != d_data->value )
    {
        d_data->value = value;
        d_data->updateTable();
    }
}

int QwtHueColorMap::hue1() const
{
    return d_data->hue1;
}

int QwtHueColorMap::hue2() const
{
    return d_data->hue2;
}

int QwtHueColorMap::saturation() const
{
    return d_data->saturation;
}

int QwtHueColorMap::value() const
{
    return d_data->value;
}

QRgb QwtHueColorMap::rgb( const QwtInterval &interval, double value ) const
{
    if ( qIsNaN(value) )
        return 0u;

    const double width = interval.width();
    if ( width <= 0 )
        return 0u;

    if ( value <= interval.minValue() )
        return d_data->rgbMin;

    if ( value >= interval.maxValue() )
        return d_data->rgbMax;

    const double ratio = ( value - interval.minValue() ) / width;
    
    int hue = d_data->hue1 + qRound( ratio * ( d_data->hue2 - d_data->hue1 ) );
    if ( hue >= 360 )
    {
        hue -= 360;

        if ( hue >= 360 )
            hue = hue % 360;
    }

    return d_data->rgbTable[hue];
}

class QwtSaturationValueColorMap::PrivateData
{
public:
    PrivateData():
        hue(0),
        sat1(255),
        sat2(255),
        value1(0),
        value2(255),
        alpha(255),
        tableType(Invalid)
    {
        updateTable();
    }

    void updateTable()
    {
        tableType = Invalid;

        if ( ( value1 == value2 ) && ( sat1 != sat2 ) )
        {
            rgbTable.resize( 256 );

            for ( int i = 0; i < 256; i++ )
                rgbTable[i] = qwtHsvToRgb( hue, i, value1, alpha );

            tableType = Saturation;
        }
        else if ( ( value1 != value2 ) && ( sat1 == sat2 ) )
        {
            rgbTable.resize( 256 );

            for ( int i = 0; i < 256; i++ )
                rgbTable[i] = qwtHsvToRgb( hue, sat1, i, alpha );

            tableType = Value;
        }
        else
        {
            rgbTable.resize( 256 * 256 );

            for ( int s = 0; s < 256; s++ )
            {
                const int v0 = s * 256;

                for ( int v = 0; v < 256; v++ )
                    rgbTable[v0 + v] = qwtHsvToRgb( hue, s, v, alpha );
            }
        }
    }

    int hue;
    int sat1, sat2;
    int value1, value2;
    int alpha;

    enum
    {
        Invalid,
        Value,
        Saturation

    } tableType;

    QVector<QRgb> rgbTable;
};

QwtSaturationValueColorMap::QwtSaturationValueColorMap()
{
    d_data = new PrivateData;
}

QwtSaturationValueColorMap::~QwtSaturationValueColorMap()
{
    delete d_data;
}

void QwtSaturationValueColorMap::setHue( int hue )
{
    hue = hue % 360;

    if ( hue != d_data->hue )
    {
        d_data->hue = hue;
        d_data->updateTable();
    }
}

void QwtSaturationValueColorMap::setSaturationInterval( int sat1, int sat2 )
{
    sat1 = qBound( 0, sat1, 255 );
    sat2 = qBound( 0, sat2, 255 );

    if ( ( sat1 != d_data->sat1 ) || ( sat2 != d_data->sat2 ) )
    {
        d_data->sat1 = sat1;
        d_data->sat1 = sat1;
        d_data->sat1 = sat1;

        d_data->updateTable();
    }
}

void QwtSaturationValueColorMap::setValueInterval( int value1, int value2 )
{
    value1 = qBound( 0, value1, 255 );
    value2 = qBound( 0, value2, 255 );

    if ( ( value1 != d_data->value1 ) || ( value2 != d_data->value2 ) )
    {
        d_data->value1 = value1;
        d_data->value2 = value2;
    
        d_data->updateTable();
    }
}

int QwtSaturationValueColorMap::value1() const
{
    return d_data->value1;
}

int QwtSaturationValueColorMap::value2() const
{
    return d_data->value2;
}

int QwtSaturationValueColorMap::hue() const
{
    return d_data->hue;
}

int QwtSaturationValueColorMap::saturation1() const
{
    return d_data->sat1;
}

int QwtSaturationValueColorMap::saturation2() const
{
    return d_data->sat2;
}

QRgb QwtSaturationValueColorMap::rgb( const QwtInterval &interval, double value ) const
{
    if ( qIsNaN(value) )
        return 0u;

    const double width = interval.width();
    if ( width <= 0 )
        return 0u;

    const QRgb *rgbTable = d_data->rgbTable.constData();

    switch( d_data->tableType )
    {
        case PrivateData::Saturation:
        {
            if ( value <= interval.minValue() )
                return d_data->rgbTable[d_data->sat1];

            if ( value >= interval.maxValue() )
                return d_data->rgbTable[d_data->sat2];

            const double ratio = ( value - interval.minValue() ) / width;
            const int sat = d_data->sat1
                + qRound( ratio * ( d_data->sat2 - d_data->sat1 ) );

            return rgbTable[sat];
        }
        case PrivateData::Value:
        {
            if ( value <= interval.minValue() )
                return d_data->rgbTable[d_data->value1];

            if ( value >= interval.maxValue() )
                return d_data->rgbTable[d_data->value2];

            const double ratio = ( value - interval.minValue() ) / width;
            const int v = d_data->value1 +
                qRound( ratio * ( d_data->value2 - d_data->value1 ) );

            return rgbTable[ v ];
        }
        default:
        {
            int s, v;
            if ( value <= interval.minValue() )
            {
                s = d_data->sat1;
                v = d_data->value1;
            }
            else if ( value >= interval.maxValue() )
            {
                s = d_data->sat2;
                v = d_data->value2;
            }
            else
            {
                const double ratio = ( value - interval.minValue() ) / width;

                v = d_data->value1 + qRound( ratio * ( d_data->value2 - d_data->value1 ) );
                s = d_data->sat1 + qRound( ratio * ( d_data->sat2 - d_data->sat1 ) );
            }

            return rgbTable[ 256 * s + v ];
        }
    }
}
