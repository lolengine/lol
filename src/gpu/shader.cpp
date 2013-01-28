//
// Lol Engine
//
// Copyright: (c) 2010-2013 Sam Hocevar <sam@hocevar.net>
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the Do What The Fuck You Want To
//   Public License, Version 2, as published by Sam Hocevar. See
//   http://www.wtfpl.net/ for more details.
//

#if defined HAVE_CONFIG_H
#   include "config.h"
#endif

#include <cstring>
#include <cstdio>

#ifdef WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   if defined USE_D3D9
#       include <algorithm>
        using std::min;
        using std::max;
#       include <d3d9.h>
#       include <d3dx9shader.h>
#   endif
#endif

#include "core.h"
#include "lolgl.h"

using namespace std;

#if defined USE_D3D9
extern IDirect3DDevice9 *g_d3ddevice;
#elif defined _XBOX
extern D3DDevice *g_d3ddevice;
#endif

namespace lol
{

/*
 * Shader implementation class
 */

class ShaderData
{
    friend class Shader;

private:
#if defined USE_D3D9
    IDirect3DVertexShader9 *vert_shader;
    IDirect3DPixelShader9 *frag_shader;
    ID3DXConstantTable *vert_table, *frag_table;
#elif defined _XBOX
    D3DVertexShader *vert_shader;
    D3DPixelShader *frag_shader;
    ID3DXConstantTable *vert_table, *frag_table;
#elif !defined __CELLOS_LV2__
    GLuint prog_id, vert_id, frag_id;
#else
    CGprogram vert_id, frag_id;
#endif
    uint32_t vert_crc, frag_crc;

    /* Shader patcher */
    static int GetVersion();
    static void Patch(char *dst, char const *vert, char const *frag);

    /* Global shader cache */
    static Shader *shaders[];
    static Hash<char const *> hash;
    static int nshaders;
};

Shader *ShaderData::shaders[256];
Hash<char const *> ShaderData::hash;
int ShaderData::nshaders = 0;

/*
 * Public Shader class
 */

Shader *Shader::Create(char const *lolfx)
{
    char *src = new char[strlen(lolfx) + 2];
    memcpy(src + 1, lolfx, strlen(lolfx) + 1);
    src[0] = '\n';

    /* Parse the crap */
    Array<char const *, char const *> sections;
    char *key = NULL;
    for (char *parser = src; *parser; )
    {
        if (key == NULL && (parser[0] == '\n' || parser[0] == '\r')
             && parser[1] == '[')
        {
            *parser = '\0';
            parser += 2;
            key = parser;
        }
        else if (key && parser[0] == ']')
        {
            *parser++ = '\0';
        }
        else if (key && (parser[0] == '\n' || parser[0] == '\r'))
        {
            sections.Push(key, parser);
            parser++;
            key = NULL;
        }
        else
        {
            parser++;
        }
    }

    char const *vert = NULL, *frag = NULL;
    for (int i = 0; i < sections.Count(); i++)
    {
#if !defined __CELLOS_LV2__ && !defined _XBOX && !defined USE_D3D9
        if (!strcmp(sections[i].m1, "vert.glsl"))
            vert = sections[i].m2;
        if (!strcmp(sections[i].m1, "frag.glsl"))
            frag = sections[i].m2;
#else
        if (!strcmp(sections[i].m1, "vert.hlsl"))
            vert = sections[i].m2;
        if (!strcmp(sections[i].m1, "frag.hlsl"))
            frag = sections[i].m2;
#endif
    }

    /* FIXME: we don’t know how to handle these yet. */
    if (!vert)
        Log::Error("no vertex shader found… sorry, I’m gonna crash now.\n");
    if (!frag)
        Log::Error("no fragment shader found… sorry, I’m gonna crash now.\n");

    uint32_t new_vert_crc = ShaderData::hash(vert);
    uint32_t new_frag_crc = ShaderData::hash(frag);

    for (int n = 0; n < ShaderData::nshaders; n++)
    {
        if (ShaderData::shaders[n]->data->vert_crc == new_vert_crc
             && ShaderData::shaders[n]->data->frag_crc == new_frag_crc)
        {
            delete[] src;
            return ShaderData::shaders[n];
        }
    }

    Shader *ret = new Shader(vert, frag);
    ShaderData::shaders[ShaderData::nshaders] = ret;
    ShaderData::nshaders++;

    delete[] src;
    return ret;
}

void Shader::Destroy(Shader *shader)
{
    /* XXX: do nothing! the shader should remain in cache */
    (void)shader;
}

Shader::Shader(char const *vert, char const *frag)
  : data(new ShaderData())
{
#if defined USE_D3D9 || defined _XBOX
    ID3DXBuffer *shader_code, *error_msg;
    HRESULT hr;
    D3DXMACRO macros[] =
    {
#if defined _XBOX
        { "_XBOX", "1" },
#endif
        { NULL, NULL }
    };
#elif !defined __CELLOS_LV2__
    char buf[4096], errbuf[4096];
    char const *shader = buf;
    GLint status;
    GLsizei len;
#else
    /* Initialise the runtime shader compiler. FIXME: this needs only
     * to be done once. */
    cgRTCgcInit();
#endif

    /* Compile vertex shader */
    data->vert_crc = ShaderData::hash(vert);
#if defined USE_D3D9 || defined _XBOX
    hr = D3DXCompileShader(vert, (UINT)strlen(vert), macros, NULL, "main",
                           "vs_3_0", 0, &shader_code, &error_msg,
                           &data->vert_table);
    if (FAILED(hr))
    {
        Log::Error("failed to compile vertex shader: %s",
                   error_msg ? error_msg->GetBufferPointer() : "error");
        Log::Error("shader source:\n%s\n", vert);
    }
    g_d3ddevice->CreateVertexShader((DWORD *)shader_code->GetBufferPointer(),
                                    &data->vert_shader);
    shader_code->Release();
#elif !defined __CELLOS_LV2__
    ShaderData::Patch(buf, vert, NULL);
    data->vert_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(data->vert_id, 1, &shader, NULL);
    glCompileShader(data->vert_id);

    glGetShaderInfoLog(data->vert_id, sizeof(errbuf), &len, errbuf);
    glGetShaderiv(data->vert_id, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        Log::Error("failed to compile vertex shader: %s", errbuf);
        Log::Error("shader source:\n%s\n", buf);
    }
    else if (len > 16)
    {
        Log::Debug("compile log for vertex shader: %s", errbuf);
        Log::Debug("shader source:\n%s\n", buf);
    }
#else
    data->vert_id = cgCreateProgram(cgCreateContext(), CG_SOURCE, vert,
                                    cgGLGetLatestProfile(CG_GL_VERTEX),
                                    NULL, NULL);
    if (data->vert_id == NULL)
    {
        Log::Error("failed to compile vertex shader");
        Log::Error("shader source:\n%s\n", vert);
    }
#endif

    /* Compile fragment shader */
    data->frag_crc = ShaderData::hash(frag);
#if defined USE_D3D9 || defined _XBOX
    hr = D3DXCompileShader(frag, (UINT)strlen(frag), macros, NULL, "main",
                           "ps_3_0", 0, &shader_code, &error_msg,
                           &data->frag_table);
    if (FAILED(hr))
    {
        Log::Error("failed to compile fragment shader: %s",
                   error_msg ? error_msg->GetBufferPointer() : "error");
        Log::Error("shader source:\n%s\n", frag);
    }
    g_d3ddevice->CreatePixelShader((DWORD *)shader_code->GetBufferPointer(),
                                   &data->frag_shader);
    shader_code->Release();
#elif !defined __CELLOS_LV2__
    ShaderData::Patch(buf, NULL, frag);
    data->frag_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(data->frag_id, 1, &shader, NULL);
    glCompileShader(data->frag_id);

    glGetShaderInfoLog(data->frag_id, sizeof(errbuf), &len, errbuf);
    glGetShaderiv(data->frag_id, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        Log::Error("failed to compile fragment shader: %s", errbuf);
        Log::Error("shader source:\n%s\n", buf);
    }
    else if (len > 16)
    {
        Log::Debug("compile log for fragment shader: %s", errbuf);
        Log::Debug("shader source:\n%s\n", buf);
    }
#else
    data->frag_id = cgCreateProgram(cgCreateContext(), CG_SOURCE, frag,
                                    cgGLGetLatestProfile(CG_GL_FRAGMENT),
                                    NULL, NULL);
    if (data->frag_id == NULL)
    {
        Log::Error("failed to compile fragment shader");
        Log::Error("shader source:\n%s\n", frag);
    }
#endif

#if defined USE_D3D9 || defined _XBOX
    /* FIXME: this is only debug code, we don't need it. */
    D3DXCONSTANTTABLE_DESC desc;
    data->frag_table->GetDesc(&desc);
    for (int i = 0; i < desc.Constants; i++)
    {
        D3DXCONSTANT_DESC cdesc;
        UINT count = 1;
        D3DXHANDLE h = data->frag_table->GetConstant(NULL, i);
        data->frag_table->GetConstantDesc(h, &cdesc, &count);
    }
    data->vert_table->GetDesc(&desc);
    for (int i = 0; i < desc.Constants; i++)
    {
        D3DXCONSTANT_DESC cdesc;
        UINT count = 1;
        D3DXHANDLE h = data->vert_table->GetConstant(NULL, i);
        data->frag_table->GetConstantDesc(h, &cdesc, &count);
    }
#elif !defined __CELLOS_LV2__
    /* Create program */
    data->prog_id = glCreateProgram();
    glAttachShader(data->prog_id, data->vert_id);
    glAttachShader(data->prog_id, data->frag_id);

    glLinkProgram(data->prog_id);
    glGetProgramInfoLog(data->prog_id, sizeof(errbuf), &len, errbuf);
    glGetProgramiv(data->prog_id, GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
    {
        Log::Error("failed to link program: %s", errbuf);
    }
    else if (len > 16)
    {
        Log::Debug("link log for program: %s", errbuf);
    }
    glValidateProgram(data->prog_id);
#endif
}

ShaderAttrib Shader::GetAttribLocation(char const *attr,
                                       VertexUsage usage, int index) const
{
    ShaderAttrib ret;
    ret.m_flags = (uint64_t)(uint16_t)usage << 16;
    ret.m_flags |= (uint64_t)(uint16_t)index;
#if defined USE_D3D9 || defined _XBOX
#elif !defined __CELLOS_LV2__
    GLint l = glGetAttribLocation(data->prog_id, attr);
    if (l < 0)
    {
        Log::Warn("tried to query invalid attribute: %s\n", attr);
        l = 0;
    }
    ret.m_flags |= (uint64_t)(uint32_t)l << 32;
#else
    /* FIXME: can we do this at all on the PS3? */
#endif
    return ret;
}

ShaderUniform Shader::GetUniformLocation(char const *uni) const
{
    ShaderUniform ret;
#if defined USE_D3D9 || defined _XBOX
    /* Global variables are prefixed with "$" */
    char tmpname[128];
    sprintf(tmpname, "$%s", uni);
    D3DXCONSTANT_DESC cdesc;
    D3DXHANDLE hr;
    UINT count;

    count = 0;
    hr = data->frag_table->GetConstantByName(NULL, tmpname);
    if (hr)
        data->frag_table->GetConstantDesc(hr, &cdesc, &count);
    if (count)
    {
        ret.frag = cdesc.RegisterIndex;
        ret.flags |= 1;
    }

    count = 0;
    hr = data->vert_table->GetConstantByName(NULL, tmpname);
    if (hr)
        data->vert_table->GetConstantDesc(hr, &cdesc, &count);
    if (count)
    {
        ret.vert = cdesc.RegisterIndex;
        ret.flags |= 2;
    }
#elif !defined __CELLOS_LV2__
    ret.frag = (uintptr_t)glGetUniformLocation(data->prog_id, uni);
    ret.vert = 0;
#else
    ret.frag = (uintptr_t)cgGetNamedParameter(data->frag_id, uni);
    ret.vert = (uintptr_t)cgGetNamedParameter(data->vert_id, uni);
#endif
    return ret;
}

/*
 * Uniform setters for scalars
 */

void Shader::SetUniform(ShaderUniform const &uni, int i)
{
#if defined USE_D3D9 || defined _XBOX
    SetUniform(uni, ivec4(i, 0, 0, 0));
#elif !defined __CELLOS_LV2__
    glUniform1i(uni.frag, i);
#else
    /* FIXME: does this exist at all? cgGLSetParameter1i doesn't. */
#endif
}

void Shader::SetUniform(ShaderUniform const &uni, ivec2 const &v)
{
#if defined USE_D3D9 || defined _XBOX
    SetUniform(uni, ivec4(v, 0, 0));
#elif !defined __CELLOS_LV2__
    glUniform2i(uni.frag, v.x, v.y);
#else
    /* FIXME: does this exist at all? */
#endif
}

void Shader::SetUniform(ShaderUniform const &uni, ivec3 const &v)
{
#if defined USE_D3D9 || defined _XBOX
    SetUniform(uni, ivec4(v, 0));
#elif !defined __CELLOS_LV2__
    glUniform3i(uni.frag, v.x, v.y, v.z);
#else
    /* FIXME: does this exist at all? */
#endif
}

void Shader::SetUniform(ShaderUniform const &uni, ivec4 const &v)
{
#if defined USE_D3D9 || defined _XBOX
    if (uni.flags & 1)
        g_d3ddevice->SetPixelShaderConstantI((UINT)uni.frag, &v[0], 1);
    if (uni.flags & 2)
        g_d3ddevice->SetVertexShaderConstantI((UINT)uni.vert, &v[0], 1);
#elif !defined __CELLOS_LV2__
    glUniform4i(uni.frag, v.x, v.y, v.z, v.w);
#else
    /* FIXME: does this exist at all? */
#endif
}

void Shader::SetUniform(ShaderUniform const &uni, float f)
{
#if defined USE_D3D9 || defined _XBOX
    SetUniform(uni, vec4(f, 0, 0, 0));
#elif !defined __CELLOS_LV2__
    glUniform1f(uni.frag, f);
#else
    if (uni.frag)
        cgGLSetParameter1f((CGparameter)uni.frag, f);
    if (uni.vert)
        cgGLSetParameter1f((CGparameter)uni.vert, f);
#endif
}

void Shader::SetUniform(ShaderUniform const &uni, vec2 const &v)
{
#if defined USE_D3D9 || defined _XBOX
    SetUniform(uni, vec4(v, 0, 0));
#elif !defined __CELLOS_LV2__
    glUniform2fv(uni.frag, 1, &v[0]);
#else
    if (uni.frag)
        cgGLSetParameter2fv((CGparameter)uni.frag, &v[0]);
    if (uni.vert)
        cgGLSetParameter2fv((CGparameter)uni.vert, &v[0]);
#endif
}

void Shader::SetUniform(ShaderUniform const &uni, vec3 const &v)
{
#if defined USE_D3D9 || defined _XBOX
    SetUniform(uni, vec4(v, 0));
#elif !defined __CELLOS_LV2__
    glUniform3fv(uni.frag, 1, &v[0]);
#else
    if (uni.frag)
        cgGLSetParameter3fv((CGparameter)uni.frag, &v[0]);
    if (uni.vert)
        cgGLSetParameter3fv((CGparameter)uni.vert, &v[0]);
#endif
}

void Shader::SetUniform(ShaderUniform const &uni, vec4 const &v)
{
#if defined USE_D3D9 || defined _XBOX
    if (uni.flags & 1)
        g_d3ddevice->SetPixelShaderConstantF((UINT)uni.frag, &v[0], 1);
    if (uni.flags & 2)
        g_d3ddevice->SetVertexShaderConstantF((UINT)uni.vert, &v[0], 1);
#elif !defined __CELLOS_LV2__
    glUniform4fv(uni.frag, 1, &v[0]);
#else
    if (uni.frag)
        cgGLSetParameter4fv((CGparameter)uni.frag, &v[0]);
    if (uni.vert)
        cgGLSetParameter4fv((CGparameter)uni.vert, &v[0]);
#endif
}

void Shader::SetUniform(ShaderUniform const &uni, mat2 const &m)
{
#if defined USE_D3D9 || defined _XBOX
    /* FIXME: do we need padding here like for the mat3 version? */
    if (uni.flags & 1)
        g_d3ddevice->SetPixelShaderConstantF((UINT)uni.frag, &m[0][0], 1);
    if (uni.flags & 2)
        g_d3ddevice->SetVertexShaderConstantF((UINT)uni.vert, &m[0][0], 1);
#elif !defined __CELLOS_LV2__
    glUniformMatrix2fv(uni.frag, 1, GL_FALSE, &m[0][0]);
#else
    mat4 tmp(m, 1.0f, 1.0f);
    if (uni.frag)
        cgGLSetMatrixParameterfc((CGparameter)uni.frag, &m[0][0]);
    if (uni.vert)
        cgGLSetMatrixParameterfc((CGparameter)uni.vert, &m[0][0]);
#endif
}

void Shader::SetUniform(ShaderUniform const &uni, mat3 const &m)
{
#if defined USE_D3D9 || defined _XBOX
    /* Padding matrix columns is necessary on DirectX. We need to create
     * a new data structure; a 4×4 matrix will do. */
    mat4 tmp(m, 1.0f);
    if (uni.flags & 1)
        g_d3ddevice->SetPixelShaderConstantF((UINT)uni.frag, &tmp[0][0], 3);
    if (uni.flags & 2)
        g_d3ddevice->SetVertexShaderConstantF((UINT)uni.vert, &tmp[0][0], 3);
#elif !defined __CELLOS_LV2__
    glUniformMatrix3fv(uni.frag, 1, GL_FALSE, &m[0][0]);
#else
    /* FIXME: check it's the proper way to do this */
    mat4 tmp(m, 1.0f);
    if (uni.frag)
        cgGLSetMatrixParameterfc((CGparameter)uni.frag, &m[0][0]);
    if (uni.vert)
        cgGLSetMatrixParameterfc((CGparameter)uni.vert, &m[0][0]);
#endif
}

void Shader::SetUniform(ShaderUniform const &uni, mat4 const &m)
{
#if defined USE_D3D9 || defined _XBOX
    if (uni.flags & 1)
        g_d3ddevice->SetPixelShaderConstantF((UINT)uni.frag, &m[0][0], 4);
    if (uni.flags & 2)
        g_d3ddevice->SetVertexShaderConstantF((UINT)uni.vert, &m[0][0], 4);
#elif !defined __CELLOS_LV2__
    glUniformMatrix4fv(uni.frag, 1, GL_FALSE, &m[0][0]);
#else
    if (uni.frag)
        cgGLSetMatrixParameterfc((CGparameter)uni.frag, &m[0][0]);
    if (uni.vert)
        cgGLSetMatrixParameterfc((CGparameter)uni.vert, &m[0][0]);
#endif
}

void Shader::SetUniform(ShaderUniform const &uni, ShaderTexture tex, int index)
{
#if defined USE_D3D9 || defined _XBOX
    g_d3ddevice->SetTexture(index, (LPDIRECT3DTEXTURE9)tex.m_flags);
#elif !defined __CELLOS_LV2__
    glActiveTexture(GL_TEXTURE0 + index);
    //glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, (int)tex.m_flags);
    SetUniform(uni, index);
#else
    /* FIXME: unimplemented */
#endif
}

/*
 * Uniform setters for arrays
 */

void Shader::SetUniform(ShaderUniform const &uni, Array<float> const &v)
{
#if defined USE_D3D9 || defined _XBOX
    /* FIXME: this will not work properly because we don't know how tell DX9
     * it's a bunch of floats instead of vec4. */
    if (uni.flags & 1)
        g_d3ddevice->SetPixelShaderConstantF((UINT)uni.frag,
                                             &v[0], v.Count() / 4);
    if (uni.flags & 2)
        g_d3ddevice->SetVertexShaderConstantF((UINT)uni.vert,
                                              &v[0], v.Count() / 4);
#elif !defined __CELLOS_LV2__
    glUniform1fv(uni.frag, v.Count(), &v[0]);
#else
    if (uni.frag)
        cgGLSetParameterArray1f((CGparameter)uni.frag,
                                0, v.Count(), &v[0]);
    if (uni.vert)
        cgGLSetParameterArray1f((CGparameter)uni.vert,
                                0, v.Count(), &v[0]);
#endif
}

void Shader::SetUniform(ShaderUniform const &uni, Array<vec2> const &v)
{
#if defined USE_D3D9 || defined _XBOX
    /* FIXME: this will not work properly because we don't know how tell DX9
     * it's a bunch of vec2 instead of vec4. */
    if (uni.flags & 1)
        g_d3ddevice->SetPixelShaderConstantF((UINT)uni.frag,
                                             &v[0][0], v.Count() / 2);
    if (uni.flags & 2)
        g_d3ddevice->SetVertexShaderConstantF((UINT)uni.vert,
                                              &v[0][0], v.Count() / 2);
#elif !defined __CELLOS_LV2__
    glUniform2fv(uni.frag, v.Count(), &v[0][0]);
#else
    if (uni.frag)
        cgGLSetParameterArray2f((CGparameter)uni.frag,
                                0, v.Count(), &v[0][0]);
    if (uni.vert)
        cgGLSetParameterArray2f((CGparameter)uni.vert,
                                0, v.Count(), &v[0][0]);
#endif
}

void Shader::SetUniform(ShaderUniform const &uni, Array<vec3> const &v)
{
#if defined USE_D3D9 || defined _XBOX
    /* FIXME: this will not work properly because we don't know how tell DX9
     * it's a bunch of vec3 instead of vec4. */
    if (uni.flags & 1)
        g_d3ddevice->SetPixelShaderConstantF((UINT)uni.frag,
                                             &v[0][0], v.Count());
    if (uni.flags & 2)
        g_d3ddevice->SetVertexShaderConstantF((UINT)uni.vert,
                                              &v[0][0], v.Count());
#elif !defined __CELLOS_LV2__
    glUniform3fv(uni.frag, v.Count(), &v[0][0]);
#else
    if (uni.frag)
        cgGLSetParameterArray3f((CGparameter)uni.frag,
                                0, v.Count(), &v[0][0]);
    if (uni.vert)
        cgGLSetParameterArray3f((CGparameter)uni.vert,
                                0, v.Count(), &v[0][0]);
#endif
}

void Shader::SetUniform(ShaderUniform const &uni, Array<vec4> const &v)
{
#if defined USE_D3D9 || defined _XBOX
    if (uni.flags & 1)
        g_d3ddevice->SetPixelShaderConstantF((UINT)uni.frag,
                                             &v[0][0], v.Count());
    if (uni.flags & 2)
        g_d3ddevice->SetVertexShaderConstantF((UINT)uni.vert,
                                              &v[0][0], v.Count());
#elif !defined __CELLOS_LV2__
    glUniform4fv(uni.frag, v.Count(), &v[0][0]);
#else
    if (uni.frag)
        cgGLSetParameterArray4f((CGparameter)uni.frag,
                                0, v.Count(), &v[0][0]);
    if (uni.vert)
        cgGLSetParameterArray4f((CGparameter)uni.vert,
                                0, v.Count(), &v[0][0]);
#endif
}

void Shader::Bind() const
{
#if defined USE_D3D9 || defined _XBOX
    HRESULT hr;
    hr = g_d3ddevice->SetVertexShader(data->vert_shader);
    hr = g_d3ddevice->SetPixelShader(data->frag_shader);
#elif !defined __CELLOS_LV2__
    glUseProgram(data->prog_id);
#else
    cgGLEnableProfile(cgGLGetLatestProfile(CG_GL_VERTEX));
    cgGLBindProgram(data->vert_id);
    cgGLEnableProfile(cgGLGetLatestProfile(CG_GL_FRAGMENT));
    cgGLBindProgram(data->frag_id);
#endif
}

void Shader::Unbind() const
{
#if defined USE_D3D9 || defined _XBOX
    HRESULT hr;
    hr = g_d3ddevice->SetVertexShader(NULL);
    hr = g_d3ddevice->SetPixelShader(NULL);
#elif !defined __CELLOS_LV2__
    /* FIXME: untested */
    glUseProgram(0);
#else
    /* FIXME: untested */
    cgGLDisableProfile(cgGLGetLatestProfile(CG_GL_VERTEX));
    cgGLDisableProfile(cgGLGetLatestProfile(CG_GL_FRAGMENT));
#endif
}

Shader::~Shader()
{
#if defined USE_D3D9 || defined _XBOX
    data->vert_shader->Release();
    data->vert_table->Release();
    data->frag_shader->Release();
    data->frag_table->Release();
#elif !defined __CELLOS_LV2__
    glDetachShader(data->prog_id, data->vert_id);
    glDetachShader(data->prog_id, data->frag_id);
    glDeleteShader(data->vert_id);
    glDeleteShader(data->frag_id);
    glDeleteProgram(data->prog_id);
#else
    cgDestroyProgram(data->vert_id);
    cgDestroyProgram(data->frag_id);
#endif
    delete data;
}

/* Try to detect shader compiler features */
int ShaderData::GetVersion()
{
    static int version = 0;

#if !defined USE_D3D9 && !defined _XBOX && !defined __CELLOS_LV2__
    if (!version)
    {
#if defined HAVE_GLES_2X
        /* GLES 2.x supports #version 100, that's all. */
        return 100;
#else
        char buf[4096];
        GLsizei len;

        int id = glCreateShader(GL_VERTEX_SHADER);

        /* Can we compile 1.30 shaders? */
        char const *test130 =
            "#version 130\n"
            "void main() { gl_Position = vec4(0.0, 0.0, 0.0, 0.0); }";
        glShaderSource(id, 1, &test130, NULL);
        glCompileShader(id);
        glGetShaderInfoLog(id, sizeof(buf), &len, buf);
        if (len <= 0)
            version = 130;

        /* If not, can we compile 1.20 shaders? */
        if (!version)
        {
            char const *test120 =
                "#version 120\n"
                "void main() { gl_Position = vec4(0.0, 0.0, 0.0, 0.0); }";
            glShaderSource(id, 1, &test120, NULL);
            glCompileShader(id);
            glGetShaderInfoLog(id, sizeof(buf), &len, buf);
            if (len <= 0)
                version = 120;
        }

        /* Otherwise, assume we can compile 1.10 shaders. */
        if (!version)
            version = 110;

        glDeleteShader(id);
#endif
    }
#endif

    return version;
}

/* Simple shader source patching for old GLSL versions.
 */
void ShaderData::Patch(char *dst, char const *vert, char const *frag)
{
    int ver_driver = GetVersion();

    strcpy(dst, vert ? vert : frag);
    if (ver_driver >= 130)
        return;

    int ver_shader = 110;
    char *parser = strstr(dst, "#version");
    if (parser)
        ver_shader = atoi(parser + strlen("#version"));

    /* This is GL ES, we only know version 100. */
    if (ver_shader > 100 && ver_driver == 100)
    {
        /* FIXME: this isn't elegant but honestly, we don't care, this
         * whole file is going to die soon. */
        char *p = strstr(dst, "#version");
        if (p)
        {
            p += 8;
            while (*p == ' ')
                p++;
            if (p[0] == '1' && p[1] && p[2])
                p[1] = p[2] = '0';
        }
    }

    if (ver_shader > 120 && ver_driver <= 120)
    {
        char const *end = dst + strlen(dst) + 1;

        /* Find main() */
        parser = strstr(dst, "main");
        if (!parser) return;
        parser = strstr(parser, "(");
        if (!parser) return;
        parser = strstr(parser, ")");
        if (!parser) return;
        parser = strstr(parser, "{");
        if (!parser) return;
        char *main = parser + 1;

        /* Perform main() replaces */
        char const * const main_replaces[] =
        {
#if 0
            "in vec2 in_Vertex;", "vec2 in_Vertex = gl_Vertex.xy;",
            "in vec3 in_Vertex;", "vec3 in_Vertex = gl_Vertex.xyz;",
            "in vec4 in_Vertex;", "vec4 in_Vertex = gl_Vertex.xyzw;",

            "in vec2 in_Color;", "vec2 in_Color = gl_Color.xy;",
            "in vec3 in_Color;", "vec3 in_Color = gl_Color.xyz;",
            "in vec4 in_Color;", "vec4 in_Color = gl_Color.xyzw;",

            "in vec2 in_MultiTexCoord0;",
               "vec2 in_MultiTexCoord0 = gl_MultiTexCoord0.xy;",
            "in vec2 in_MultiTexCoord1;",
               "vec2 in_MultiTexCoord1 = gl_MultiTexCoord1.xy;",
            "in vec2 in_MultiTexCoord2;",
               "vec2 in_MultiTexCoord2 = gl_MultiTexCoord2.xy;",
            "in vec2 in_MultiTexCoord3;",
               "vec2 in_MultiTexCoord3 = gl_MultiTexCoord3.xy;",
            "in vec2 in_MultiTexCoord4;",
               "vec2 in_MultiTexCoord4 = gl_MultiTexCoord4.xy;",
            "in vec2 in_MultiTexCoord5;",
               "vec2 in_MultiTexCoord5 = gl_MultiTexCoord5.xy;",
            "in vec2 in_MultiTexCoord6;",
               "vec2 in_MultiTexCoord6 = gl_MultiTexCoord6.xy;",
            "in vec2 in_MultiTexCoord7;",
               "vec2 in_MultiTexCoord7 = gl_MultiTexCoord7.xy;",
#endif

            NULL
        };

        for (char const * const *rep = main_replaces; rep[0]; rep += 2)
        {
            char *match = strstr(dst, rep[0]);
            if (match && match < main)
            {
                size_t l0 = strlen(rep[0]);
                size_t l1 = strlen(rep[1]);
                memmove(main + l1, main, end - main);
                memcpy(main, rep[1], l1);
                memset(match, ' ', l0);
                main += l1;
                end += l1;
            }
        }

        /* Perform small replaces */
        char const * const fast_replaces[] =
        {
            "#version 130", "#version 120",
            "in vec2", vert ? "attribute vec2" : "varying vec2",
            "in vec3", vert ? "attribute vec3" : "varying vec3",
            "in vec4", vert ? "attribute vec4" : "varying vec4",
            "in mat4", vert ? "attribute mat4" : "varying mat4",
            "out vec2", "varying vec2",
            "out vec3", "varying vec3",
            "out vec4", "varying vec4",
            "out mat4", "varying mat4",
            NULL
        };

        for (char const * const *rep = fast_replaces; rep[0]; rep += 2)
        {
            char *match;
            while ((match = strstr(dst, rep[0])))
            {
                size_t l0 = strlen(rep[0]);
                size_t l1 = strlen(rep[1]);

                if (l1 > l0)
                    memmove(match + l1, match + l0, (end - match) - l0);
                memcpy(match, rep[1], l1);
                if (l1 < l0)
                    memset(match + l0, ' ', l1 - l0);
                end += l1 - l0;
            }
        }
    }
}

} /* namespace lol */

