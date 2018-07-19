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
#include <memory>
#include <boost/filesystem.hpp>

class WaveformBuffer;
class Options;

class FileExporter
{
	public:
		~FileExporter() = default;
		
		bool ExportToFile(std::vector<std::unique_ptr<WaveformBuffer>> &buffers);

		typedef enum {
			VERSION_1 = 1U,
			VERSION_2
		} FILE_VERSION;
 
	protected:
		FileExporter(const Options &options,
					 const boost::filesystem::path& output_filename);
			
		std::string getOutputFilename(const boost::filesystem::path& output_filename, 
                                      int chan_num);
		
		virtual void writeHeader(std::ofstream& stream,
		                         const std::uint32_t chan,
		                         const std::uint32_t size,
		                         const std::uint32_t sample_rate_,
		                         const std::uint32_t samples_per_pixel_) = 0;

		virtual void writeChannel(std::ostream &stream,
		                  WaveformBuffer *data,
		                  const std::uint32_t chan_num) = 0;

		virtual void writeFooter(std::ofstream& stream) = 0;

		std::uint32_t bits_;
		FILE_VERSION version_;
		const Options &options_;
		const boost::filesystem::path& output_filename_;
};

#endif