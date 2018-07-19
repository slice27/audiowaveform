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

namespace fs = boost::filesystem;

FileExporter::FileExporter(const Options &options,
                           const boost::filesystem::path& output_filename) :
		    bits_(options.getBits()),
			version_(static_cast<FILE_VERSION>(options.getFileVersion())),
		    options_(options),
			output_filename_(output_filename)
{
	
}
			
bool FileExporter::ExportToFile(std::vector<std::unique_ptr<WaveformBuffer>> &buffers)
{
	bool ret = true;
	try {
		if (bits_ != 8 && bits_ != 16) {
			throw std::runtime_error("Invalid bits: must be either 8 or 16");
			return false;
		}

		std::ofstream file;
		int size = buffers[0]->getSize();
		file.exceptions(std::ios::badbit | std::ios::failbit);

		uint32_t chan = 0;
		uint32_t num_chans = static_cast<uint32_t>(buffers.size());
		if (version_ == VERSION_2) {
			writeHeader(file, chan, num_chans, size, buffers[0]->getSampleRate(),
			            buffers[0]->getSamplesPerPixel());
		}

		for_each(buffers.begin(), buffers.end(), [&](auto &buf) {
			if (version_ == VERSION_1) {
				writeHeader(file, chan, num_chans, size, buf->getSampleRate(), 
				                buf->getSamplesPerPixel());
			}
			writeChannel(file, buf.get(), chan++);
			
			if (version_ == VERSION_1) {
				writeFooter(file);
			}
		});
		if (version_ == VERSION_2) {
			writeFooter(file);
		}	
		
	} catch (const std::exception& e) {
		error_stream << e.what() << std::endl;
		ret = false;
	}
	return ret;
}

std::string FileExporter::getOutputFilename(const boost::filesystem::path& output_filename, 
                                            int chan_num) {
	fs::path fn = output_filename;
	if (!options_.getMono() && (version_ == VERSION_1)) {
		// If this isn't a mono waveform, but writing as a version 1 file, then append
		// the channel number to the filename.
		fs::path ext = fn.extension();
		std::string chan_fn = fn.filename().replace_extension("").string();
		fn.remove_filename().append(chan_fn + "-chan" + std::to_string(chan_num) + ext.string());
	}
	return fn.string();
}
