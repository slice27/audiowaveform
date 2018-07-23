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

#if !defined(INC_DAT_FILE_IMPORTER_H)
#define INC_DAT_FILE_IMPORTER_H

#include "FileImporter.h"
#include "FileExporter.h"

class DatFileImporter: public FileImporter
{
	public:
		DatFileImporter(WaveformBuffer &buffer,
		                const Options &options,
		                const fs::path& input_filename);
		~DatFileImporter() = default;
		
		DatFileImporter() = delete;
		DatFileImporter(DatFileImporter &&) = delete;
		DatFileImporter(const DatFileImporter &) = delete;
		DatFileImporter& operator=(const DatFileImporter &) = delete;

	private:
		void readFile(std::ifstream& stream);

		void readHeader(std::ifstream& stream);
		void readData(std::ifstream& stream);

		FileExporter::FILE_VERSION version_;
		uint32_t channels_;
		int32_t size_;
};

#endif