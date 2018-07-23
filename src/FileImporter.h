//------------------------------------------------------------------------------
//
// Copyright 2013-2018 BBC Research and Development
//
// Author: Chris Chaffey
//
// This file is part of Audio Waveform Image Generator.
//
// Audio Waveform Image Generator is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation, either version 3 of the License,
// or (at your option) any later version.
//
// Audio Waveform Image Generator is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// Audio Waveform Image Generator.  If not, see <http://www.gnu.org/licenses/>.

#if !defined(INC_FILE_IMPORTER_H)
#define INC_FILE_IMPORTER_H

#include <boost/filesystem.hpp>
#include "FileExporter.h"

namespace fs = boost::filesystem;

class FileImporter
{
	public:
		~FileImporter() = default;
		
		bool ImportFromFile();
		
	protected:
		FileImporter(WaveformBuffer &buffer,
		             const Options &options,
		             const fs::path& input_filename);

		virtual void readFile(std::ifstream& stream) = 0;

		bool openFile(std::ifstream& stream);
		void closeFile(std::ifstream& stream);

		WaveformBuffer& buffer_;
		const Options& options_;
		const fs::path& input_filename_;
};

#endif