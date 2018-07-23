//------------------------------------------------------------------------------
//
// Copyright 2013-2014 BBC Research and Development
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

#if !defined(INC_WAVEFORM_BUFFER_H)
#define INC_WAVEFORM_BUFFER_H

//------------------------------------------------------------------------------

#include <vector>
#include <stdexcept>

//------------------------------------------------------------------------------

class WaveformBuffer
{
    public:
		static const uint32_t FLAG_8_BIT = 0x00000001U;
	
		typedef std::vector<short> vector_type;
        typedef vector_type::size_type size_type;

        WaveformBuffer();
		WaveformBuffer(const WaveformBuffer& buffer);
        WaveformBuffer& operator=(const WaveformBuffer& buffer) = delete;

		std::vector<WaveformBuffer> SplitChannels();

    public:
        void setSampleRate(int sample_rate);
		int getSampleRate() const;
		
        void setSamplesPerPixel(int samples_per_pixel);
        int getSamplesPerPixel() const;
		
		void setBits(int bits);
		int getBits() const;

        int32_t getSize(int chan = 0) const;
        void setSize(int32_t size);

        short getMinSample(size_type index, int chan = 0) const;
        short getMaxSample(size_type index, int chan = 0) const;
        void appendSamples(short min, short max, int chan = 0);
        void setSamples(size_type index, short min, short max, int chan = 0);

		int getNumChannels() const;
		bool channelSizesMatch();

    private:
        int sample_rate_;
        int samples_per_pixel_;
        int bits_;

		std::vector<vector_type> channels_;
		
		void appendChannels(int chan);
};

//------------------------------------------------------------------------------

#endif // #if !defined(INC_WAVEFORM_BUFFER_H)

//------------------------------------------------------------------------------
