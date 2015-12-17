
#pragma once

#ifndef _WIN32

#include "boost_image.h"

#else

#include <filesystem>

#include "matrix.h"

#include <wincodec.h>
#pragma comment(lib, "windowscodecs.lib")


// Laods an image and turns it into a matrix.
inline matrix load_image(const std::tr2::sys::path &path)
{
	auto hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	matrix x;

	IWICImagingFactory *wic = nullptr;
	IWICStream *stream = nullptr;
	IWICBitmapDecoder *decoder = nullptr;
	IWICBitmapFrameDecode *frame = nullptr;
	IWICFormatConverter *convert = nullptr;

	hr = [&]
	{

		HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory2), reinterpret_cast<LPVOID*>(&wic));
		if (FAILED(hr))
		{
			hr = CoCreateInstance(CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), reinterpret_cast<LPVOID*>(&wic));
		}
		if (FAILED(hr))
		{
			return hr;
		}

		wic->CreateStream(&stream);
		hr = stream->InitializeFromFilename(path.c_str(), GENERIC_READ);
		if (FAILED(hr))
		{
			return hr;
		}

		wic->CreateDecoderFromStream(stream, nullptr, WICDecodeMetadataCacheOnDemand, &decoder);
		hr = decoder->GetFrame(0, &frame);
		if (FAILED(hr))
		{
			return hr;
		}

		wic->CreateFormatConverter(&convert);
		hr = convert->Initialize(frame, GUID_WICPixelFormat8bppGray, WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeMedianCut);
		if (FAILED(hr))
		{
			return hr;
		}

		unsigned w, h;
		convert->GetSize(&w, &h);
		BYTE *buf = new BYTE[w * h];
		hr = convert->CopyPixels(nullptr, w, w * h, buf);
		if (SUCCEEDED(hr))
		{
			x.create(h, w);

			auto ptr = buf;
			auto end = ptr + w * h;
			auto xptr = x.data;
			while (ptr != end)
			{
				*xptr++ = *ptr++;
			}
		}

		delete[]buf;
		return hr;
	}();

	if (convert) convert->Release();
	if (frame) frame->Release();
	if (decoder) decoder->Release();
	if (wic) wic->Release();

	CoUninitialize();

	return x;
}


// Writes the matrix as an 8-bit-per-pixel PNG
inline void save_png(const matrix &x, const std::tr2::sys::path &path)
{
	auto hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	IWICImagingFactory *wic = nullptr;
	IWICStream *stream = nullptr;
	IWICBitmapEncoder *encoder = nullptr;
	IWICBitmapFrameEncode *frame = nullptr;
	IPropertyBag2 *props = nullptr;

	hr = [&]
	{

		HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory2), reinterpret_cast<LPVOID*>(&wic));
		if (FAILED(hr))
		{
			hr = CoCreateInstance(CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), reinterpret_cast<LPVOID*>(&wic));
		}
		if (FAILED(hr))
		{
			return hr;
		}

		wic->CreateStream(&stream);
		hr = stream->InitializeFromFilename(path.c_str(), GENERIC_WRITE);
		if (FAILED(hr))
		{
			return hr;
		}

		wic->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder);
		hr = encoder->Initialize(stream, WICBitmapEncoderNoCache);
		if (FAILED(hr))
		{
			return hr;
		}

		hr = encoder->CreateNewFrame(&frame, &props);
		if (FAILED(hr))
		{
			return hr;
		}

		hr = frame->Initialize(props);
		if (FAILED(hr))
		{
			return hr;
		}

		frame->SetSize(x.cols, x.rows);

		GUID format = GUID_WICPixelFormat8bppGray;
		frame->SetPixelFormat(&format);

		BYTE *buf = new BYTE[x.rows * x.cols];
		auto xptr = x.data;
		auto xend = xptr + x.rows * x.cols;
		auto ptr = buf;
		while (xptr != xend)
		{
			auto i = *xptr++;
			if (i < 0) i = 0;
			else if (i > 255) i = 255;
			*ptr++ = i;
		}

		hr = frame->WritePixels(x.rows, x.cols, x.rows * x.cols, buf);
		delete[] buf;
		if (FAILED(hr))
		{
			return hr;
		}

		hr = frame->Commit();
		if (FAILED(hr))
		{
			return hr;
		}

		hr = encoder->Commit();

		return hr;

	}();

	if (frame) frame->Release();
	if (props) props->Release();
	if (encoder) encoder->Release();
	if (wic) wic->Release();

	CoUninitialize();

}

#endif
