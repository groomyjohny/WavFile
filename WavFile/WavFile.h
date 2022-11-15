#pragma once
#include <fstream>
#include <sstream>
#include <vector>

namespace WavFile
{
	class WavHeader //This class was not meant to be used outside WavFile. Setters may leave object in invalid state, use finalize or writeToFile methods to get valid header if possible. If not, it will throw a std::runtime_error with error message.
	{
	public:
		bool verify()
		{
			std::stringstream msg;

			if (chunkId != 0x52494646) msg << "WavHeader: bad chunkId (bytes 0-3), expected 0x52494646 (\"RIFF\"), got " << std::hex << chunkId << "\n";
			if (format != 0x57415645) msg << "WavHeader: bad format, expected 0x57415645 (\"WAVE\"), got" << std::hex << format << "\n";
			if (subchunk1Id != 0x666d7420) msg << "WavHeader: bad subchunk1Id, expected 0x666d7420 (\"fmt \"), got " << std::hex << subchunk1Id << "\n";
			if (subchunk1Size != 16) msg << "WavHeader: unexpected subchunk1Size, expected 16, got " << std::dec << subchunk1Size << "\n";
			if (audioFormat != 1) msg << "WavHeader: unsupported audio format: " << std::dec << audioFormat << ". Only format 1 (uncompressed PCM) supported\n";
			if (bitsPerSample != 8 || bitsPerSample != 16) msg << "WavHeader: unsupported bitsPerSample value. Only values of 8, 16 are suppored\n";

			uint32_t correctBlockAlign = (numChannels*bitsPerSample) / 8;
			uint32_t correctByteRate = correctBlockAlign * sampleRate;
			if (byteRate != correctByteRate) msg << "WavHeader: incorrect byteRate. Expected " << correctByteRate << ", got " << byteRate << "\n";
			if (blockAlign != correctBlockAlign) msg << "WavHeader: incorrect blockAlign. Expected " << correctBlockAlign << ", got " << blockAlign << "\n";

			if (subchunk2Id != 0x64617461) msg << "WavHeader: bad subchunk2Id. Expected 0x64617461 (\"data\"), got " << std::hex << subchunk2Id << "\n";

			std::string s_msg = msg.str();
			if (s_msg.empty()) return true;
			else throw std::runtime_error(s_msg.c_str());
		}

		double getDuration()
		{
			if (this->duration == -1)
			{
				return double(getSubchunk2Size()) / ((getNumChannels()*getBitsPerSample()*getSampleRate()) / 8)
			}
			else return this->duration;
		}

		uint32_t getSubchunk2Size()
		{
			if (subchunk2Size == 0) throw std::runtime_error("WavHeader: attempted to get unitiailized field: subchunk2Size\n");
			return subchunk2Size;
		}

		uint16_t getNumChannels()
		{
			if (numChannels == 0) throw std::runtime_error("WavHeader: attempted to get unitiailized field: numChannels\n");
			return numChannels;
		}

		uint16_t getBitsPerSample()
		{
			if (bitsPerSample == 0) throw std::runtime_error("WavHeader: attempted to get unitiailized field: bitsPerSample\n");
			return bitsPerSample;
		}

		uint32_t getSampleRate()
		{
			if (sampleRate == 0) throw std::runtime_error("WavHeader: attempted to get unitiailized field: sampleRate\n");
			return sampleRate;
		}

		uint16_t getBlockAlign()
		{
			if (blockAlign == 0) throw std::runtime_error("WavHeader: attempted to get unitiailized field: blockAlign\n");
			return blockAlign;
		}

		uint16_t getByteRate()
		{
			if (byteRate == 0) throw std::runtime_error("WavHeader: attempted to get unitiailized field: byteRate\n");
			return byteRate;
		}

	private:
		uint32_t chunkId = 0;
		uint32_t chunkSize = 0;
		uint32_t format = 0;
		uint32_t subchunk1Id = 0;
		uint32_t subchunk1Size = 0;
		uint16_t audioFormat = 0;
		uint16_t numChannels = 0;
		uint32_t sampleRate = 0;
		uint32_t byteRate = 0;
		uint16_t blockAlign = 0;
		uint16_t bitsPerSample = 0;
		uint32_t subchunk2Id = 0;
		uint32_t subchunk2Size = 0;
		double duration = -1;

		

		/*void setSampleRate(uint32_t sampleRate)
		{
			this->sampleRate = sampleRate;
			byteRate = sampleRate * blockAlign;
			this->duration = duration;

			double targetDuration = this->duration;
			if (targetDuration == -1) 
			this->setDuration(duration);
		}

		void setNumChannels(uint16_t numChannels)
		{
			this->numChannels = numChannels;
			blockAlign = (numChannels*bitsPerSample) / 8;
			byteRate = blockAlign * sampleRate;
		}

		void setDuration(double seconds)
		{
			uint32_t newSampleCount = uint32_t(seconds*sampleRate) + 1;
			subchunk2Size = newSampleCount * blockAlign;
			chunkSize = subchunk2Size + 44 - 8;
		}*/
	};

	//Samples are represented as an array a[x][y], where is x is channel number, y - samples of channel x as doubles in range [-1...1].
	class WavFile
	{
	public:
		std::vector<std::vector<double>> getSamples()
		{
			return samples;
		}
		std::vector<double> getSamplesOfChannel(int channelNum)
		{
			return samples[channelNum];
		}

		/*//Samples are represented as doubles in range [-1...1]. Use at your own risk, as class internals do not expect external sample changes
		std::vector<std::vector<double>>& getSamplesReference();*/

		//Sets sample rate of a file. Setting convertSamples to false will skip sample conversion, which will change the speed and duration of the file.
		void setSampleRate(uint32_t newSampleRate, bool convertSamples = true)
		{
			uint32_t oldSampleRate = header.getSampleRate();
			double oldSampleTime = 1.0 / oldSampleRate;
			header.sampleRate = newSampleRate;

			if (convertSamples)
			{
				uint32_t newSampleCount = header.getDuration()*newSampleRate;
				double newSampleTime = 1.0 / newSampleRate;
				double currTime = 0;
				for (auto& oldChan : samples)
				{
					std::vector<double> newChan(newSampleCount);
					for (uint32_t i = 0; i < newSampleCount; ++i) //interpolate samples
					{
						uint32_t oldInd = currTime * oldSampleRate;
						newChan[i] = oldChan[oldInd];
						currTime += newSampleTime;
					}
					oldChan = newChan;
				}
			}
		}

		//Sets new number of channels. Newly created channels will be initialized with zeros if init is 1, special scheme below if 2, and left unitialized if 0. If init == 2, then:
		//If newNumChannels < oldNumChannels, then channels with indices[newNumChannels-1...oldNumChannels-1] will be removed;
		//If newNumChannels > oldNumChannels, new channels will be copied from old ones. Channel n will be a copy of channel (n % oldNumChannels).
		//If newNumChannels == oldNumChannels, no changes are made
		void setNumChannels(uint16_t newNumChannels, int init = 1)
		{
			uint16_t oldNumChannels = header.numChannels;
			header.numChannels = newNumChannels;

			this->samples.resize(newNumChannels);
			if (newNumChannels > oldNumChannels)
			{
				if (init == 1)
				{
					for (size_t i = oldNumChannels-1; i < newNumChannels; ++i)
					{
						samples[i] = std::vector<double>()
					}
				}
			}
		}
	private:
		WavHeader header;
		std::fstream file;
		std::vector<std::vector<double>> samples;
	};
}