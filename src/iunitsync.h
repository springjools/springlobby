#ifndef SPRINGLOBBY_HEADERGUARD_IUNITSYNC_H
#define SPRINGLOBBY_HEADERGUARD_IUNITSYNC_H

#include <string>
#include <wx/arrstr.h>

class wxImage;

struct UnitSyncMod
{
  UnitSyncMod() : name(""),hash("NULL") { }
  std::string name;
  std::string hash;
};

struct StartPos
{
  int x;
  int y;
};

struct MapInfo
{
  std::string description;
  int tidalStrength;
  int gravity;
  float maxMetal;
  int extractorRadius;
  int minWind;
  int maxWind;

  int width;
  int height;
  int posCount;
  StartPos positions[16];

  std::string author;
};

struct UnitSyncMap
{
  UnitSyncMap() : name(""),hash("NULL") { }
  std::string name;
  std::string hash;
  MapInfo info;
};

typedef int GameFeature;
enum {
  GF_XYStartPos = 1,
  USYNC_Sett_Handler = 2
};

class IUnitSync
{
  public:
    virtual ~IUnitSync() { }

    virtual int GetNumMods() = 0;
    virtual bool ModExists( const std::string& modname ) = 0;
    virtual UnitSyncMod GetMod( const std::string& modname ) = 0;
    virtual UnitSyncMod GetMod( int index ) = 0;
    virtual int GetModIndex( const std::string& name ) = 0;
    virtual std::string GetModArchive( int index ) = 0;

    virtual int GetNumMaps() = 0;
    virtual bool MapExists( const std::string& mapname ) = 0;
    virtual bool MapExists( const std::string& mapname, const std::string hash ) = 0;

    virtual UnitSyncMap GetMap( const std::string& mapname ) = 0;
    virtual UnitSyncMap GetMap( int index ) = 0;
    virtual UnitSyncMap GetMapEx( const std::string& mapname ) = 0;
    virtual UnitSyncMap GetMapEx( int index ) = 0;

    virtual int GetMapIndex( const std::string& name ) = 0;
    virtual wxImage GetMinimap( const std::string& mapname, int max_w, int max_h, bool store_size = false ) = 0;

    virtual int GetSideCount( const std::string& modname ) = 0;
    virtual std::string GetSideName( const std::string& modname, int index ) = 0;
    virtual wxImage GetSidePicture( const std::string& modname, const std::string& SideName ) =0;

    virtual int GetNumUnits( const std::string& modname ) = 0;
    virtual wxArrayString GetUnitsList( const std::string& modname ) = 0;

    virtual bool LoadUnitSyncLib( const wxString& springdir, const wxString& unitsyncloc ) = 0;
    virtual void FreeUnitSyncLib() = 0;

    virtual bool IsLoaded() = 0;

    virtual std::string GetSpringVersion() = 0;
    virtual bool VersionSupports( GameFeature feature ) = 0;

    virtual wxArrayString GetAIList() = 0;

    virtual bool CacheMapInfo( const wxString& map ) = 0;
    virtual bool CacheMinimap( const wxString& map ) = 0;
    virtual bool CacheModUnits( const wxString& mod ) = 0;
    virtual bool ReloadUnitSyncLib() = 0;

};

IUnitSync* usync();

#endif // SPRINGLOBBY_HEADERGUARD_IUNITSYNC_H
