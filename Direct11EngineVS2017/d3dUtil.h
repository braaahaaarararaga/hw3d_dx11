#pragma once
#include <windows.h>
// ------------------------------
// D3D11SetDebugObjectName����
// ------------------------------
// ΪD3D�豸���������Ķ�����ͼ�ε����������ö�����
// [In]ID3D11DeviceChild    D3D11�豸�������Ķ���
// [In]name                 ������
template<UINT TNameLength>
inline void D3D11SetDebugObjectName(_In_ ID3D11DeviceChild* resource, _In_ const char(&name)[TNameLength])
{
	resource->SetPrivateData(WKPDID_D3DDebugObjectName, TNameLength - 1, name);
}

// ------------------------------
// DXGISetDebugObjectName����
// ------------------------------
// ΪDXGI������ͼ�ε����������ö�����
// [In]IDXGIObject          DXGI����
// [In]name                 ������
template<UINT TNameLength>
inline void DXGISetDebugObjectName(_In_ IDXGIObject* resource, _In_ const char(&name)[TNameLength])
{
	resource->SetPrivateData(WKPDID_D3DDebugObjectName, TNameLength - 1, name);
}