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

#include "FileExporter.h"
#include "Options.h"
#include "WaveformBuffer.h"
#include "Streams.h"
#include "Utils.h"

FileExporter::FileExporter(WaveformBuffer &buffer,
                           const Options &options,
                           const fs::path& output_filename) :
		    buffer_(buffer),
		    options_(options),
			output_filename_(output_filename)
{
}

bool FileExporter::ExportToFile()
{
	try {
		int bits = options_.getBits();
		if (bits != 8 && bits != 16) {
			throwError("FileExporter::ExportToFile", "Invalid bits: must be either 8 or 16");
		}
		
		FILE_VERSION version = static_cast<FILE_VERSION>(options_.getFileVersion());
		if ((FileExporter::VERSION_1 != version) && (FileExporter::VERSION_2 != version)) {
			throwError("FileExporter::ExportToFile", "Unknown file version.  Version: " + 
			           std::to_string(version) + " - Version must be either 1 or 2.");
		}

		std::ofstream file;
		file.exceptions(std::ios::badbit | std::ios::failbit);

		writeFile(file);
		
	} catch (const std::exception& e) {
		throw;
	}
	return true;
}

std::string FileExporter::getOutputFilename(const fs::path& output_filename, 
                                            int chan_num) {
	fs::path fn = output_filename;
	if (!options_.getMono() && (options_.getFileVersion() == VERSION_1)) {
		// If this isn't a mono waveform, but writing as a version 1 file, then append
		// the channel number to the filename.
		fs::path ext = fn.extension();
		std::string chan_fn = fn.filename().replace_extension("").string();
		fn.remove_filename().append(chan_fn + "-chan" + std::to_string(chan_num) + ext.string());
	}
	return fn.string();
}

bool FileExporter::openFile(std::ofstream& stream, int chan, std::string& filename)
{
	if (stream.is_open()) {
		closeFile(stream);
	}
	try {
		filename = getOutputFilename(output_filename_, chan);
		stream.open(filename);
	} catch (std::exception &e) {
		throwError("FileExporter::openFile", e.what(), filename);
	}
	return true;
}

//------------------------------------------------------------------------------

void FileExporter::closeFile(std::ofstream& stream)
{
	if (stream.is_open()) {
		stream.flush();
		stream.close();
	}
}

//------------------------------------------------------------------------------
