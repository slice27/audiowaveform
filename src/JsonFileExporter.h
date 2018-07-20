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

#if !defined(INC_JSON_FILE_EXPORTER_H)
#define INC_JSON_FILE_EXPORTER_H

#include "FileExporter.h"

class JsonFileExporter: public FileExporter
{
	public:
		JsonFileExporter(WaveformBuffer &buffer,
		                 const Options &options,
						 const boost::filesystem::path& output_filename);
		~JsonFileExporter() = default;
		
		JsonFileExporter() = delete;
		JsonFileExporter(JsonFileExporter &&) = delete;
		JsonFileExporter(const JsonFileExporter &) = delete;
		JsonFileExporter& operator=(const JsonFileExporter &) = delete;

	private:
	    void writeHeader(std::ofstream& stream);
		void writeData(std::ofstream &stream);
		void writeFooter(std::ofstream& stream);
		
		void prepareHeader(std::ofstream& stream, int chan, FILE_VERSION version);
		void prepareData(std::ofstream& stream, int chan, FILE_VERSION version);
		void prepareFooter(std::ofstream& stream);

};

#endif