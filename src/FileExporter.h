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

#if !defined(INC_FILE_EXPORTER_H)
#define INC_FILE_EXPORTER_H

#include <vector>

class WaveformBuffer;

class FileExporter
{
	public:
		~FileExporter();
		
		bool ExportToFile(std::vector<std::unique_ptr<WaveformBuffer>> &buffers, 
		                  std::string filename) = 0;

		constexpr enum {
			VERSION_1 = 1U,
			VERSION_2
		} FILE_VERSION;
 
	protected:
		FileExporter(const Options &options,
					 const std::string output_filename) :
		    bits_(options.getBits()),
		    options_(options),
			output_filename_(output_filename),
		    version_(options.getFileVersion()) {}
			
		std::string OptionHandler::getOutputFilename(const boost::filesystem::path& output_filename, 
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

		std::uint32_t bits_;
		FILE_VERSION version_;
		const Options &options_;
		std::string output_filename_;
}

#endif