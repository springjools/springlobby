#ifndef SL_PLAYBACKLIST_H_INCLUDED
#define SL_PLAYBACKLIST_H_INCLUDED

#include <map>


class wxArrayString;

template <class PlaybackImp>
class PlaybackList : public wxEvtHandler
{
  public:
    typedef PlaybackImp
        PlaybackType;

    typedef unsigned int playback_id_t;

    //! @brief mapping from playback id number to playback object
    typedef std::map<playback_id_t, PlaybackType> playback_map_t;
    //! @brief iterator for playback map
    typedef typename playback_map_t::iterator playback_iter_t;
    //! @brief const iterator for playback map
    typedef typename playback_map_t::const_iterator playback_const_iter_t;

    virtual void LoadPlaybacks( const wxArrayString& filenames ) = 0;
    //!loads replays between two indices
//    virtual void LoadPlaybacks( const unsigned int from, const unsigned int to) = 0;

    PlaybackType& AddReplay( const PlaybackType& replay );
    void AddReplay( PlaybackType* replay );
    void RemoveReplay( playback_id_t const& id );

    PlaybackType &GetReplayById( playback_id_t const& id );

    ///Replay& GetReplay( int const index ) ;

    bool ReplayExists( playback_id_t const& id );
    bool DeleteReplay( playback_id_t const& id );
    typename playback_map_t::size_type GetNumReplays();

    void RemoveAll();

    const playback_map_t& GetReplaysMap() const;


  protected:
    PlaybackList() {};

    playback_map_t m_replays;

    unsigned long m_fails;

};

template <class ListImp>
ListImp& playbacklist();

#include "playbacklist.cpp"

#endif // SL_PLAYBACKLIST_H_INCLUDED
