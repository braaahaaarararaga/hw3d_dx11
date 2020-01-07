#pragma once
#include <windows.h>
// ------------------------------
// D3D11SetDebugObjectName函数
// ------------------------------
// 为D3D设备创建出来的对象在图形调试器中设置对象名
// [In]ID3D11DeviceChild    D3D11设备创建出的对象
// [In]name                 对象名
template<UINT TNameLength>
inline void D3D11SetDebugObjectName(_In_ ID3D11DeviceChild* resource, _In_ const char(&name)[TNameLength])
{
	resource->SetPrivateData(WKPDID_D3DDebugObjectName, TNameLength - 1, name);
}

// ------------------------------
// DXGISetDebugObjectName函数
// ------------------------------
// 为DXGI对象在图形调试器中设置对象名
// [In]IDXGIObject          DXGI对象
// [In]name                 对象名
template<UINT TNameLength>
inline void DXGISetDebugObjectName(_In_ IDXGIObject* resource, _In_ const char(&name)[TNameLength])
{
	resource->SetPrivateData(WKPDID_D3DDebugObjectName, TNameLength - 1, name);
}