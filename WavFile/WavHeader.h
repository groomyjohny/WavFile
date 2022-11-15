#pragma once
#include <stdint.h>
#include <fstream>
#include <sstream>

namespace WavFile
{
	template <typename T>
	void writeToFile(T val, std::ofstream& f)
	{
		f.write(&val, sizeof val);
	}

	//This class was not meant to be used outside WavFile. Setters may leave object in invalid state, use finalize or writeToFile methods to get valid header if possible. If not, it will throw a std::runtime_error with error message.
	//Usage: create instance by using overloaded constructor, then prepare your samples, finalize and write
	class WavHeader
	{
	public:
		WavHeader() = default;

		WavHeader(size_t sampleRate, size_t numChannels, size_t bitsPerSample)
		{
			chunkId = 0x52494646;
			format = 0x57415645;
			subchunk1Id = 0x666d7420;
			subchunk1Size = 16;
			audioFormat = 1;
			this->bitsPerSample = bitsPerSample;
			blockAlign = (numChannels * bitsPerSample) / 8;
			byteRate = blockAlign * sampleRate;
			subchunk2Id = 0x64617461;
			this->verify();
		}

		bool verify()
		{
			std::stringstream msg;

			if (chunkId != 0x52494646) msg << "WavHeader: bad chunkId (bytes 0-3), expected 0x52494646 (\"RIFF\"), got " << std::hex << chunkId << "\n";
			if (format != 0x57415645) msg << "WavHeader: bad format, expected 0x57415645 (\"WAVE\"), got" << std::hex << format << "\n";
			if (subchunk1Id != 0x666d7420) msg << "WavHeader: bad subchunk1Id, expected 0x666d7420 (\"fmt \"), got " << std::hex << subchunk1Id << "\n";
			if (subchunk1Size != 16) msg << "WavHeader: unexpected subchunk1Size, expected 16, got " << std::dec << subchunk1Size << "\n";
			if (audioFormat != 1) msg << "WavHeader: unsupported audio format: " << std::dec << audioFormat << ". Only format 1 (uncompressed PCM) supported\n";
			if (bitsPerSample != 8 && bitsPerSample != 16) msg << "WavHeader: unsupported bitsPerSample value. Only values of 8, 16 are suppored\n";

			uint32_t correctBlockAlign = (numChannels * bitsPerSample) / 8;
			uint32_t correctByteRate = correctBlockAlign * sampleRate;
			if (byteRate != correctByteRate) msg << "WavHeader: incorrect byteRate. Expected " << correctByteRate << ", got " << byteRate << "\n";
			if (blockAlign != correctBlockAlign) msg << "WavHeader: incorrect blockAlign. Expected " << correctBlockAlign << ", got " << blockAlign << "\n";

			if (subchunk2Id != 0x64617461) msg << "WavHeader: bad subchunk2Id. Expected 0x64617461 (\"data\"), got " << std::hex << subchunk2Id << std::dec << "\n";

			std::string s_msg = msg.str();
			if (s_msg.empty()) return true;
			else throw std::runtime_error(s_msg.c_str());
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

		void finalize(size_t sampleCount)
		{
			subchunk2Size = sampleCount * blockAlign;
		}

		void write(std::ofstream& f)
		{
			this->verify();
			writeToFile(chunkId, f);
			writeToFile(chunkSize, f);
			writeToFile(format, f);
			writeToFile(subchunk1Id, f);
			writeToFile(subchunk1Size, f);
			writeToFile(audioFormat, f);
			writeToFile(numChannels, f);
			writeToFile(sampleRate, f);
			writeToFile(byteRate, f);
			writeToFile(blockAlign, f);
			writeToFile(bitsPerSample, f);
			writeToFile(subchunk2Id, f);
			writeToFile(subchunk2Size, f);
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

		/*void setSampleRate(uint32_t sampleRate)
		{
			this->sampleRate = sampleRate;
			byteRate = sampleRate * blockAlign;
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
}