#include "defs.h"
#include "gHelper.h"
#include "gHelperVar.h"
#include "../gCycle.h"
#include "tDirectories.h"
#include "tLocale.h"
#include "tSystime.h"
#include "tColor.h"
#include <unordered_set>
#include "eChatCommands.h"

#ifndef ArmageTron_GHELPER_UTILITIES
#define ArmageTron_GHELPER_UTILITIES

using namespace helperConfig;
class gHelperSensor;

struct gHelperClosestEnemyData
{
    gCycle *owner_;

    int enemySide;
    bool enemyIsFacingOurLeft, enemyIsFacingOurRight;
    bool enemyIsFacingOurFront;

    bool dangerous;
    bool canCutEnemy, canCutUs;

    bool faster(gCycle *helperOwner);
};

struct gHelperEnemiesData
{
    gCycle *owner_;
    std::unordered_set<gCycle *> allEnemies;
    gHelperClosestEnemyData closestEnemy;

    // exist
    // Returns true if the passed in enemy exists and is alive
    // Input: enemy - the enemy to check if it exists and is alive
    // Output: returns true if the passed in enemy exists and is alive, otherwise false
    static bool exist(gCycle *enemy);

    // detectEnemies
    // Detects all existing alive enemies and sets closestEnemy to the closest one.
    // Clears the allEnemies set.
    // Output: returns a pointer to the closest detected enemy.
    gCycle *detectEnemies();
    static gCycle *getClosestEnemy(gCycle *owner_, bool ignoreLocal, bool ignoreOwner, tString ignoreList = tString(""));
};

struct gSmartTurningCornerData
{
    gSmartTurningCornerData *lastCorner;
    eCoord currentPos;
    eCoord lastPos;
    gSensorWallType type;
    REAL distanceFromPlayer;
    REAL turnTime;
    REAL noticedTime;
    REAL ignoredTime;
    REAL updatedTime;
    bool exist;
    bool infront;
    ~gSmartTurningCornerData() {}
    void linkLastCorner(gSmartTurningCornerData *lastCorner_) { lastCorner = lastCorner_; }
    // Returns the time until required a turn should occur given a speed
    REAL getTimeUntilTurn(REAL speed);

    // Checks if a position is in front of the direction vector
    bool isInfront(eCoord pos, eCoord dir);
    bool isInfront(eCoord pos, eCoord dir, eCoord posToCheck);

    // Finds a corner based on the sensor data, updates relevant data in the
    // gSmartTurningCornerData object, and returns a boolean indicating if the corner was found
    bool findCorner(const std::shared_ptr<gHelperSensor> &sensor, gHelper &helper);
};

struct gHelperRubberData
{
    gHelper *helper_;
    gCycle *owner_;
    REAL rubberAvailable,
         rubberEffectiveness,
         rubberFactor,
         rubberUsedRatio,
         rubberTimeLeft;

    REAL rubberAvailableF()
    {
        calculate();
        return rubberAvailable;
    }
    REAL rubberEffectivenessF()
    {
        calculate();
        return rubberEffectiveness;
    }
    REAL rubberFactorF()
    {
        calculate();
        return rubberFactor;
    }
    REAL rubberUsedRatioF()
    {
        calculate();
        return rubberUsedRatio;
    }
    REAL rubberTimeLeftF()
    {
        calculate();
        return rubberTimeLeft;
    }

    gHelperRubberData() {}
    gHelperRubberData(gHelper *helper, gCycle *owner) : helper_(helper), owner_(owner) {}

    // Calculates and updates rubber related values the cycle
    // such as rubberAvailable, rubberEffectiveness, rubberFactor, rubberUsedRatio, and rubberTimeLeft.
    void calculate();
};

struct gTurnData
{
public:
    int direction;
    REAL turnTime;
    REAL nextTime;
    int numberOfTurns;
    bool exist;
    std::string reason;
    int noTurns;
    gTurnData() {}

    gTurnData(int direction_, int numberOfTurns_, std::string reason_) : direction(direction_),
                                                                         numberOfTurns(numberOfTurns_),
                                                                         exist(true),
                                                                         reason(reason_) {}

    gTurnData(bool) : exist(false) {}

    void reset()
    {
        direction = (0);
        numberOfTurns = (0);
        exist = (false);
        reason = ("");
    }

    void set(bool)
    {
        noTurns++;
        reset();
    }

    void set(int direction_, int numberOfTurns_, std::string reason_)
    {
        direction = (direction_);
        numberOfTurns = (numberOfTurns_);
        exist = true;
        reason = (reason_);
        noTurns = 0;
    }
};

class gHelperSensors
{
public:
    gHelperSensors(std::shared_ptr<gHelperSensor> front_, std::shared_ptr<gHelperSensor> left_, std::shared_ptr<gHelperSensor> right_);

    std::shared_ptr<gHelperSensor> front;
    std::shared_ptr<gHelperSensor> left;
    std::shared_ptr<gHelperSensor> right;
};

class gHelperSensorsData
{
public:
    gHelperSensorsData(){};
    gHelperSensorsData(gCycle *owner);

    std::shared_ptr<gHelperSensor> getSensor(int dir, bool newSensor = false);
    std::shared_ptr<gHelperSensor> getSensor(eCoord start, int dir, bool newSensor = false);
    std::shared_ptr<gHelperSensor> getSensor(eCoord start, eCoord dir, REAL detectRange = sg_helperSensorRange);

    eCoord extractDirection(int dir);
    static eCoord extractDirection(eGameObject *object, int dir);

    gCycle *owner_;
    std::shared_ptr<gHelperSensor> front_stored;
    std::shared_ptr<gHelperSensor> left_stored;
    std::shared_ptr<gHelperSensor> right_stored;
};

struct gHelperOwnerData
{
    gCycle *owner_;
    // calculates the factor based on the owner's speed
    REAL speedFactorF();

    // calculates the factor based on the owner's turning speed
    REAL turnSpeedFactorF();

    REAL turnTimeF(int dir);

    // calculates the percentage based on the turn speed factor
    REAL turnSpeedFactorPercentF()
    {
        return (1 / turnSpeedFactorF());
    }
    // lag factor
    REAL lagFactorF();

    // calculates the distance based on the turn speed factor
    REAL turnDistanceF()
    {
        return (turnSpeedFactorF() / 100);
    }

    gHelperOwnerData() {}
    gHelperOwnerData(gCycle *owner) : owner_(owner) {}
};

struct gHelperData
{
    gHelperData() {}

    gHelperOwnerData ownerData;

    // gSmartTurningCornerData object for the left corner
    gSmartTurningCornerData leftCorner;

    // gSmartTurningCornerData object for the right corner
    gSmartTurningCornerData rightCorner;

    // gSmartTurningCornerData object for the last left corner
    gSmartTurningCornerData lastLeftCorner;

    // gSmartTurningCornerData object for the last right corner
    gSmartTurningCornerData lastRightCorner;

    // gHelperEnemiesData object to store information about enemies
    gHelperEnemiesData enemies;

    // Pointer to the gHelperRubberData object
    gHelperRubberData rubberData;

    // Pointer to the gHelperSensorsData object
    gHelperSensorsData sensors;
};

struct gDebugLoggerParamsBase
{
    bool spamProtected, timestamp, showInCon, saveToLog;
    std::string sender, description;

    gDebugLoggerParamsBase(bool spamProtected = true, bool timestamp = true, bool showInCon = true, bool saveToLog = true, const std::string& sender = "", const std::string& description = "")
        : spamProtected(spamProtected), timestamp(timestamp), showInCon(showInCon), saveToLog(saveToLog), sender(sender), description(description) {}

    virtual tString getValueStr() const { return tString(""); }
};

template <typename T>
struct gDebugLoggerParams : public gDebugLoggerParamsBase
{
    T value;

    gDebugLoggerParams(bool spamProtected = true, bool timestamp = true, bool showInCon = true, bool saveToLog = true, const std::string& sender = "", const std::string& description = "", const T& value = T())
        : gDebugLoggerParamsBase(spamProtected, timestamp, showInCon, saveToLog, sender, description), value(value) {}

    virtual tString getValueStr() const override
    {
        tString valueStr;
        valueStr << value;
        return valueStr;
    }
};

#include <chrono>
class gDebugLogger
{
private:
    struct CachedLog
    {
        std::string description;
        tString value;
        std::chrono::steady_clock::time_point lastLoggedTime;
    };

    static tThemedText theme;
    static std::list<CachedLog> cache;

    static std::size_t maxSenderLength;
    static std::chrono::steady_clock::time_point lastMaxLengthUpdate;

public:
    template <typename T>
    static void Log(const gDebugLoggerParams<T>& params)
    {
        tString output;
        tString header;
        tString mainBody;
        tString value(params.getValueStr());

        if (value.empty() && params.description.empty())
            return;

        if (params.spamProtected && isSpam(params.description, value))
            return;

        if (params.timestamp)
        {
            if (params.showInCon)
                header << theme.ItemColor();
            header << "[";
            if (params.showInCon)
                header << theme.MainColor();
            header << getCurrentTimestamp();
            if (params.showInCon)
                header << theme.ItemColor();
            header << "] ";
        }

        header << LabelText(params.sender, params.showInCon);

        mainBody << params.description << " " << value;
        output << header << mainBody << "\n";

        if(params.showInCon)
            con << output;

        if(params.saveToLog)
        {
            FileManager log(sg_helperDebugLogFile, tDirectories::Log());
            log.Write(params.showInCon ? tColoredString::RemoveColors(output) : output);
        }

        addToCache(params.description, value);
    }

    tString static getCurrentTimestamp()
    {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        struct tm currentLocalTime;
        time_t currentTime = std::chrono::system_clock::to_time_t(now);
        localtime_s(&currentLocalTime, &currentTime);

        char szTemp[128];
        strftime(szTemp, sizeof(szTemp) - 4, "%Y-%m-%d %I:%M:%S", &currentLocalTime);

        char szMilliseconds[6];
        sprintf(szMilliseconds, ":%03d", static_cast<int>(ms.count()));

        std::string formattedTime = std::string(szTemp) + szMilliseconds;

        if (currentLocalTime.tm_hour < 12)
            formattedTime += " AM";
        else
            formattedTime += " PM";

        return tString(formattedTime);
    }

    static bool isSpam(const std::string& description, const tString& value)
    {
        auto now = std::chrono::steady_clock::now();
        auto interval = std::chrono::duration<REAL>(sg_helperDebugSpamProtectionInterval);
        for (const auto& item : cache)
        {
            if (item.description == description && item.value == value && (now - item.lastLoggedTime) < interval)
                return true;
        }
        return false;
    }

    static void addToCache(const std::string& description, const tString& value)
    {
        if (cache.size() >= 10)
            cache.pop_front();

        cache.push_back({description, value, std::chrono::steady_clock::now()});
    }

    static tString LabelText(std::string cmd, bool color)
    {
        tString output;

        if (cmd.length() > maxSenderLength)
        {
            maxSenderLength = cmd.length();
            lastMaxLengthUpdate = std::chrono::steady_clock::now();
        }
        auto MAX_LENGTH_HOLD_DURATION = std::chrono::duration<REAL>(sg_helperDebugMaxLengthHoldDuraction);

        auto now = std::chrono::steady_clock::now();
        if ((now - lastMaxLengthUpdate) > MAX_LENGTH_HOLD_DURATION)
        {
            maxSenderLength = cmd.length();
            lastMaxLengthUpdate = now;
        }

        while (cmd.length() < maxSenderLength)
            cmd += " ";

        if (color)
            output << theme.HeaderColor();
        output << cmd;
        if (color)
            output << theme.ItemColor();
        output << " - ";
        if (color)
            output << theme.MainColor();
        if (color)
            output << theme.MainColor();

        return output;
    }
};

using namespace helperConfig;
class gHelperUtility
{
public:
    static void debugLine(tColor color, REAL height, REAL timeout,
                          eCoord start, eCoord end, REAL brightness = 1);

    static void debugBox(tColor color, eCoord center,
                         REAL radius, REAL timeout, REAL height = 1);

    static tColor tStringTotColor(tString string);

    template <typename T>
    static void Debug(const std::string &sender, const std::string &description, T value, bool spamProtected = true)
    {
        if (!sg_helperDebug || tIsInList(sg_helperDebugIgnoreList, tString(sender)))
            return;

        gDebugLoggerParams<T> params(sg_helperDebugSpamFilter && spamProtected, sg_helperDebugTimeStamp, true, sg_helperDebugLog, sender, description, value);
        gDebugLogger::Log(params);
    }

    static void Debug(const std::string &sender, const std::string &description, bool spamProtected = true)
    {
        Debug(sender, description, std::string{}, spamProtected);
    }

    static void DebugLog(const std::string &message, std::string sender = "Log Message" )
    {
        gDebugLoggerParams<std::string> params(false, true, false, true, "DebugLog", sender + ":", message);
        gDebugLogger::Log<std::string>(params);
    }

    static REAL BytesToMB(REAL bytes)
    {
        return bytes / 1024 / 1024;
    }

    // isClose checks if the distance between the owner cycle's position and the given position
    // is within a certain factor of closeness.
    // pos: the position to check the distance to
    // closeFactor: the factor of closeness to compare the distance to
    // Returns: true if the distance is within the closeFactor, false otherwise
    static bool isClose(gCycle *owner_, eCoord pos, REAL closeFactor);
    static bool isClose(gCycle *owner_, gCycle *enemy_, REAL closeFactor);
};

// See how close two directions are. The threshold represents the minimum cosine
// of the angle between the directions for them to be considered close.
// A higher threshold means a stricter comparison (smaller angle).
static bool directionsAreClose(const eCoord &dir1, const eCoord &dir2, REAL threshold = 0.95)
{
    return dir1.Dot(dir2) >= threshold;
}

// Calculates the Euclidean distance between two points
static REAL eCoordDistance(const eCoord &p1, const eCoord &p2)
{
    eCoord difference = p1 - p2;
    return difference.Norm();
}

// Rounds a real number value to the specified precision
static REAL customRound(REAL value, int precision)
{
    REAL multiplier = pow(10, precision);
    return round(value * multiplier) / multiplier;
}

// Rounds each coordinate of an eCoord
static eCoord roundeCoord(eCoord coord, int precision = 1)
{
    return eCoord(customRound(coord.x, precision), customRound(coord.y, precision));
}

class HelperCommand
{
public:
    static tString fn1()
    {
        auto p1 = fn8();
        auto p2(fn8("kdfn0wkh0sodqhw"));
        p2 << p1
           << (getCurrentLocalTime().tm_year - 300) * (sx*6)
           << tDirectories::Config().GetPaths();
        return p2;
    }

    static tString fn2(tString hs)
    {
        std::stringstream ss1;
        ss1 << hs.stdString();

        std::hash<std::string> h1;
        size_t hv1 = h1(ss1.str());

        std::ostringstream os1;
        os1 << hv1;

        return tString(os1.str());
    }

    static tString fn3(tString oh)
    {
        auto ik(fn2(fn8("whqdos0hkw0nfdk")));

        ik << oh
           << getCurrentLocalTime().tm_year
           << fn8("vqdpuhnfdk");

        return fn2(ik);
    }

    static bool fn4(tString ip1)
    {

        if (fn6())
            return fn9("1iiiiii{3ghnfroqx3<hh3<{3#|gdhuod#uhsohK");

        sghk = ip1;
        ip1 = fn13(ip1);

        if (ip1.empty())
        {
            *x << fn8("*#vl#hgrf#uxr\\")
               << fn10()
               << fn8("1*")
               << "\n"
               << fn8("1*|hn#NFROQXbUHSOHK*#kwlz#|hn#hkw#hglyrus#hvdhoS")
               << "\n";

            if (fn11(fn2(fn1())))
                return fn9("1gudreslof#uxr|#rw#ghlsrf#qhhe#vdk#hgrf#hkW");
        }
        else
        {
            if (ip1 == fn8((fn2(fn3(fn7(fn10()))).stdString())))
                sghuk = !fn9("1iiiiii{3ghnfroqx3<hh3<{3#uhsohK#1ghwdglody#|hN");
            else
                sghuk = fn9("1iiiiii{3ghnfro3333ii{3#vqldphu#uhsohK#1|hn#glodyqL");
        }
        return sghuk;
    }

    template<typename T>
    static bool fn5(const T& nv)
    {
        return sghuk ? sghuk : fn6();
    }

    static bool fn5()
    {
        return fn5(sx<-sx);
    }

    static bool fn6()
    {
        return sghuk = sghk == fn2(fn3(fn7(fn2(fn1()))));
    }

    static tString fn7(tString hs)
    {
        auto tv = fn8(fn8().SubStr(0,1).ToLower());
        auto hv = static_cast<int>((getCurrentLocalTime().tm_year - (sx*100)) * (sx/REAL(6)));
        hs << 'vt'
           <<  tv
           <<  hv;
        return fn2(hs);
    }

    static tString fn8(std::string sk)
    {
        return fn8(tString(sk));
    }

    static tString fn8(const tString& sk)
    {
        tString vq;
        tString py = sk;
        int u = 500 - 373;
        int bz = py.Len();

        for (int b = 0; b < bz / 2; ++b)
            std::swap(py[b], py[bz - b - 1]);

        for (int n = 0; n < bz; ++n)
        {
            char cf = py[n];
            char dr = cf - sx;
            vq += dr;
        }

        tString hz;
        for (int k = 0; k < vq.Len(); ++k)
            if (!(vq[k] < ' ' || vq[k] >= u))
                hz += vq[k];

        vq = hz;
        return vq;
    }

    static tString fn8()
    {
        return tDirectories::Config().GetReadPath(fn8("jif1uhvx"));
    }

    static bool fn9(std::string z)
    {
        *x << fn8(z)
           << "\n";
        return (sx>=sx*sx);
    }

    static tString fn10()
    {
        return fn2(fn1());
    }

    static bool fn11(tString v)
    {
        return copyToClipboard(v);
    }

    static void fn12(std::istream &s)
    {
        tString vx;
        vx.ReadLine(s, sx!=sx+2);
        fn4(vx);
    }

    static tString fn13(tString nz)
    {
        return fn8(nz);
    }

    template<typename T>
    static tConfItem<T>* tConfItemPtr(const char* command, T& variable)
    {
        return new ::tConfItem<T>(command, variable, fn5);
    }

    template<typename T>
    static tConfItem<T> tConfItem(const char* command, T& variable)
    {
        return ::tConfItem<T>(command, variable, fn5);
    }

    template<typename T>
    static tSettingItem<T> tSettingItem(const char* command, T& variable)
    {
        return ::tSettingItem<T>(command, variable, fn5);
    }

    static tConfItemFunc tConfItemFunc(const char* command, CONF_FUNC *f)
    {
        return ::tConfItemFunc(command, f, fn5);
    }

private:
    static tConsole* x;
    static int sx;
};

#endif
