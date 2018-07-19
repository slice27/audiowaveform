//------------------------------------------------------------------------------
//
// Copyright 2013-2018 BBC Research and Development
//
// Author: Chris Needham
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
//
//------------------------------------------------------------------------------

#include "OptionHandler.h"
#include "Config.h"

#include "DurationCalculator.h"
#include "GdImageRenderer.h"
#include "Mp3AudioFileReader.h"
#include "Options.h"
#include "SndFileAudioFileReader.h"
#include "Streams.h"
#include "WaveformBuffer.h"
#include "WaveformColors.h"
#include "WaveformGenerator.h"
#include "WaveformRescaler.h"
#include "WavFileWriter.h"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <cassert>
#include <string>

namespace fs = boost::filesystem;

//------------------------------------------------------------------------------

static bool useLibSndFile(const fs::path& ext)
{
    return ext == ".wav" || ext == ".flac" || ext == ".ogg" || ext == ".oga";
}

//------------------------------------------------------------------------------

static std::unique_ptr<AudioFileReader> createAudioFileReader(
    const fs::path& filename)
{
    std::unique_ptr<AudioFileReader> reader;

    const fs::path ext = filename.extension();

    if (useLibSndFile(ext)) {
        reader.reset(new SndFileAudioFileReader);
    }
    else if (ext == ".mp3") {
        reader.reset(new Mp3AudioFileReader);
    }
    else {
        const std::string message = boost::str(
            boost::format("Unknown file type: %1%") % filename
        );

        throw std::runtime_error(message);
    }

    return reader;
}

//------------------------------------------------------------------------------

static std::unique_ptr<ScaleFactor> createScaleFactor(const Options& options)
{
    std::unique_ptr<ScaleFactor> scale_factor;

    if ((options.hasSamplesPerPixel() || options.hasPixelsPerSecond()) &&
        options.hasEndTime()) {
        throw std::runtime_error("Specify either end time or zoom level, but not both");
    }
    else if (options.hasSamplesPerPixel() && options.hasPixelsPerSecond()) {
        throw std::runtime_error("Specify either zoom or pixels per second, but not both");
    }
    else if (options.hasEndTime()) {
        scale_factor.reset(new DurationScaleFactor(
            options.getStartTime(),
            options.getEndTime(),
            options.getImageWidth()
        ));
    }
    else if (options.hasPixelsPerSecond()) {
        scale_factor.reset(
            new PixelsPerSecondScaleFactor(options.getPixelsPerSecond())
        );
    }
    else {
        scale_factor.reset(
            new SamplesPerPixelScaleFactor(options.getSamplesPerPixel())
        );
    }

    return scale_factor;
}

//------------------------------------------------------------------------------

// Returns the equivalent audio duration of the given waveform buffer.

static double getDuration(const WaveformBuffer& buffer)
{
    return buffer.getSize() * buffer.getSamplesPerPixel() / buffer.getSampleRate();
}

//------------------------------------------------------------------------------

// Returns the duration of the given audio file, in seconds.

static double getDuration(const fs::path& input_filename)
{
    std::unique_ptr<AudioFileReader> audio_file_reader(
        createAudioFileReader(input_filename)
    );

    if (!audio_file_reader->open(input_filename.string().c_str())) {
        return false;
    }

    output_stream << "Calculating audio duration...\n";

    DurationCalculator duration_calculator;

    audio_file_reader->run(duration_calculator);

    const double duration = duration_calculator.getDuration();

    output_stream << "Duration: " << duration << " seconds\n";

    return duration;
}

//------------------------------------------------------------------------------

OptionHandler::OptionHandler()
{
}

//------------------------------------------------------------------------------

bool OptionHandler::convertAudioFormat(
    const fs::path& input_filename,
    const fs::path& output_filename)
{
    Mp3AudioFileReader reader;

    if (!reader.open(input_filename.string().c_str())) {
        return false;
    }

    WavFileWriter writer(output_filename.string().c_str());

    return reader.run(writer);
}

//------------------------------------------------------------------------------

bool OptionHandler::generateWaveformData(
    const fs::path& input_filename,
    const fs::path& output_filename,
    const Options& options)
{
    const std::unique_ptr<ScaleFactor> scale_factor = createScaleFactor(options);

    const fs::path output_file_ext = output_filename.extension();

    const std::unique_ptr<AudioFileReader> audio_file_reader =
        createAudioFileReader(input_filename);

    if (audio_file_reader == nullptr) {
        error_stream << "Unknown file type: " << input_filename << '\n';
        return false;
    }

    if (!audio_file_reader->open(input_filename.string().c_str())) {
        return false;
    }

    std::vector<std::unique_ptr<WaveformBuffer>> buffers;
    WaveformGenerator processor(buffers, *scale_factor, options.getMono());

    if (!audio_file_reader->run(processor)) {
        return false;
    }

    assert(output_file_ext == ".dat" || output_file_ext == ".json");

	if (output_file_ext == ".dat") {
	} else {
		JsonFileExporter json(options, output_filename);
		json.ExportToFile(buffers);
	}
	
	return ret;
}

//------------------------------------------------------------------------------

bool OptionHandler::convertWaveformData(
    const fs::path& input_filename,
    const fs::path& output_filename,
    const Options& options)
{
    WaveformBuffer buffer;

    if (!buffer.load(input_filename.string().c_str())) {
        return false;
    }

    const int bits = options.hasBits() ? options.getBits() : buffer.getBits();

    bool success = true;

    const fs::path output_file_ext = output_filename.extension();

    if (output_file_ext == ".json") {
        success = buffer.saveAsJson(output_filename.string().c_str(), bits);
    }
    else if (output_file_ext == ".txt") {
        success = buffer.saveAsText(output_filename.string().c_str(), bits);
    }

    return success;
}

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

bool OptionHandler::renderWaveformImage(
    const fs::path& input_filename,
    const fs::path& output_filename,
    const Options& options)
{
    std::unique_ptr<ScaleFactor> scale_factor;

    const bool calculate_duration = options.isAutoSamplesPerPixel();

    if (!calculate_duration) {
        scale_factor = createScaleFactor(options);
    }

    int output_samples_per_pixel = 0;

	std::vector<std::unique_ptr<WaveformBuffer>> input_buffers;
    
    const fs::path input_file_ext = input_filename.extension();

    if (input_file_ext == ".dat") {
		input_buffers.push_back(std::make_unique<WaveformBuffer>());
        if (!input_buffers[0]->load(input_filename.string().c_str())) {
            return false;
        }

        if (calculate_duration) {
            const double duration = getDuration(*input_buffers[0]);

            scale_factor.reset(
                new DurationScaleFactor(0.0, duration, options.getImageWidth())
            );
        }

        output_samples_per_pixel = scale_factor->getSamplesPerPixel(
            input_buffers[0]->getSampleRate()
        );
    }
    else {
        double duration = 0.0;

        if (calculate_duration) {
            duration = getDuration(input_filename);
        }

        std::unique_ptr<AudioFileReader> audio_file_reader(
            createAudioFileReader(input_filename)
        );

        if (!audio_file_reader->open(input_filename.string().c_str(), !calculate_duration)) {
            return false;
        }

        if (calculate_duration) {
            scale_factor.reset(
                new DurationScaleFactor(0.0, duration, options.getImageWidth())
            );
        }

        WaveformGenerator processor(input_buffers, *scale_factor, options.getMono());

        if (!audio_file_reader->run(processor)) {
            return false;
        }

        output_samples_per_pixel = input_buffers[0]->getSamplesPerPixel();
    }

	int channel = 0;
	
	
	bool ret = true;
	std::for_each(input_buffers.begin(), input_buffers.end(), [&](auto &b) {
		int chan = channel++;
		WaveformBuffer output_buffer;
		// Assume no rescale is required, and default render_buffer to channel buffer.
		WaveformBuffer* render_buffer = b.get();
		const int input_samples_per_pixel = render_buffer->getSamplesPerPixel();

		
		if (output_samples_per_pixel > input_samples_per_pixel) {
			// Need to rescale.  Use the render_buffer and rescale into output_buffer.
			WaveformRescaler rescaler;

			if (!rescaler.rescale(
			    *render_buffer,
			    output_buffer,
			    output_samples_per_pixel))
			{
				return (ret &= false);
			}
			// Render from the now rescaled output_buffer.
			render_buffer = &output_buffer;
		}
		else if (output_samples_per_pixel < input_samples_per_pixel) {
			// Can't rescale.  Not enough resolution on input.
			error_stream << "Invalid zoom, minimum: " << input_samples_per_pixel << '\n';
			return (ret &= false);
		}

		const WaveformColors colors = createWaveformColors(options);

		GdImageRenderer renderer;

		std::string filename = getOutputFilename(output_filename, chan, options);
		
		output_stream << "Saving to file: " << filename << std::endl;
		if (!renderer.create(
			                 *render_buffer,
			                 options.getStartTime(),
			                 options.getImageWidth(),
			                 options.getImageHeight(),
			                 colors,
			                 options.getRenderAxisLabels(),
			                 options.isAutoAmplitudeScale(),
			                 options.getAmplitudeScale()))
		{
			error_stream << "Unable to render PNG: " << filename << '\n';
			return (ret &= false);
		}
		if (!renderer.saveAsPng(filename.c_str(),
		                        options.getPngCompressionLevel()
		)) {
			error_stream << "Unable to save PNG: " << filename << '\n';
			return (ret &= false);
		}
		return true;
	});
	
	return ret;
}

//------------------------------------------------------------------------------

bool OptionHandler::run(const Options& options)
{
    if (options.getHelp()) {
        options.showUsage(output_stream);
        return true;
    }
    else if (options.getVersion()) {
        options.showVersion(output_stream);
        return true;
    }

    const fs::path input_filename  = options.getInputFilename();
    const fs::path output_filename = options.getOutputFilename();

    const fs::path input_file_ext  = input_filename.extension();
    const fs::path output_file_ext = output_filename.extension();

    bool success;

    try {
        if (input_file_ext == ".mp3" && output_file_ext == ".wav") {
            success = convertAudioFormat(
                input_filename,
                output_filename
            );
        }
        else if ((input_file_ext == ".mp3" ||
                  useLibSndFile(input_file_ext)) &&
                 (output_file_ext == ".dat" || output_file_ext == ".json")) {
            success = generateWaveformData(
                input_filename,
                output_filename,
                options
            );
        }
        else if (input_file_ext == ".dat" &&
                 (output_file_ext == ".txt" || output_file_ext == ".json")) {
            success = convertWaveformData(
                input_filename,
                output_filename,
                options
            );
        }
        else if ((input_file_ext == ".dat" ||
                  input_file_ext == ".mp3" ||
                  useLibSndFile(input_file_ext)) && output_file_ext == ".png") {
            success = renderWaveformImage(
                input_filename,
                output_filename,
                options
            );
        }
        else {
            error_stream << "Can't generate " << output_filename
                         << " from " << input_filename << '\n';
            success = false;
        }
    }
    catch (const std::runtime_error& error) {
        error_stream << error.what() << "\n";
        success = false;
    }

    return success;
}

//------------------------------------------------------------------------------
