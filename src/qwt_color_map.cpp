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

//! Constructor
QwtColorMap::QwtColorMap( Format format ):
    d_format( format )
{
}

//! Destructor
QwtColorMap::~QwtColorMap()
{
}

/*!
   Build and return a color map of 256 colors

   The color table is needed for rendering indexed images in combination
   with using colorIndex().

   \param interval Range for the values
   \return A color table, that can be used for a QImage
*/
QVector<QRgb> QwtColorMap::colorTable( const QwtInterval &interval ) const
{
    QVector<QRgb> table( 256 );

    if ( interval.isValid() )
    {
        const double step = interval.width() / ( table.size() - 1 );
        for ( int i = 0; i < table.size(); i++ )
            table[i] = rgb( interval, interval.minValue() + step * i );
    }

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

    double ratio = 0.0;
    if ( width > 0.0 )
        ratio = ( value - interval.minValue() ) / width;

    return d_data->colorStops.rgb( d_data->mode, ratio );
}

/*!
  \brief Map a value of a given interval into a color index

  \param interval Range for all values
  \param value Value to map into a color index

  \return Index, between 0 and 255
*/
unsigned char QwtLinearColorMap::colorIndex(
    const QwtInterval &interval, double value ) const
{
    const double width = interval.width();

    if ( qIsNaN(value) || width <= 0.0 || value <= interval.minValue() )
        return 0;

    if ( value >= interval.maxValue() )
        return 255;

    const double ratio = ( value - interval.minValue() ) / width;

    unsigned char index;
    if ( d_data->mode == FixedColors )
        index = static_cast<unsigned char>( ratio * 255 ); // always floor
    else
        index = static_cast<unsigned char>( ratio * 255 + 0.5 );

    return index;
}

class QwtAlphaColorMap::PrivateData
{
public:
    QColor color;
    QRgb rgb;
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
}

/*!
  \return the color
  \sa setColor()
*/
QColor QwtAlphaColorMap::color() const
{
    return d_data->color;
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
        return d_data->rgb;

    const double width = interval.width();
    if ( width <= 0.0 )
        return d_data->rgb;

    const double ratio = ( value - interval.minValue() ) / width;
    int alpha = qRound( 255 * ratio );
    if ( alpha < 0 )
        alpha = 0;
    if ( alpha > 255 )
        alpha = 255;

    return d_data->rgb | ( alpha << 24 );
}

/*!
  Dummy function, needed to be implemented as it is pure virtual
  in QwtColorMap. Color indices make no sense in combination with
  an alpha channel.

  \return Always 0
*/
unsigned char QwtAlphaColorMap::colorIndex(
    const QwtInterval &, double ) const
{
    return 0;
}

class QwtHueColorMap::PrivateData
{
public:
    int round255( double value ) const;
    QRgb toRgb( int h, int s, int v ) const;
    
    int hue1;
    int hue2;
    int saturation;
    int value;
};

inline int QwtHueColorMap::PrivateData::round255( double value ) const
{
#if 1
    // The HSV -> RGB implementation of QColor looks buggy
    // but, when we want to have exactly the same values:

    return qRound( ( 257.0 / 255.0 ) * value ) / 256;
#else
    // this one  should be the correct implementation,
    // but differs sometimes by 1

    return qRound( value / 255 );
#endif
}

inline QRgb QwtHueColorMap::PrivateData::toRgb( int h, int s, int v ) const
{
    if ( s == 0 )
        return qRgb( v, v, v );

    // p doesn't depend on h: could be precalculated !

    const double hd = (h % 360) / 60.0;
    const int hi = int(hd);

    const double f = ( hi & 1 ) ? ( hd - hi ) : 1.0 - ( hd - hi );

    const int p = round255( v * ( 255 - s ) );
    const int q = round255( v * ( 255 - s * f ) );

    switch( hi )
    {
        case 1:
            return qRgb( q, v, p );
        case 2:
            return qRgb( p, v, q );
        case 3:
            return qRgb( p, q, v );
        case 4:
            return qRgb( q, p, v );
        case 5:
            return qRgb( v, p, q );
        default:
            return qRgb( v, q, p );
    }
}

QwtHueColorMap::QwtHueColorMap()
{
    d_data = new PrivateData;

    d_data->hue1 = 0;
    d_data->hue2 = 0;
    d_data->saturation = 255;
    d_data->value = 255;
}

QwtHueColorMap::~QwtHueColorMap()
{
    delete d_data;
}

void QwtHueColorMap::setHueInterval( int hue1, int hue2 )
{
    d_data->hue1 = qBound( 0, hue1, 359 );
    d_data->hue2 = qBound( 0, hue2, 359 );
}

void QwtHueColorMap::setSaturation( int saturation )
{
    d_data->saturation = qBound( 0, saturation, 255 );
}

void QwtHueColorMap::setValue( int value )
{
    d_data->value = qBound( 0, value, 255 );
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
    const double width = interval.width();
    if ( qIsNaN(value) || interval.width() <= 0.0 )
        return 0u;

    if ( value <= interval.minValue() )
    {
        return d_data->toRgb( d_data->hue1, d_data->saturation, d_data->value );
    }
    if ( value >= interval.maxValue() )
    {
        return d_data->toRgb( d_data->hue2, d_data->saturation, d_data->value );
    }

    const double ratio = ( value - interval.minValue() ) / width;

    int hue2 = d_data->hue2;
    if ( hue2 <= d_data->hue1 )
        hue2 += 360;

    int hue = d_data->hue1 + qRound( ratio * ( hue2 - d_data->hue1 ) );
    if ( hue >= 360 )
        hue -= 360;

    return d_data->toRgb( hue, d_data->saturation, d_data->value );
}

unsigned char QwtHueColorMap::colorIndex( const QwtInterval &, double ) const
{
    return 0;
}

