#!/usr/bin/env python3
"""Esia shader build: one HLSL source -> every backend's shader format.

    HLSL (src/esia/shaders/*.hlsl)
      |-- glslang (HLSL front end) --> SPIR-V ------------------------------> spirv    (Vulkan)
      |                                  |-- SPIRV-Cross ------------------> glsl330  (OpenGL 3.3 core)
      |                                  |-- SPIRV-Cross ------------------> essl300  (OpenGL ES 3.0 / WebGL 2)
      |                                  '-- SPIRV-Cross ------------------> msl      (Metal 2.0)
      |-- fxc (Windows, or under Wine) --------------------------------------> dxbc_sm5 (D3D11 / D3D12), dxbc_sm4 (D3D10)
      '-- DXC (optional) ----------------------------------------------------> dxil     (D3D12 SM 6)

The output is checked in, so backends build without any shader tool: src/esia/shaders/generated/<format>/
<Program>.<vs|ps>.<ext> (readable GLSL / MSL, binary SPIR-V / DXBC / DXIL) and esia_shader_table.cpp (the
lookup table with sizes and GL sampler bindings). CMake embeds the files at build time (cmake/EsiaEmbed.cmake).
Run this script after changing a shader; the generated files must be committed with the change:

    python3 tools/shaders/build_shaders.py                      # SPIR-V, GLSL, ESSL, MSL (+ HLSL sources)
    python3 tools/shaders/build_shaders.py --fxc fxc.exe        # + DXBC (Windows SDK fxc)
    python3 tools/shaders/build_shaders.py --fxc "wine fxc.exe" # + DXBC on Linux / macOS through Wine
    python3 tools/shaders/build_shaders.py --dxc dxc            # + DXIL
    python3 tools/shaders/build_shaders.py --check              # regenerate into a temp dir and diff (CI)

Formats a run cannot produce keep their previously generated file (the script never deletes a format because a
tool is missing). Requirements: glslangValidator, spirv-val, spirv-cross (Ubuntu: glslang-tools spirv-tools
spirv-cross; macOS: brew install glslang spirv-tools spirv-cross).
"""
import argparse
import json
import os
import re
import shlex
import shutil
import struct
import subprocess
import sys
import tempfile

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
SRC = os.path.join(ROOT, 'src', 'esia', 'shaders')
OUT = os.path.join(SRC, 'generated')

# rhi::ShaderProgram order (include/esia/rhi/rhi.hpp): name -> (file, vertex entry, pixel entry)
PROGRAMS = [
    ('UiGeometry', 'esia_ui.hlsl', 'UiVS', 'UiPS'),
    ('TextGray', 'esia_ui.hlsl', 'UiVS', 'TextGrayPS'),
    ('TextLcd', 'esia_ui.hlsl', 'UiVS', 'TextLcdPS'),
    ('TextLcdGray', 'esia_ui.hlsl', 'UiVS', 'TextLcdGrayPS'),
    ('Fx', 'esia_fx.hlsl', 'FxVS', 'FxPS'),
    ('Downsample', 'esia_post.hlsl', 'FullscreenVS', 'DownsamplePS'),
    ('LayerComposite', 'esia_post.hlsl', 'FullscreenVS', 'LayerCompositePS'),
    ('Clear', 'esia_post.hlsl', 'FullscreenVS', 'ClearPS'),
]
DUAL_SOURCE = {'TextLcdPS'}

# shaders::Format order (include/esia/render/shader_library.hpp): name -> FX instance storage
FORMATS = [
    ('spirv', 'buffer'),
    ('glsl330', 'texture'),
    ('essl300', 'texture'),
    ('msl', 'buffer'),
    ('dxbc_sm5', 'buffer'),
    ('dxbc_sm4', 'texture'),
    ('dxil', 'buffer'),
]
TEXTURE_SLOTS = {'gTex': 0, 'gBackdrop0': 1, 'gBackdrop1': 2, 'gBackdrop2': 3, 'gBackdrop3': 4, 'gBackdrop4': 5,
                 'gBackdrop5': 6, 'gFxData': 7}
SAMPLERS = {'gLinear': 0, 'gPoint': 1}


def run(cmd, **kw):
    r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, **kw)
    if r.returncode != 0:
        sys.exit('command failed: %s\n%s' % (' '.join(cmd), r.stdout))
    return r.stdout


def tool(name, override=None):
    if override:
        return shlex.split(override)
    path = shutil.which(name)
    return [path] if path else None


# --------------------------------------------------------------------------------------------- SPIR-V
def compile_spirv(glslang, file, entry, stage, storage, tmp):
    out = os.path.join(tmp, '%s_%s_%s.spv' % (os.path.splitext(file)[0], entry, storage))
    cmd = glslang + ['-D', '-V', '--target-env', 'vulkan1.0', '-S', 'vert' if stage == 'vs' else 'frag', '-e', entry,
                     '-DESIA_SPIRV=1', '-o', out, os.path.join(SRC, file)]
    if storage == 'texture':
        cmd.insert(-1, '-DESIA_FX_STORAGE_TEXTURE=1')
    run(cmd)
    data = open(out, 'rb').read()
    if entry in DUAL_SOURCE:
        data = dual_source(data)
        open(out, 'wb').write(data)
    return out, data


def dual_source(spv):
    """SV_Target1 -> Location 0, Index 1 (glslang has no [[vk::index]]): dual-source blending on attachment 0."""
    words = list(struct.unpack('<%dI' % (len(spv) // 4), spv))
    outputs = set()
    i = 5
    while i < len(words):   # OpVariable (59) in the Output storage class (3)
        n, op = words[i] >> 16, words[i] & 0xFFFF
        if op == 59 and words[i + 3] == 3:
            outputs.add(words[i + 2])
        i += n
    res, i, patched = words[:5], 5, False
    while i < len(words):
        n, op = words[i] >> 16, words[i] & 0xFFFF
        ins = words[i:i + n]
        if op == 71 and n == 4 and ins[2] == 30 and ins[3] == 1 and ins[1] in outputs:   # OpDecorate Location 1
            res += [ins[0], ins[1], 30, 0]
            res += [(4 << 16) | 71, ins[1], 32, 1]                                         # OpDecorate Index 1
            patched = True
        else:
            res += ins
        i += n
    if not patched:
        sys.exit('dual-source patch: no Location 1 output found')
    return struct.pack('<%dI' % len(res), *res)


# --------------------------------------------------------------------------------------------- SPIRV-Cross
def cross(spirv_cross, spv, fmt, stage, entry, tmp):
    base = [spirv_cross[0]] if len(spirv_cross) == 1 else spirv_cross
    if fmt == 'msl':
        out = run(base + [spv, '--msl', '--msl-version', '20000', '--msl-decoration-binding',
                          '--rename-entry-point', entry, 'esia_main', 'vert' if stage == 'vs' else 'frag'])
        return out
    args = [spv, '--no-420pack-extension', '--no-support-nonzero-baseinstance', '--remove-unused-variables']
    args += ['--es', '--version', '300'] if fmt == 'essl300' else ['--version', '330', '--no-es']
    # GL 3.3 / GLES 3 link varyings by name: give both stages the same name per location
    refl = json.loads(run(base + [spv, '--reflect']))
    key = 'outputs' if stage == 'vs' else 'inputs'
    for v in refl.get(key, []):
        if 'location' in v:
            args += ['--rename-interface-variable', 'out' if stage == 'vs' else 'in', str(v['location']), 'esia_v%d' % v['location']]
    text = run(base + args)
    if fmt == 'essl300' and 'index = 1' in text:
        # dual-source blending on GLES needs EXT_blend_func_extended (the backend only builds TextLcd with it)
        text = text.replace('#version 300 es\n', '#version 300 es\n#extension GL_EXT_blend_func_extended : require\n', 1)
    return text


def glsl_bindings(src):
    """Combined samplers SPIRV-Cross created: GL name -> (texture slot, sampler: 0 linear / 1 point)."""
    res = []
    for name in re.findall(r'uniform (?:highp |mediump |lowp )?sampler2D (\w+);', src):
        m = re.match(r'SPIRV_Cross_Combined(g\w+?)(gLinear|gPoint|SPIRV_Cross_DummySampler)$', name)
        if not m or m.group(1) not in TEXTURE_SLOTS:
            sys.exit('unexpected sampler %s' % name)
        res.append((name, TEXTURE_SLOTS[m.group(1)], SAMPLERS.get(m.group(2), 1)))
    return res


def validate_glsl(glslang, text, stage, fmt, tmp):
    path = os.path.join(tmp, 'check.' + ('vert' if stage == 'vs' else 'frag'))
    open(path, 'w').write(text)
    run(glslang + [path])


# --------------------------------------------------------------------------------------------- D3D
def compile_fxc(fxc, file, entry, stage, profile, storage, tmp):
    out = os.path.join(tmp, '%s_%s.dxbc' % (entry, profile))
    defines = ['/DESIA_FX_STORAGE_TEXTURE=1'] if storage == 'texture' else []
    src = os.path.join(SRC, file)
    if fxc[0].endswith('wine') or os.path.basename(fxc[0]) == 'wine':
        src = run(['winepath', '-w', src]).strip()
        out_arg = run(['winepath', '-w', out]).strip()
    else:
        out_arg = out
    run(fxc + ['/nologo', '/O3', '/Ges', '/T', '%s_%s' % (stage, profile), '/E', entry] + defines + ['/Fo', out_arg, src])
    return open(out, 'rb').read()


def compile_dxc(dxc, file, entry, stage, tmp):
    out = os.path.join(tmp, '%s.dxil' % entry)
    run(dxc + ['-nologo', '-O3', '-T', '%s_6_0' % stage, '-E', entry, '-Fo', out, os.path.join(SRC, file)])
    return open(out, 'rb').read()


# --------------------------------------------------------------------------------------------- output
EXT = {'spirv': 'spv', 'glsl330': 'glsl', 'essl300': 'essl', 'msl': 'metal', 'dxbc_sm5': 'dxbc', 'dxbc_sm4': 'dxbc', 'dxil': 'dxil'}
PROGRAM_NAMES = [p[0] for p in PROGRAMS]


def blob_file(fmt, prog, stage):
    return '%s.%s.%s' % (PROGRAM_NAMES[prog], 'vs' if stage == 0 else 'ps', EXT[fmt])


def blob_symbol(fmt, prog, stage):
    # must match cmake/EsiaEmbed.cmake: <format dir>_<file name with non-identifier characters as _>
    return re.sub(r'[^A-Za-z0-9_]', '_', '%s_%s' % (fmt, blob_file(fmt, prog, stage)))


def write_files(fmt, entries, out_dir):
    """entries: list of (program index, stage, entry, bytes or text, is_text, bindings). Replaces the format's directory."""
    d = os.path.join(out_dir, fmt)
    if os.path.isdir(d):
        shutil.rmtree(d)
    os.makedirs(d)
    for prog, stage, entry, data, text, binds in entries:
        mode = 'w' if text else 'wb'
        with open(os.path.join(d, blob_file(fmt, prog, stage)), mode, newline='\n' if text else None) as f:
            f.write(data)


def read_existing(fmt, out_dir):
    """Entries of a format generated by an earlier run (a tool this run lacks, e.g. fxc): kept as they are."""
    d = os.path.join(out_dir, fmt)
    res = []
    if not os.path.isdir(d):
        return res
    for pi, (prog, file, vs, ps) in enumerate(PROGRAMS):
        for st, entry in ((0, vs), (1, ps)):
            path = os.path.join(d, blob_file(fmt, pi, st))
            if os.path.exists(path):
                res.append((pi, st, entry, open(path, 'rb').read(), False, []))
    return res


def write_table(produced, out_dir):
    """The lookup table (C++): which blob serves which program / stage, sizes and GL sampler bindings. The blob
    bytes themselves are embedded at build time from the files next to it (cmake/EsiaEmbed.cmake)."""
    lines = ['// Generated by tools/shaders/build_shaders.py - do not edit.',
             '// Shader blobs live in src/esia/shaders/generated/<format>/ and are embedded at build time.',
             '#include "esia/render/shader_library.hpp"', '', 'namespace esia::shaders', '{', '    namespace blobs', '    {']
    for fmt, _ in FORMATS:
        for prog, stage, entry, data, text, binds in produced[fmt]:
            lines.append('        extern const unsigned char %s[];' % blob_symbol(fmt, prog, stage))
    lines += ['    }', '', '    namespace', '    {']
    for fmt, _ in FORMATS:
        for prog, stage, entry, data, text, binds in produced[fmt]:
            if binds:
                lines.append('        const TextureBinding kBind_%s[] = {%s};' % (blob_symbol(fmt, prog, stage),
                             ', '.join('{"%s", %d, %d}' % b for b in binds)))
    lines += ['    }', '']
    for fmt, storage in FORMATS:
        lines.append('    // %s: FX instances in %s storage' % (fmt, storage))
        lines.append('    extern const ShaderBlob kLibrary_%s[] = {' % fmt)
        for prog, stage, entry, data, text, binds in produced[fmt]:
            sym = blob_symbol(fmt, prog, stage)
            size = len(data.encode('utf-8')) if text else len(data)
            lines.append('        {%d, %d, "%s", blobs::%s, %d, %s, %s, %d},' % (prog, stage, entry, sym, size, 'true' if text else 'false',
                         ('kBind_' + sym) if binds else 'nullptr', len(binds)))
        lines += ['        {-1, 0, nullptr, nullptr, 0, false, nullptr, 0},', '    };', '']
    lines += ['}', '']
    with open(os.path.join(out_dir, 'esia_shader_table.cpp'), 'w', newline='\n') as f:
        f.write('\n'.join(lines))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--glslang', help='glslangValidator command')
    ap.add_argument('--spirv-cross', help='spirv-cross command')
    ap.add_argument('--fxc', help='fxc command (e.g. "fxc.exe" or "wine /path/fxc.exe"): adds dxbc_sm5 / dxbc_sm4')
    ap.add_argument('--dxc', help='dxc command: adds dxil')
    ap.add_argument('--out', default=OUT)
    ap.add_argument('--check', action='store_true', help='generate into a temp dir and compare with the checked-in files')
    args = ap.parse_args()

    glslang = tool('glslangValidator', args.glslang)
    spirv_cross = tool('spirv-cross', args.spirv_cross)
    spirv_val = tool('spirv-val')
    if not glslang or not spirv_cross:
        sys.exit('glslangValidator and spirv-cross are required (see the docstring)')
    fxc = tool('fxc', args.fxc) if args.fxc else None
    dxc = tool('dxc', args.dxc) if args.dxc else None

    out_dir = tempfile.mkdtemp() if args.check else args.out
    os.makedirs(out_dir, exist_ok=True)
    with tempfile.TemporaryDirectory() as tmp:
        produced = {f: [] for f, _ in FORMATS}
        spv_cache = {}
        for pi, (prog, file, vs, ps) in enumerate(PROGRAMS):
            for stage, entry in (('vs', vs), ('ps', ps)):
                st = 0 if stage == 'vs' else 1
                for storage in ('buffer', 'texture'):
                    key = (file, entry, storage)
                    if key not in spv_cache:
                        path, data = compile_spirv(glslang, file, entry, stage, storage, tmp)
                        if spirv_val:
                            run(spirv_val + [path])
                        spv_cache[key] = (path, data)
                path_b, data_b = spv_cache[(file, entry, 'buffer')]
                path_t, _ = spv_cache[(file, entry, 'texture')]
                produced['spirv'].append((pi, st, entry, data_b, False, []))
                for fmt in ('glsl330', 'essl300'):
                    text = cross(spirv_cross, path_t, fmt, stage, entry, tmp)
                    validate_glsl(glslang, text, stage, fmt, tmp)
                    produced[fmt].append((pi, st, entry, text, True, glsl_bindings(text)))
                produced['msl'].append((pi, st, entry, cross(spirv_cross, path_b, 'msl', stage, entry, tmp), True, []))
                if fxc:
                    produced['dxbc_sm5'].append((pi, st, entry, compile_fxc(fxc, file, entry, stage, '5_0', 'buffer', tmp), False, []))
                    produced['dxbc_sm4'].append((pi, st, entry, compile_fxc(fxc, file, entry, stage, '4_0', 'texture', tmp), False, []))
                if dxc:
                    produced['dxil'].append((pi, st, entry, compile_dxc(dxc, file, entry, stage, tmp), False, []))
        for fmt, storage in FORMATS:
            if produced[fmt]:
                write_files(fmt, produced[fmt], out_dir)
            else:
                produced[fmt] = read_existing(fmt, args.out)
                if produced[fmt]:
                    write_files(fmt, produced[fmt], out_dir)
        write_table(produced, out_dir)

    if args.check:
        bad = []
        for dirpath, _, files in os.walk(out_dir):
            for name in files:
                rel = os.path.relpath(os.path.join(dirpath, name), out_dir)
                ref = os.path.join(args.out, rel)
                if not os.path.exists(ref) or open(ref, 'rb').read() != open(os.path.join(dirpath, name), 'rb').read():
                    bad.append(rel)
        shutil.rmtree(out_dir)
        if bad:
            sys.exit('generated shaders are out of date: %s (run tools/shaders/build_shaders.py)' % ', '.join(sorted(bad)))
        print('generated shaders are up to date')
    else:
        print('wrote %s' % out_dir)


if __name__ == '__main__':
    main()
