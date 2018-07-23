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

#include <boost/filesystem.hpp>

class WaveformBuffer;
class Options;

class FileExporter
{
	public:
		~FileExporter() = default;
		
		bool ExportToFile();

		typedef enum {
			VERSION_1 = 1U,
			VERSION_2
		} FILE_VERSION;
 
	protected:
		FileExporter(WaveformBuffer& buffer,
		             const Options &options,
					 const boost::filesystem::path& output_filename);

		std::string getOutputFilename(const boost::filesystem::path& output_filename, 
                                      int chan_num);
		
		bool openFile(std::ofstream& stream, int chan, std::string& filename);
		void closeFile(std::ofstream& stream);
		
		virtual void writeFile(std::ofstream& stream) = 0;

		WaveformBuffer &buffer_;
		const Options &options_;
		const boost::filesystem::path& output_filename_;
};

#endif