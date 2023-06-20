#include "example.h"
#include <string>

class AudioResource
{
private:
	HMMIO			hmmio;
	BYTE*			pcmData;
	DWORD			pcmDataSize;
	WAVEFORMATEX	waveFormat;
	std::string		filename;
public:
	AudioResource(std::string filename)
		:hmmio(NULL), pcmData(NULL), pcmDataSize(), waveFormat(), filename(filename)
	{
		assert(!filename.empty());
		hmmio = mmioOpenA(&filename.front(), NULL, MMIO_ALLOCBUF | MMIO_READ);
		assert(hmmio != NULL);
		MMCKINFO mainChunk = {};
		MMCKINFO subChunk = {};
		mainChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
		MMRESULT mr = mmioDescend(hmmio, &mainChunk, NULL, MMIO_FINDRIFF);
		assert(mr == 0);
		subChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
		mr = mmioDescend(hmmio, &subChunk, &mainChunk, MMIO_FINDCHUNK);
		assert(mr == 0);
		mr = mmioRead(hmmio, (HPSTR)&waveFormat, sizeof(waveFormat)) != sizeof(waveFormat);
		assert(mr == 0);
		assert(waveFormat.wFormatTag == WAVE_FORMAT_PCM);
		waveFormat.cbSize = 0;
		mr = mmioAscend(hmmio, &subChunk, 0);
		assert(mr == 0);
		subChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
		mr = mmioDescend(hmmio, &subChunk, &mainChunk, MMIO_FINDCHUNK);
		assert(mr == 0);
		pcmDataSize = subChunk.cksize;
	}
	LONG read(BYTE* buf, DWORD size)
	{
		return mmioRead(hmmio, (HPSTR)buf, size);
	}
	bool readAll()
	{
		if (pcmData == nullptr)
		{
			pcmData = new BYTE[pcmDataSize];
		}
		backToTop();
		return read(pcmData, pcmDataSize) > 0;
	}
	LONG seek(LONG offset)
	{
		return mmioSeek(hmmio, offset, SEEK_CUR);
	}
	void backToTop()
	{
		mmioSeek(hmmio, 0, SEEK_END);
		mmioSeek(hmmio, -(LONG)pcmDataSize, SEEK_CUR);
	}
	const std::string& getFilename()const { return filename; }
	BYTE* getPcmData()const { return pcmData; }
	const DWORD& getPcmDataSize()const { return pcmDataSize; }
	const WAVEFORMATEX& getWaveFormat()const { return waveFormat; }
	~AudioResource()
	{
		if (hmmio) { mmioClose(hmmio, 0); }
		if (pcmData != NULL) { delete pcmData; }
	}
};

WavePainter::WavePainter(ID3D11Device* device, const char* waveFilename)
	:Painter(device)
{
	AudioResource audioResource{ waveFilename };
	audioResource.readAll();
	WAVEFORMATEX format = audioResource.getWaveFormat();
	const UINT bytePerSample = (UINT)format.wBitsPerSample / 8;

	data.sampleCount = (UINT)audioResource.getPcmDataSize() / bytePerSample;
	data.samplePerSec = (UINT)format.nSamplesPerSec;

	float* wave = new float[data.sampleCount];

	if (bytePerSample == 1)
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

	loadPixelShader(device, &pixelShader, "asset\\WavePaint_ps.cso");
	loadVertexShader(device, &vertexShader, "asset\\WavePaint_vs.cso", 0, 0);
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
	:Painter(device)
{
	createConstantBuffer(device, &constantBuffer, sizeof(Data), &data);
	loadPixelShader(device, &pixelShader, "asset\\Destruction_ps.cso");
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	loadVertexShader(device, &vertexShader, "asset\\Destruction_vs.cso", inputElementDesc, 2);
	loadDomainShader(device, &domainShader, "asset\\Destruction_ds.cso");
	loadHullShader(device, &hullShader, "asset\\Destruction_hs.cso");
	loadGeometryShader(device, &geometryShader, "asset\\Destruction_gs.cso");
}

void DestructionPainter::draw(ID3D11DeviceContext* immediateContext, Geometry* geometry)
{
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	setDepthStencilState(immediateContext, DepthStencilState::common);
	setRasterizerState(immediateContext, RasterizerState::solid);
	pixelShader.set(immediateContext);
	vertexShader.set(immediateContext);
	domainShader.set(immediateContext);
	hullShader.set(immediateContext);
	geometryShader.set(immediateContext);
	constantBuffer.updateSubresource(immediateContext, &data);
	constantBuffer.set(immediateContext, 0, true, true, true, true, true);
	geometry->set(immediateContext);
	immediateContext->DrawIndexed(geometry->indexCount, 0, 0);
}

ToonPainter::ToonPainter(ID3D11Device* device)
	:Painter(device)
{
	createConstantBuffer(device, &constantBuffer, ((sizeof(Data) + (15)) & ~15), &data);
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	loadPixelShader(device, &pixelShader, "asset\\Toon_ps.cso");
	loadVertexShader(device, &vertexShader, "asset\\Toon_vs.cso", inputElementDesc, 2);
}

void ToonPainter::draw(ID3D11DeviceContext* immediateContext, Geometry* geometry)
{
	immediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	setDepthStencilState(immediateContext, DepthStencilState::common);
	setRasterizerState(immediateContext, RasterizerState::solid);
	pixelShader.set(immediateContext);
	vertexShader.set(immediateContext);
	constantBuffer.updateSubresource(immediateContext, &data);
	constantBuffer.set(immediateContext, 0, true, true, true, true, true);
	geometry->set(immediateContext);
	immediateContext->DrawIndexed(geometry->indexCount, 0, 0);
}
