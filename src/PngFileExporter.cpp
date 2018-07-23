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

#include "PngFileExporter.h"
#include "WaveformColors.h"
#include "Streams.h"
#include "WaveformBuffer.h"
#include "GdImageRenderer.h"
#include "WaveformRescaler.h"
#include "Utils.h"

#include <boost/format.hpp>

//------------------------------------------------------------------------------

static WaveformColors createWaveformColors(const Options& options)
{
    WaveformColors colors;

    const std::string& color_scheme = options.getColorScheme();

    if (color_scheme == "audacity") {
        colors = audacity_waveform_colors;
    }
    else if (color_scheme == "audition") {
        colors = audition_waveform_colors;
    }
    else {
        std::string message = boost::str(
            boost::format("Unknown color scheme: %1%") % color_scheme
        );

        throw std::runtime_error(message);
    }

    if (options.hasBorderColor()) {
        colors.border_color = options.getBorderColor();
    }

    if (options.hasBackgroundColor()) {
        colors.background_color = options.getBackgroundColor();
    }

    if (options.hasWaveformColor()) {
        colors.waveform_color = options.getWaveformColor();
    }

    if (options.hasAxisLabelColor()) {
        colors.axis_label_color = options.getAxisLabelColor();
    }

    return colors;
}

//------------------------------------------------------------------------------

PngFileExporter::PngFileExporter(WaveformBuffer& buffer,
                                 const Options &options,
                                 const fs::path& output_filename, 
                                 const int output_samples_per_pixel):
	FileExporter(buffer, options, output_filename),
	output_samples_per_pixel_(output_samples_per_pixel)
{
}

void PngFileExporter::writeFile(std::ofstream& stream)
{
	UNUSED(stream);
	// TODO: GdImageRenderer.cpp must be modified to handle the new WaveformBuffer
	// object, and draw the appropriate image.  This is beyond my scope of work,
	// and thus, I am unable to complete this.
	std::vector<WaveformBuffer> buffers = buffer_.SplitChannels();
	// Force output to VERSION_1.  This can be removed when GdImageRenderer is fixed;
	// The SplitChannels that was added to WaveformBuffer can be removed as well.
	const_cast<Options&>(options_).setFileVersion(FileExporter::VERSION_1);
	
	int channel = 0;
	std::for_each(buffers.begin(), buffers.end(), [&](auto &b) {
		int chan = channel++;
		std::string filename = getOutputFilename(output_filename_, chan);
		output_stream << "Saving to file: " << filename << std::endl;
		WaveformBuffer output_buffer;
		const WaveformColors colors = createWaveformColors(options_);
		GdImageRenderer renderer;
		
		const int input_samples_per_pixel = b.getSamplesPerPixel();

		// Assume no rescale is required, and default render_buffer to channel buffer.
		WaveformBuffer* render_buffer = &b;
		
		if (output_samples_per_pixel_ > input_samples_per_pixel) {
			// Need to rescale.  Use the render_buffer and rescale into output_buffer.
			WaveformRescaler rescaler;

			if (!rescaler.rescale(
			    *render_buffer,
			    output_buffer,
			    output_samples_per_pixel_))
			{
				throwError("PngFileExporter::writeFile", 
				           "Unable to rescale render buffer.", filename);
			}
			// Render from the now rescaled output_buffer.
			render_buffer = &output_buffer;
		}
		else if (output_samples_per_pixel_ < input_samples_per_pixel) {
			// Can't rescale.  Not enough resolution on input.
			throwError("PngFileExporter::writeFile", "Invalid zoom, minimum: " + 
			           std::to_string(input_samples_per_pixel), filename);
		}
		
		
		if (!renderer.create(
			                 *render_buffer,
			                 options_.getStartTime(),
			                 options_.getImageWidth(),
			                 options_.getImageHeight(),
			                 colors,
			                 options_.getRenderAxisLabels(),
			                 options_.isAutoAmplitudeScale(),
			                 options_.getAmplitudeScale()))
		{
			throwError("PngFileExporter::writeFile",
			           "Unable to render PNG.", filename);
		}
		if (!renderer.saveAsPng(filename.c_str(),
		                        options_.getPngCompressionLevel())) {
			throwError("PngFileExporter::writeFile",
			           "Unable to save PNG.", filename);
		}
	});
}
