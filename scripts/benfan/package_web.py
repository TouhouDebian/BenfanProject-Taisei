#!/usr/bin/env python3
"""Package one WASM build into two separately addressable Pages editions."""
from pathlib import Path
import shutil
root = Path(__file__).resolve().parents[2]
source = root / 'dist-web'
output = root / '.site'
output.mkdir(exist_ok=True)
for name in ('taisei.js', 'taisei.wasm', 'COPYING.txt'):
    shutil.copy2(source / name, output / name)
shutil.copytree(source / 'data', output / 'data', dirs_exist_ok=True)
for name, rule in [('shooting', 0), ('dodging', 1)]:
    target = output / name
    target.mkdir(exist_ok=True)
    shutil.copy2(root / 'web-benfan/index.html', target / 'index.html')
    (target / 'mode.js').write_text(f'window.BENFAN_RULE={rule};\n')
    game = (root / 'web-benfan/game.html').read_text()
    game = game.replace('<meta charset="utf-8">', '<meta charset="utf-8"><base href="../">')
    game = game.replace('<script src="mode.js"></script>', f'<script>window.BENFAN_RULE={rule};</script>')
    game = game.replace('href="index.html"', f'href="{name}/"')
    (target / 'game.html').write_text(game)
(output / '.nojekyll').touch()
(output / 'index.html').write_text('''<!doctype html><html lang="zh-CN"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>奔饭 Project · 两种试玩</title><style>body{max-width:760px;margin:15vh auto;padding:30px;background:#edf0e7;color:#253e32;font:20px/1.8 system-ui}h1{font-size:56px}a{display:inline-block;background:#285240;color:white;padding:20px 35px;border-radius:12px;margin:12px;text-decoration:none}small{font-size:14px}</style><h1>奔饭 Project</h1><p>同一条奔饭路，两种玩法。四位角色，六面 + Extra。</p><a href="shooting/">A / 射击版 →</a><a href="dodging/">B / 避让版 →</a><p><small>基于 Taisei C / WebAssembly。玩法验证初版，部分立绘、Boss、音乐为上游素材。<br><a href="COPYING.txt">许可与署名</a></small></p></html>''')
print(output)
