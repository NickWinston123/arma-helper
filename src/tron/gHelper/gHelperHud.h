#include "defs.h"
#ifndef ArmageTron_GHELPER_HUD
#define ArmageTron_GHELPER_HUD
#include "../gHud.h"
#include "rFont.h"
#include "tConfiguration.h"
#include "tString.h"

namespace helperConfig
{
    extern bool sg_helperHud;
    extern bool sg_helperHudFreeze;
    extern REAL sg_helperHudX;
    extern REAL sg_helperHudY;
    extern REAL sg_helperHudSize;
}

class gHelperHudBase
{
public:
    int id;
    static tThemedText theme;
    
protected:
    const std::string label;
    std::string parent;

public:
    gHelperHudBase(int id_, std::string label_, std::string parent = "");

    virtual std::string getLabel() { return label; }
    virtual tString getLabelStr() { return tString(label); }
    virtual std::string getParent() { return parent; }
    virtual tString getValue() { return tString("Default"); };
    virtual tString getLastValue() { return tString("Default"); }
    virtual bool valueSame() { return true; }
    virtual void setLastValue() { return; }
    virtual tString displayString() { return tString("gHelperHudBase (ERROR?)"); }

    static void Render();

public:
    typedef std::map<std::string, gHelperHudBase *> gHelperHudMap;
    static gHelperHudMap &GetHelperHudMap();
};

template <typename T>
class gHelperHudItem : virtual public gHelperHudBase
{
public:
    gHelperHudItem(std::string label_, T value_, std::string parent_ = "")
        : value(value_), id(0), parentID(0), gHelperHudBase(0, label_, parent_)
    {
        setLastValue();
    }

    int id;
    int parentID;
    REAL locX;
    REAL locY;
    REAL size;
    T value;
    T lastValue;
    
    bool reference;

    
    virtual tString getValue()
    {
        tString valueStr;
        !helperConfig::sg_helperHudFreeze ? valueStr << value : valueStr << lastValue;
        return valueStr;
    };

    virtual tString getLastValue()
    {
        tString valueStr;
        valueStr << lastValue;
        return valueStr;
    };

    virtual void setValue(T val)
    {
        if (!helperConfig::sg_helperHud || helperConfig::sg_helperHudFreeze)
            return;

        if (val != value)
        {
            value = val;
            setLastValue();
        }
    };

    gHelperHudItem &operator<<(T val)
    {
        setValue(val);
        return *this;
    }

    virtual bool valueSame() { return getLastValue() == getValue(); }

    virtual void setLastValue()
    {
        if (helperConfig::sg_helperHudFreeze)
            return;
        lastValue = value;
    }

    virtual tString displayString()
    {
        tString displayString;
        displayString << theme.HeaderColor()
                      << gHelperHudItem<T>::label 
                      << "0xRESETT: "
                      << theme.ItemColor()
                      << getValue()
                      << "\n";
        return displayString;
    }
};

template <typename T>
class gHelperHudItemRef : public gHelperHudItem<T>
{
public:
    gHelperHudItemRef(std::string label_, T &value_, std::string parent_ = "")
        : gHelperHudItem<T>(label_, value_), valueRef(value_), lastValueRef(valueRef), gHelperHudBase(0, label_, parent_)
    {
        gHelperHudItem<T>::reference = true;
        setLastValue();
    }

    T &valueRef;
    T &lastValueRef;

    virtual tString getValue()
    {
        tString valueStr;
        valueStr << valueRef;
        return valueStr;
    }

    virtual tString getLastValue()
    {
        tString valueStr;
        valueStr << lastValueRef;
        return valueStr;
    }

    virtual tString displayString()
    {
        tString displayStr;
        tString valueStr = getValue();
        tString value;

        if (valueStr == "0")
            value << gHelperHudBase::theme.ErrorColor() << "Disabled";
        else if (valueStr == "1")
            value << gHelperHudBase::theme.MainColor() << "Enabled";
        else
            value << gHelperHudBase::theme.ItemColor() << valueStr;

        displayStr << gHelperHudBase::theme.HeaderColor() << gHelperHudItem<T>::label << "0xRESETT: " << value << "\n";
        
        return displayStr;
    }

    virtual bool valueSame() { return getValue() == getLastValue(); }

    virtual void setLastValue() { lastValueRef = valueRef; }
};
#endif
