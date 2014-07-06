/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include "prdownloader.h"

#include <lslutils/globalsmanager.h>
#include "lib/src/Downloader/IDownloader.h"
#include "lib/src/FileSystem/FileSystem.h"
#include "lib/src/FileSystem/File.h"
#include "utils/uievents.h"
#include "utils/conversion.h"
#include "utils/globalevents.h"
#include "utils/slpaths.h"
#include "mainwindow.h"
#include "downloadsobserver.h"
#include <list>

#include <wx/log.h>
#include <lslunitsync/unitsync.h>
#include <lslutils/thread.h>
#include <settings.h>
#include "helper/slconfig.h"


SLCONFIG("/Spring/PortableDownload", false, "true to download portable versions of spring, if false cache/settings/etc are shared (bogous!)");
SLCONFIG("/Spring/RapidMasterUrl", "http://repos.springrts.com/repos.gz", "master url for rapid downloads");

std::string PrDownloader::GetEngineCat()
{
#ifdef WIN32
	return "engine_windows";
#elif defined(__APPLE__)
	return "engine_macosx";
#elif defined(__x86_64__)
	return "engine_linux64";
#else
	return "engine_linux";
#endif
}

class DownloadItem : public LSL::WorkItem
{
public:
	DownloadItem( std::list<IDownload*> item, IDownloader* loader)
		: m_item(item)
		, m_loader(loader)
	{}

	void Run() {
		if (!m_item.empty()) {
			UiEvents::ScopedStatusMessage msg("Downloading: " + m_item.front()->name, 0);
			//we create this in avance cause m_item gets freed
			wxString d(_("Download complete: "));
			d += TowxString(m_item.front()->name);
			m_loader->download( m_item, sett().GetHTTPMaxParallelDownloads() );
			std::list<IDownload*>::iterator it;
			for( it = m_item.begin(); it!=m_item.end(); ++it) {
				IDownload* dl = *it;
				switch(dl->cat) {
				case IDownload::CAT_ENGINE_LINUX:
				case IDownload::CAT_ENGINE_WINDOWS:
				case IDownload::CAT_ENGINE_LINUX64:
				case IDownload::CAT_ENGINE_MACOSX: {
					fileSystem->extractEngine(dl->name, dl->version);
					SlPaths::RefreshSpringVersionList(); //FIXME: maybe not thread-save!
					SlPaths::SetUsedSpringIndex(dl->version);
					break;
				}
				default:
					continue;
				}
			}
			m_loader->freeResult( m_item );
			UiEvents::ScopedStatusMessage msgcomplete(d, 0);
			LSL::usync().ReloadUnitSyncLib();
			// prefetch map data after a download as well
			for( it = m_item.begin(); it!=m_item.end(); ++it) {
				IDownload* dl = *it;
				switch(dl->cat) {
					case IDownload::CAT_MAPS: {
						LSL::usync().PrefetchMap(dl->name); //FIXME: do the same for games, too
					}
					default:
						continue;
				}
			}

			//notify about finished download
			GlobalEvent::Send(GlobalEvent::OnUnitsyncReloaded);
		}
	}

private:
	std::list<IDownload*> m_item;
	IDownloader* m_loader;
};

class SearchItem : public LSL::WorkItem
{
public:
	SearchItem(std::list<IDownloader*> loaders, std::string name, IDownload::category cat);
	void Run();

private:
	const std::list<IDownloader*> m_loaders;
	const std::string m_name;
	const IDownload::category m_cat;
	int m_result_size;
};

SearchItem::SearchItem(std::list<IDownloader*> loaders, const std::string name, IDownload::category cat)
	: m_loaders(loaders)
	, m_name(name)
	, m_cat(cat)
	, m_result_size(0)
{}

void SearchItem::Run()
{
	std::list<IDownload*> results;
	std::list<IDownloader*>::const_iterator it = m_loaders.begin();
	for( ; it != m_loaders.end(); ++it ) {
		(*it)->search(results, m_name, m_cat);
		if (!results.empty()) {
			DownloadItem* dl_item = new DownloadItem(results, *it);
			prDownloader().m_dl_thread->DoWork(dl_item);
			m_result_size = results.size();
			return;
		}
	}
	return;
}


PrDownloader::PrDownloader():
	wxEvtHandler(),
	m_dl_thread(new LSL::WorkerThread())
{
	IDownloader::Initialize(&downloadsObserver());
	UpdateSettings();
	m_game_loaders.push_back(rapidDownload);
	m_game_loaders.push_back(httpDownload);
	m_game_loaders.push_back(plasmaDownload);
	m_map_loaders.push_back(httpDownload);
	m_map_loaders.push_back(plasmaDownload);
	ConnectGlobalEvent(this, GlobalEvent::OnSpringStarted, wxObjectEventFunction(&PrDownloader::OnSpringStarted));
	ConnectGlobalEvent(this, GlobalEvent::OnSpringTerminated, wxObjectEventFunction(&PrDownloader::OnSpringTerminated));
}

PrDownloader::~PrDownloader()
{
	delete m_dl_thread;
	IDownloader::Shutdown();
}

void PrDownloader::ClearFinished()
{
}

void PrDownloader::UpdateSettings()
{
	fileSystem->setWritePath(SlPaths::GetDownloadDir());
	fileSystem->setEnginePortableDownload(cfg().ReadBool(_T("/Spring/PortableDownload")));
	rapidDownload->setOption("masterurl", STD_STRING(cfg().ReadString(_T("/Spring/RapidMasterUrl"))));
}

void PrDownloader::RemoveTorrentByName(const std::string &/*name*/)
{
}

int PrDownloader::GetDownload(const std::string& category, const std::string &name)
{
	if (category == "map") {
		return Get(m_map_loaders, name, IDownload::CAT_MAPS);
	} else if (category == "game") {
		return Get(m_game_loaders, name, IDownload::CAT_GAMES);
	} else if (category == "engine_linux") {
		return Get(m_map_loaders, name, IDownload::CAT_ENGINE_LINUX);
	} else if (category == "engine_linux64") {
		return Get(m_map_loaders, name, IDownload::CAT_ENGINE_LINUX64);
	} else if (category == "engine_windows") {
		return Get(m_map_loaders, name, IDownload::CAT_ENGINE_WINDOWS);
	} else if (category == "engine_macosx") {
		return Get(m_map_loaders, name, IDownload::CAT_ENGINE_MACOSX);
	}
	wxLogError(_T("Category %s not found"), category.c_str());
	return -1;
}

void PrDownloader::OnSpringStarted(wxCommandEvent& /*data*/)
{
	//FIXME: pause downloads
}

void PrDownloader::OnSpringTerminated(wxCommandEvent& /*data*/)
{
	//FIXME: resume downloads
}

int PrDownloader::Get(std::list<IDownloader*> loaders, const std::string &name, IDownload::category cat)
{
	SearchItem* searchItem = new SearchItem(loaders, name, cat);
	m_dl_thread->DoWork(searchItem);
	return 1;
}

PrDownloader& prDownloader()
{
	static LSL::Util::LineInfo<PrDownloader> m( AT );
	static LSL::Util::GlobalObjectHolder<PrDownloader, LSL::Util::LineInfo<PrDownloader> > s_PrDownloader( m );
	return s_PrDownloader;
}

bool PrDownloader::IsRunning()
{
	return !downloadsObserver().IsEmpty();
}
