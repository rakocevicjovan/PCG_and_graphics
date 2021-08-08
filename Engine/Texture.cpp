#include "pch.h"

#include "Texture.h"
#include "Image.h"
#include "WICTextureLoader.h"
#include "STBImporter.h"
#include "Perlin.h"


Texture::Texture(ID3D11Device* device, const std::string& fileName)
{
	loadFromFile(device, fileName.c_str());
}


Texture::Texture(ID3D11Device* device, uint32_t w, uint32_t h, DXGI_FORMAT format, void* data, uint32_t flags, uint32_t mips, uint32_t arrSize)
	: _w(w), _h(h), _format(format), _numMips(mips), _arrSize(arrSize)
{
	//auto apiFormat = TO_API_FORMAT(format);

	auto desc = Create2DTexDesc(
		w, h, format,
		data ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DEFAULT,
		flags, 0u, 0u, mips, arrSize);

	bool isSrv = flags && D3D11_BIND_SHADER_RESOURCE;

	if (data)
	{
		auto texData = CreateSubresourceData(data, desc.Width, format);	// FORMAT_TO_SIZE(apiFormat)
		create(device, desc, &texData, isSrv);
	}
	else
	{
		create(device, desc, nullptr, isSrv);
	}
}


bool Texture::loadFromFile(ID3D11Device* device, const char* filename)
{
	Image img = STBImporter::ImportFromFile(filename);
	return loadFromImage(device, std::move(img));
}


bool Texture::loadFromMemory(ID3D11Device* device, const unsigned char* data, size_t size)
{
	Image img = STBImporter::ImportFromMemory(data, size);
	return loadFromImage(device, img);
}


bool Texture::loadFromImage(ID3D11Device* device, const Image& image, bool asSRV)
{
	_w = image.width();
	_h = image.height();
	_nc = image.numChannels();

	// Remove this eventually
	_mdata = image.data();

	// For now always uses 1 byte per channel textures, @TODO add proper format both in image and here!
	auto inferredFormat = TO_API_FORMAT(_nc - 1);
	auto desc = Create2DTexDesc(_w, _h, inferredFormat, D3D11_USAGE_IMMUTABLE);

	UINT pixelWidth = _nc;
	auto texData = CreateSubresourceData(image.data().get(), desc.Width, pixelWidth);

	create(device, desc, &texData, asSRV);
	return true;
}


bool Texture::loadWithMipLevels(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& path)
{
	std::wstring temp(path.begin(), path.end());
	const wchar_t* widecstr = temp.c_str();

	HRESULT result = DirectX::CreateWICTextureFromFile(device, context, widecstr, nullptr, &_srv, 0);

	if (FAILED(result))
	{
		OutputDebugStringA("Can't create texture2d with mip levels (WIC). \n");
		return false;
	}
	return true;
}


// make asSrv into flags instead
void Texture::create(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& desc, const D3D11_SUBRESOURCE_DATA* data, bool asSRV)
{
	_dxID = CreateTexture2D(device, desc, data);

	if (_mdata.get())
	{
		_mdata.reset();
	}

	if (asSRV)
	{
		createSRV(device, desc);
	}
}


void Texture::createSRV(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& desc)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = desc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	_srv = CreateSRV(device, _dxID.Get(), shaderResourceViewDesc);
}


Microsoft::WRL::ComPtr<ID3D11Texture2D> Texture::CreateTexture2D(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& desc, const D3D11_SUBRESOURCE_DATA* data)
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex2D;
	if (FAILED(device->CreateTexture2D(&desc, data, tex2D.GetAddressOf())))
	{
		OutputDebugStringA("Can't create texture2d. \n");
		__debugbreak();
	}
	return tex2D;
}


Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Texture::CreateSRV(ID3D11Device* device, ID3D11Resource* resource, const D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc)
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	if (FAILED(device->CreateShaderResourceView(resource, &srvDesc, srv.GetAddressOf())))
	{
		OutputDebugStringA("Can't create shader resource view. \n");
		__debugbreak();
	}
	return srv;
}


D3D11_SUBRESOURCE_DATA Texture::CreateSubresourceData(void* data, uint32_t textureWidth, uint32_t texelWidth, uint16_t mips, uint16_t arrSize)
{
	//std::vector<D3D11_SUBRESOURCE_DATA> result;
	//result.reserve(mips * arrSize);

	//for (auto texIdx = 0u; texIdx < arrSize; ++texIdx)
	//{
	//	for (auto mipLevel = 0u; mipLevel < mips; ++mipLevel)
	//	{
			D3D11_SUBRESOURCE_DATA texData{};
			texData.pSysMem = data;
			texData.SysMemPitch = textureWidth * texelWidth;	// (textureWidth >> mipLevel)
			texData.SysMemSlicePitch = 0;
			return texData;
			//result.emplace_back(std::move(texData));
	//	}
	//}
	//return result;
}


std::vector<uint8_t> Texture::LoadToSysMem(ID3D11Device* device, ID3D11DeviceContext* dc, const Texture& tex)
{
	// Create a staging texture to copy to, currently not the entire format is stored in texture so I only use number of channels
	Microsoft::WRL::ComPtr<ID3D11Texture2D> stagingTexture{};

	D3D11_TEXTURE2D_DESC texDesc{};
	texDesc.Width = tex.w();
	texDesc.Height = tex.h();
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = TO_API_FORMAT(tex._nc - 1);
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_STAGING;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	texDesc.BindFlags = 0;
	texDesc.MiscFlags = 0;

	if (FAILED(device->CreateTexture2D(&texDesc, 0, stagingTexture.GetAddressOf())))
	{
		OutputDebugStringA("Can't create off-screen texture. \n");
		return {};
	}

	// Copy data from the GPU texture to the staging texture
	dc->CopyResource(stagingTexture.Get(), tex._dxID.Get());

	D3D11_MAPPED_SUBRESOURCE msr;
	dc->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &msr);

	uint8_t* pDataPtr = static_cast<unsigned char*>(msr.pData);

	std::vector<uint8_t> result(pDataPtr, pDataPtr + msr.DepthPitch);

	dc->Unmap(stagingTexture.Get(), 0);

	return result;
}