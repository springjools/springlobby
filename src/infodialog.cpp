/**
    This file is part of SpringLobby,
    Copyright (C) 2007-2010

    SpringLobby is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as published by
    the Free Software Foundation.

    springsettings is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SpringLobby.  If not, see <http://www.gnu.org/licenses/>.
**/


#include "infodialog.h"

#include <wx/sizer.h>
#include "springunitsync.h"
#include "settings.h"
#include <vector>
#include <fstream>
#include <utility>
#include <wx/filename.h>
#include "nonportable.h"
#include "utils/conversion.h"
#include "utils/debug.h"
#include "utils/math.h"
#include "utils/platform.h"
#include <wx/textctrl.h>
#ifdef __unix__
# include <unistd.h>
# define WRITABLE W_OK
#elif __WIN32__ || _MSC_VER
# include <io.h>
# define WRITABLE 02
#else
# error "os not supported"
#endif

inline wxString BtS( bool y ) { return y ? wxString(_T("Yes")) : wxString(_T("No")) ; }

InfoDialog::InfoDialog(wxWindow* parent )
	:wxDialog(parent,wxID_ANY, _("path shit"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX)
{
	wxBoxSizer* main_sizer = new wxBoxSizer( wxVERTICAL );

	typedef  std::vector< std::pair< wxString,wxString > >
		Paths;
	Paths paths;
	paths.push_back( std::make_pair( sett().GetLobbyWriteDir(), _T("LobbyWriteDir") ) );
	paths.push_back( std::make_pair( sett().GetTempStorage(), _T("TempStorage")) );
	paths.push_back( std::make_pair( sett().GetCachePath(), _T("CachePath")) );
	paths.push_back( std::make_pair( sett().GetCurrentUsedDataDir(), _T("CurrentUsedDataDir")) );
	paths.push_back( std::make_pair( GetExecutableFolder() , _T("ExecutableFolder")));
//	paths.push_back( std::make_pair( , _T("")));
//	paths.push_back( std::make_pair( , _T("")));
//	paths.push_back( std::make_pair( , _T("")));

	wxTextCtrl* out = new wxTextCtrl( this, wxNewId(), _T( "" ), wxDefaultPosition, wxDefaultSize,
									 wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH | wxTE_AUTO_URL );
	for ( size_t i =0; i < paths.size(); ++i )
	{
		*out << wxString::Format( _T("%s (%s)\n"), paths[i].second.c_str(), paths[i].first.c_str());
		wxString path = paths[i].first;
		wxString dummy_fn = path + wxFileName::GetPathSeparator() + _T("dummy.txt");
		const bool wx = wxFileName::IsDirWritable( path );
		const bool posix = access(STD_STRING(path).c_str(), WRITABLE) == 0;
		bool tried = false;
		try{
			std::ofstream of;
			of.open( STD_STRING(dummy_fn).c_str() );

			if ( of.is_open() )
			{
				of << "fhreuohgeiuhguie";
				of.flush();
				of.close();
				tried = wxRemoveFile(dummy_fn);
			}
		}
		catch (...){}
		*out << wxString::Format( _T("\tWX: %s POSIX: %s TRY: %s\n"), BtS(wx).c_str(), BtS(posix).c_str(), BtS(tried).c_str() );
	}
	*out << wxString::Format( _T("Portable mode: %s\n"), BtS(sett().IsPortableMode()).c_str() );
	main_sizer->Add( out, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	SetSizer( main_sizer );
	Layout();
}

