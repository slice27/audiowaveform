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

#include "TxtFileExporter.h"
#include "Streams.h"
#include "WaveformBuffer.h"

TxtFileExporter::TxtFileExporter(const Options &options,
						 const boost::filesystem::path& output_filename):
	FileExporter(options, output_filename)
{
}
	
void TxtFileExporter::writeHeader(std::ofstream& stream,
                                  const std::uint32_t chan,
                                  const std::uint32_t num_chans,
                                  const std::uint32_t size,
                                  const std::uint32_t sample_rate,
                                  const std::uint32_t samples_per_pixel)
{
	if (chan == num_chans == size == sample_rate == samples_per_pixel) {
		output_stream << "Weirdness!" << std::endl;
	}
	std::string filename = getOutputFilename(output_filename_, chan);
	output_stream << "Writing output file: " << filename << std::endl;
	stream.open(filename);
}

void TxtFileExporter::writeChannel(std::ostream &stream,
                                   WaveformBuffer *data,
                                   const std::uint32_t chan_num)
{
	const int size = data->getSize();

	if (bits_ == 8) {
		for (int i = 0; i < size; ++i) {
			const int min_value = data->getMinSample(i) / 256;
			const int max_value = data->getMaxSample(i) / 256;

			stream << min_value << ',' << max_value;
			if (VERSION_2 == version_) {
				stream << ',' << chan_num;
			}
			stream << std::endl;
		}
	}
	else {
		for (int i = 0; i < size; ++i) {
			stream << data->getMinSample(i) << ','
				   << data->getMaxSample(i) << '\n';
		}
	}
}

void TxtFileExporter::writeFooter(std::ofstream& stream)
{
	stream.flush();
	stream.close();
}
