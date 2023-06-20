#include "example.h"
#include <string>

class AudioResource
{
private:
	HMMIO			hmmio;
	BYTE*			pcm_data;
	DWORD			pcm_data_size;
	WAVEFORMATEX	wave_format;
	std::string		filename;
public:
	AudioResource(std::string filename)
		:hmmio(NULL), pcm_data(NULL), pcm_data_size(), wave_format(), filename(filename)
	{
		assert(!filename.empty());
		hmmio = mmioOpenA(&filename.front(), NULL, MMIO_ALLOCBUF | MMIO_READ);
		assert(hmmio != NULL);
		MMCKINFO main_chunk;
		MMCKINFO sub_chunk;
		main_chunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
		MMRESULT mr = mmioDescend(hmmio, &main_chunk, NULL, MMIO_FINDRIFF);
		assert(mr == 0);
		sub_chunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
		mr = mmioDescend(hmmio, &sub_chunk, &main_chunk, MMIO_FINDCHUNK);
		assert(mr == 0);
		mr = mmioRead(hmmio, (HPSTR)&wave_format, sizeof(wave_format)) != sizeof(wave_format);
		assert(mr == 0);
		assert(wave_format.wFormatTag == WAVE_FORMAT_PCM);
		wave_format.cbSize = 0;
		mr = mmioAscend(hmmio, &sub_chunk, 0);
		assert(mr == 0);
		sub_chunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
		mr = mmioDescend(hmmio, &sub_chunk, &main_chunk, MMIO_FINDCHUNK);
		assert(mr == 0);
		pcm_data_size = sub_chunk.cksize;
	}
	LONG read(BYTE* buf, DWORD size)
	{
		return mmioRead(hmmio, (HPSTR)buf, size);
	}
	bool readAll()
	{
		if (pcm_data == nullptr)
		{
			pcm_data = new BYTE[pcm_data_size];
		}
		backToTop();
		return read(pcm_data, pcm_data_size) > 0;
	}
	LONG seek(LONG offset)
	{
		return mmioSeek(hmmio, offset, SEEK_CUR);
	}
	void backToTop()
	{
		mmioSeek(hmmio, 0, SEEK_END);
		mmioSeek(hmmio, -(LONG)pcm_data_size, SEEK_CUR);
	}
	const std::string& getFilename()const { return filename; }
	BYTE* getPcmData()const { return pcm_data; }
	const DWORD& getPcmDataSize()const { return pcm_data_size; }
	const WAVEFORMATEX& getWaveFormat()const { return wave_format; }
	~AudioResource()
	{
		if (hmmio) { mmioClose(hmmio, 0); }
		if (pcm_data != NULL) { delete pcm_data; }
	}
};

WavePainter::WavePainter(ID3D11Device* device, const char* waveFilename)
	:Painter()
{
	AudioResource audioResource{ waveFilename };
	audioResource.readAll();
	WAVEFORMATEX format = audioResource.getWaveFormat();
	const UINT byte_per_sample = (UINT)format.wBitsPerSample / 8;

	data.sampleCount = (UINT)audioResource.getPcmDataSize() / byte_per_sample;
	data.samplePerSec = (UINT)format.nSamplesPerSec;

	float* wave = new float[data.sampleCount];

	if (byte_per_sample == 1)
	{
		BYTE* src = audioResource.getPcmData();
		for (UINT i = 0; i < data.sampleCount; i++)
		{
			wave[i] = (src[i] / (float)BYTE_MAX) * 2.0f - 1.0f;
		}
	}
	else
	{
		short* src = (short*)audioResource.getPcmData();
		for (UINT i = 0; i < data.sampleCount; i++)
		{
			wave[i] = (src[i] / (float)SHORT_MAX);
		}
	}

	createStructuredBuffer(device, &structuredBuffer, sizeof(float), data.sampleCount, wave);
	createConstantBuffer(device, &constantBuffer, sizeof(Data), &data);
	delete[] wave;

	loadPixelShader(device, &pixelShader, "WavePaint_ps.cso");
	loadVertexShader(device, &vertexShader, "WavePaint_vs.cso", 0, 0);
}

void WavePainter::draw(ID3D11DeviceContext* immediateContext)
{
	immediateContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	pixelShader.set(immediateContext);
	vertexShader.set(immediateContext);
	structuredBuffer.set(immediateContext, 0, 0, 1, 0, 0, 0);
	constantBuffer.updateSubresource(immediateContext, &data);
	constantBuffer.set(immediateContext, 0, 0, 1, 0, 0, 0);
	immediateContext->Draw(4, 0);
}

void WavePainter::bake(ID3D11DeviceContext* immediateContext, Layer* layer)
{
	CachedHandle ch = pushCachedComObjects(immediateContext);
	layer->switching(immediateContext);
	draw(immediateContext);
	popCachedComObjects(immediateContext, ch);
}

DestructionPainter::DestructionPainter(ID3D11Device* device)
{
}

void DestructionPainter::draw(ID3D11DeviceContext* immediateContext)
{

}
