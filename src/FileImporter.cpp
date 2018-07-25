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

#include "FileImporter.h"
#include "Streams.h"
#include "Options.h"

bool FileImporter::ImportFromFile()
{
	int bits = options_.getBits();
	if (bits != 8 && bits != 16) {
		throwErrorEx("FileImporter::ImportFromFile", "invalid bits - must be either 8 or 16");
	}
	
	FileExporter::FILE_VERSION version = 
		static_cast<FileExporter::FILE_VERSION>(options_.getFileVersion());
	if ((FileExporter::VERSION_1 != version) && 
	    (FileExporter::VERSION_2 != version)) {
		throwErrorEx("FileImporter::ImportFromFile", "Unknown file version.  Version: " + 
		             std::to_string(version) + " - Version must be either 1 or 2");
	}

	std::ifstream file;
	readFile(file);

	return true;
}

FileImporter::FileImporter(WaveformBuffer &buffer,
                           const Options &options,
                           const fs::path& input_filename):
	buffer_(buffer),
	options_(options),
	input_filename_(input_filename)
{
}

bool FileImporter::openFile(std::ifstream& stream, bool binary)
{
	if (stream.is_open()) {
		closeFile(stream);
	}
	try {
		stream.exceptions(std::ios::badbit | std::ios::failbit);
		if (binary) {
			stream.open(input_filename_.string(), std::ios::in);
		} else {
			stream.open(input_filename_.string(),
			            std::ios::in | std::ios::binary);
		}
	} catch (std::exception &e) {
		throwErrorEx("FileImporter::openFile",
		             e.what(), input_filename_.string());
	}
	return true;
}

void FileImporter::closeFile(std::ifstream& stream)
{
	if (stream.is_open()) {
		stream.close();
	}
}
