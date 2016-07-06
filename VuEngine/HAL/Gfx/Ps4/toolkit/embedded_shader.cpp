/* SCE CONFIDENTIAL
PlayStation(R)4 Programmer Tool Runtime Library Release 02.000.071
* Copyright (C) 2013 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#include "embedded_shader.h"
#include <string.h>
#include <gnmx/shader_parser.h>
#include <algorithm>
#include "memory_requests.h"
using namespace sce;

void Gnmx::Toolkit::EmbeddedPsShader::addToMemoryRequests(MemoryRequests *memoryRequests) const
{
	Gnmx::ShaderInfo shaderInfo;
	Gnmx::parseShader(&shaderInfo, m_source);

	memoryRequests->m_garlic.request(shaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	memoryRequests->m_onion.request(shaderInfo.m_psShader->computeSize(), Gnm::kAlignmentOfBufferInBytes);
}

void Gnmx::Toolkit::EmbeddedPsShader::initializeWithMemoryRequests(MemoryRequests *memoryRequests)
{
	Gnmx::ShaderInfo shaderInfo;
	Gnmx::parseShader(&shaderInfo, m_source);

	void *shaderBinary = memoryRequests->m_garlic.redeem(shaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	void *shaderHeader = memoryRequests->m_onion.redeem(shaderInfo.m_psShader->computeSize(), Gnm::kAlignmentOfBufferInBytes);

	memcpy(shaderBinary, shaderInfo.m_gpuShaderCode, shaderInfo.m_gpuShaderCodeSize);
	memcpy(shaderHeader, shaderInfo.m_psShader, shaderInfo.m_psShader->computeSize());

	m_shader = static_cast<Gnmx::PsShader*>(shaderHeader);
	m_shader->patchShaderGpuAddress(shaderBinary);
}

void Gnmx::Toolkit::EmbeddedCsShader::addToMemoryRequests(MemoryRequests *memoryRequests) const
{
	Gnmx::ShaderInfo shaderInfo;
	Gnmx::parseShader(&shaderInfo, m_source);

	memoryRequests->m_garlic.request(shaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	memoryRequests->m_onion.request(shaderInfo.m_csShader->computeSize(), Gnm::kAlignmentOfBufferInBytes);
}

void Gnmx::Toolkit::EmbeddedCsShader::initializeWithMemoryRequests(MemoryRequests *memoryRequests)
{
	Gnmx::ShaderInfo shaderInfo;
	Gnmx::parseShader(&shaderInfo, m_source);

	void *shaderBinary = memoryRequests->m_garlic.redeem(shaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	void *shaderHeader = memoryRequests->m_onion.redeem(shaderInfo.m_csShader->computeSize(), Gnm::kAlignmentOfBufferInBytes);

	memcpy(shaderBinary, shaderInfo.m_gpuShaderCode, shaderInfo.m_gpuShaderCodeSize);
	memcpy(shaderHeader, shaderInfo.m_csShader, shaderInfo.m_csShader->computeSize());

	m_shader = static_cast<Gnmx::CsShader*>(shaderHeader);
	m_shader->patchShaderGpuAddress(shaderBinary);
}

void Gnmx::Toolkit::EmbeddedVsShader::addToMemoryRequests(MemoryRequests *memoryRequests) const
{
	Gnmx::ShaderInfo shaderInfo;
	Gnmx::parseShader(&shaderInfo, m_source);

	memoryRequests->m_garlic.request(shaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	memoryRequests->m_onion.request(shaderInfo.m_vsShader->computeSize(), Gnm::kAlignmentOfBufferInBytes);
}

void Gnmx::Toolkit::EmbeddedVsShader::initializeWithMemoryRequests(MemoryRequests *memoryRequests)
{
	Gnmx::ShaderInfo shaderInfo;
	Gnmx::parseShader(&shaderInfo, m_source);

	void *shaderBinary = memoryRequests->m_garlic.redeem(shaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	void *shaderHeader = memoryRequests->m_onion.redeem(shaderInfo.m_vsShader->computeSize(), Gnm::kAlignmentOfBufferInBytes);

	memcpy(shaderBinary, shaderInfo.m_gpuShaderCode, shaderInfo.m_gpuShaderCodeSize);
	memcpy(shaderHeader, shaderInfo.m_vsShader, shaderInfo.m_vsShader->computeSize());

	m_shader = static_cast<Gnmx::VsShader*>(shaderHeader);
	m_shader->patchShaderGpuAddress(shaderBinary);
}

void Gnmx::Toolkit::EmbeddedEsShader::addToMemoryRequests(MemoryRequests *memoryRequests) const
{
	Gnmx::ShaderInfo shaderInfo;
	Gnmx::parseShader(&shaderInfo, m_source);

	memoryRequests->m_garlic.request(shaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	memoryRequests->m_onion.request(shaderInfo.m_esShader->computeSize(), Gnm::kAlignmentOfBufferInBytes);
}

void Gnmx::Toolkit::EmbeddedEsShader::initializeWithMemoryRequests(MemoryRequests *memoryRequests)
{
	Gnmx::ShaderInfo shaderInfo;
	Gnmx::parseShader(&shaderInfo, m_source);

	void *shaderBinary = memoryRequests->m_garlic.redeem(shaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	void *shaderHeader = memoryRequests->m_onion.redeem(shaderInfo.m_esShader->computeSize(), Gnm::kAlignmentOfBufferInBytes);

	memcpy(shaderBinary, shaderInfo.m_gpuShaderCode, shaderInfo.m_gpuShaderCodeSize);
	memcpy(shaderHeader, shaderInfo.m_esShader, shaderInfo.m_esShader->computeSize());

	m_shader = static_cast<Gnmx::EsShader*>(shaderHeader);
	m_shader->patchShaderGpuAddress(shaderBinary);
}

void Gnmx::Toolkit::EmbeddedGsShader::addToMemoryRequests(MemoryRequests *memoryRequests) const
{
	Gnmx::ShaderInfo gsShaderInfo;
	Gnmx::ShaderInfo vsShaderInfo;
	Gnmx::parseGsShader(&gsShaderInfo, &vsShaderInfo, m_source);

	memoryRequests->m_garlic.request(gsShaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	memoryRequests->m_garlic.request(vsShaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	memoryRequests->m_onion.request(gsShaderInfo.m_gsShader->computeSize(), Gnm::kAlignmentOfBufferInBytes);
}

void Gnmx::Toolkit::EmbeddedGsShader::initializeWithMemoryRequests(MemoryRequests *memoryRequests)
{
	Gnmx::ShaderInfo gsShaderInfo;
	Gnmx::ShaderInfo vsShaderInfo;
	Gnmx::parseGsShader(&gsShaderInfo, &vsShaderInfo, m_source);

	void *gsShaderBinary = memoryRequests->m_garlic.redeem(gsShaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	void *vsShaderBinary = memoryRequests->m_garlic.redeem(vsShaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	void *gsShaderHeader = memoryRequests->m_onion.redeem(gsShaderInfo.m_gsShader->computeSize(), Gnm::kAlignmentOfBufferInBytes);

	memcpy(gsShaderBinary, gsShaderInfo.m_gpuShaderCode, gsShaderInfo.m_gpuShaderCodeSize);
	memcpy(vsShaderBinary, vsShaderInfo.m_gpuShaderCode, vsShaderInfo.m_gpuShaderCodeSize);
	memcpy(gsShaderHeader, gsShaderInfo.m_gsShader, gsShaderInfo.m_gsShader->computeSize());

	m_shader = static_cast<Gnmx::GsShader*>(gsShaderHeader);
	m_shader->patchShaderGpuAddresses(gsShaderBinary, vsShaderBinary);
}

void Gnmx::Toolkit::EmbeddedLsShader::addToMemoryRequests(MemoryRequests *memoryRequests) const
{
	Gnmx::ShaderInfo shaderInfo;
	Gnmx::parseShader(&shaderInfo, m_source);

	memoryRequests->m_garlic.request(shaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	memoryRequests->m_onion.request(shaderInfo.m_lsShader->computeSize(), Gnm::kAlignmentOfBufferInBytes);
}

void Gnmx::Toolkit::EmbeddedLsShader::initializeWithMemoryRequests(MemoryRequests *memoryRequests)
{
	Gnmx::ShaderInfo shaderInfo;
	Gnmx::parseShader(&shaderInfo, m_source);

	void *shaderBinary = memoryRequests->m_garlic.redeem(shaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	void *shaderHeader = memoryRequests->m_onion.redeem(shaderInfo.m_lsShader->computeSize(), Gnm::kAlignmentOfBufferInBytes);

	memcpy(shaderBinary, shaderInfo.m_gpuShaderCode, shaderInfo.m_gpuShaderCodeSize);
	memcpy(shaderHeader, shaderInfo.m_lsShader, shaderInfo.m_lsShader->computeSize());

	m_shader = static_cast<Gnmx::LsShader*>(shaderHeader);
	m_shader->patchShaderGpuAddress(shaderBinary);
}

void Gnmx::Toolkit::EmbeddedHsShader::addToMemoryRequests(MemoryRequests *memoryRequests) const
{
	Gnmx::ShaderInfo shaderInfo;
	Gnmx::parseShader(&shaderInfo, m_source);

	memoryRequests->m_garlic.request(shaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	memoryRequests->m_onion.request(shaderInfo.m_hsShader->computeSize(), Gnm::kAlignmentOfBufferInBytes);
}

void Gnmx::Toolkit::EmbeddedHsShader::initializeWithMemoryRequests(MemoryRequests *memoryRequests)
{
	Gnmx::ShaderInfo shaderInfo;
	Gnmx::parseShader(&shaderInfo, m_source);

	void *shaderBinary = memoryRequests->m_garlic.redeem(shaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	void *shaderHeader = memoryRequests->m_onion.redeem(shaderInfo.m_hsShader->computeSize(), Gnm::kAlignmentOfBufferInBytes);

	memcpy(shaderBinary, shaderInfo.m_gpuShaderCode, shaderInfo.m_gpuShaderCodeSize);
	memcpy(shaderHeader, shaderInfo.m_hsShader, shaderInfo.m_hsShader->computeSize());

	m_shader = static_cast<Gnmx::HsShader*>(shaderHeader);
	m_shader->patchShaderGpuAddress(shaderBinary);
}

void Gnmx::Toolkit::EmbeddedCsVsShader::addToMemoryRequests(MemoryRequests *memoryRequests) const
{
	Gnmx::ShaderInfo csvsShaderInfo;
	Gnmx::ShaderInfo csShaderInfo;
	Gnmx::parseCsVsShader(&csvsShaderInfo, &csShaderInfo, m_source);

	memoryRequests->m_garlic.request(csvsShaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	memoryRequests->m_garlic.request(csShaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	memoryRequests->m_onion.request(csvsShaderInfo.m_csvsShader->computeSize(), Gnm::kAlignmentOfBufferInBytes);
}

void Gnmx::Toolkit::EmbeddedCsVsShader::initializeWithMemoryRequests(MemoryRequests *memoryRequests)
{
	Gnmx::ShaderInfo csShaderInfo;
	Gnmx::ShaderInfo csvsShaderInfo;
	Gnmx::parseCsVsShader(&csvsShaderInfo, &csShaderInfo, m_source);

	void *vsShaderBinary = memoryRequests->m_garlic.redeem(csvsShaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	void *csShaderBinary = memoryRequests->m_garlic.redeem(csShaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
	void *csvsShaderHeader = memoryRequests->m_onion.redeem(csvsShaderInfo.m_csvsShader->computeSize(), Gnm::kAlignmentOfBufferInBytes);

	memcpy(vsShaderBinary, csvsShaderInfo.m_gpuShaderCode, csvsShaderInfo.m_gpuShaderCodeSize);
	memcpy(csShaderBinary, csShaderInfo.m_gpuShaderCode, csShaderInfo.m_gpuShaderCodeSize);
	memcpy(csvsShaderHeader, csvsShaderInfo.m_csvsShader, csvsShaderInfo.m_csvsShader->computeSize());

	m_shader = static_cast<Gnmx::CsVsShader*>(csvsShaderHeader);
	m_shader->patchShaderGpuAddresses(vsShaderBinary, csShaderBinary);
}


void Gnmx::Toolkit::EmbeddedShaders::addToMemoryRequests(MemoryRequests *memoryRequests) const
{
	for(uint32_t i = 0; i < m_embeddedCsShaders; ++i)
		m_embeddedCsShader[i]->addToMemoryRequests(memoryRequests);
	for(uint32_t i = 0; i < m_embeddedPsShaders; ++i)
		m_embeddedPsShader[i]->addToMemoryRequests(memoryRequests);
	for(uint32_t i = 0; i < m_embeddedVsShaders; ++i)
		m_embeddedVsShader[i]->addToMemoryRequests(memoryRequests);
	for(uint32_t i = 0; i < m_embeddedEsShaders; ++i)
		m_embeddedEsShader[i]->addToMemoryRequests(memoryRequests);
	for(uint32_t i = 0; i < m_embeddedGsShaders; ++i)
		m_embeddedGsShader[i]->addToMemoryRequests(memoryRequests);
	for(uint32_t i = 0; i < m_embeddedLsShaders; ++i)
		m_embeddedLsShader[i]->addToMemoryRequests(memoryRequests);
	for(uint32_t i = 0; i < m_embeddedHsShaders; ++i)
		m_embeddedHsShader[i]->addToMemoryRequests(memoryRequests);
	for(uint32_t i = 0; i < m_embeddedCsVsShaders; ++i)
		m_embeddedCsVsShader[i]->addToMemoryRequests(memoryRequests);
}

void Gnmx::Toolkit::EmbeddedShaders::initializeWithMemoryRequests(MemoryRequests *memoryRequests)
{
	for(uint32_t i = 0; i < m_embeddedCsShaders; ++i)
		m_embeddedCsShader[i]->initializeWithMemoryRequests(memoryRequests);
	for(uint32_t i = 0; i < m_embeddedPsShaders; ++i)
		m_embeddedPsShader[i]->initializeWithMemoryRequests(memoryRequests);
	for(uint32_t i = 0; i < m_embeddedVsShaders; ++i)
		m_embeddedVsShader[i]->initializeWithMemoryRequests(memoryRequests);
	for(uint32_t i = 0; i < m_embeddedEsShaders; ++i)
		m_embeddedEsShader[i]->initializeWithMemoryRequests(memoryRequests);
	for(uint32_t i = 0; i < m_embeddedGsShaders; ++i)
		m_embeddedGsShader[i]->initializeWithMemoryRequests(memoryRequests);
	for(uint32_t i = 0; i < m_embeddedLsShaders; ++i)
		m_embeddedLsShader[i]->initializeWithMemoryRequests(memoryRequests);
	for(uint32_t i = 0; i < m_embeddedHsShaders; ++i)
		m_embeddedHsShader[i]->initializeWithMemoryRequests(memoryRequests);
	for(uint32_t i = 0; i < m_embeddedCsVsShaders; ++i)
		m_embeddedCsVsShader[i]->initializeWithMemoryRequests(memoryRequests);
}
