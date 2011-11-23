#ifndef _SETTINGS_
#define _SETTINGS_

class Settings
{
public:
    Settings()
    {
        legend.isEnabled = false;
        legend.position = 0;

        legendItem.isEnabled = false;
        legendItem.numColumns = 0;
        legendItem.alignment = 0;
        legendItem.backgroundMode = 0;

        numCurves = 0;
    }
    
    struct
    {
        bool isEnabled;
        int position;
    } legend;

    struct
    {
        bool isEnabled;
        int numColumns;
        int alignment;
        int backgroundMode;
        
    } legendItem;
    
    int numCurves;
};

#endif
