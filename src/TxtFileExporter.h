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

#if !defined(INC_TXT_FILE_EXPORTER_H)
#define INC_TXT_FILE_EXPORTER_H

#include "FileExporter.h"

class TxtFileExporter: public FileExporter
{
	public:
		TxtFileExporter(WaveformBuffer &buffer,
		                const Options &options,
						const fs::path& output_filename);
		~TxtFileExporter() = default;
		
		TxtFileExporter() = delete;
		TxtFileExporter(TxtFileExporter &&) = delete;
		TxtFileExporter(const TxtFileExporter &) = delete;
		TxtFileExporter& operator=(const TxtFileExporter &) = delete;

	private:
	    void writeFile(std::ofstream& stream);

		void writeData(std::ofstream& stream, int chan, size_t len, FILE_VERSION version);
		int bits_;
};

#endif