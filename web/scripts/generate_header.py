#!/usr/bin/env python3
"""
将 web/dist/index.html 压缩为 gzip，并生成可嵌入 ESP32 PROGMEM 的 C 头文件。
用法: python scripts/generate_header.py
"""
import gzip
import os
import sys

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)  # web/
    input_file = os.path.join(project_root, 'dist', 'index.html')
    output_file = os.path.join(project_root, '..', 'src', 'webpage.h')

    if not os.path.exists(input_file):
        print(f'错误: 找不到 {input_file}')
        print('请先运行 npm run build')
        sys.exit(1)

    with open(input_file, 'rb') as f:
        content = f.read()

    compressed = gzip.compress(content, compresslevel=9)

    with open(output_file, 'w', encoding='utf-8') as f:
        f.write('#pragma once\n')
        f.write('#include <Arduino.h>\n\n')
        f.write('// Auto-generated from web/dist/index.html (gzipped)\n')
        f.write(f'// Original size: {len(content)} bytes\n')
        f.write(f'// Compressed size: {len(compressed)} bytes\n')
        f.write(f'// Compression ratio: {len(compressed)/len(content)*100:.1f}%\n\n')
        f.write('const uint8_t index_html_gz[] PROGMEM = {\n')

        for i, byte in enumerate(compressed):
            if i % 16 == 0:
                f.write('  ')
            f.write(f'0x{byte:02x}')
            if i < len(compressed) - 1:
                f.write(', ')
            if i % 16 == 15:
                f.write('\n')

        if len(compressed) % 16 != 0:
            f.write('\n')
        f.write('};\n\n')
        f.write(f'const size_t index_html_gz_len = {len(compressed)};\n')

    print(f'Generated {os.path.abspath(output_file)}')
    print(f'  Original:   {len(content):>8,} bytes')
    print(f'  Compressed: {len(compressed):>8,} bytes')
    print(f'  Ratio:      {len(compressed)/len(content)*100:>7.1f}%')

if __name__ == '__main__':
    main()
