#!/usr/bin/env python3
"""package.py: an Esia example's shared library as an Android APK - a NativeActivity, no Java code, no Gradle.

    python3 tools/android/package.py --lib build/android/bin/libshowcase.so --name showcase --package org.esia.showcase
                                     --sdk <Android SDK> --out build/android/apk/showcase.apk [--abi arm64-v8a]
                                     [--asset fonts/Icons.otf ...]

The SDK needs a platform (android.jar) and build-tools (aapt2, zipalign, apksigner); keytool and java come from the
JDK on PATH (or JAVA_HOME). Each --asset file goes into the APK's assets under its own name (AAssetManager_open).
The APK is signed with a debug key made next to it on first use, and is debuggable:
`adb exec-out run-as <package> cat files/<file>` reads what the app wrote.
"""
import argparse
import os
import shutil
import subprocess
import sys
import tempfile
import zipfile

MANIFEST = """<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android" package="{package}" android:versionCode="1" android:versionName="1.0">
    <uses-feature android:glEsVersion="0x00030000" android:required="true"/>
    <application android:label="{label}" android:hasCode="false" android:debuggable="true" android:extractNativeLibs="true"
                 android:theme="@android:style/Theme.DeviceDefault.NoActionBar.Fullscreen">
        <activity android:name="android.app.NativeActivity" android:exported="true" android:launchMode="singleTask"
                  android:configChanges="orientation|screenSize|smallestScreenSize|screenLayout|keyboardHidden|keyboard|density|uiMode"
                  android:windowSoftInputMode="adjustResize">
            <meta-data android:name="android.app.lib_name" android:value="{name}"/>
            <intent-filter>
                <action android:name="android.intent.action.MAIN"/>
                <category android:name="android.intent.category.LAUNCHER"/>
            </intent-filter>
        </activity>
    </application>
</manifest>
"""


def newest(directory):
    entries = sorted(os.listdir(directory), key=lambda v: [int(p) if p.isdigit() else p for p in v.replace('-', '.').split('.')])
    return os.path.join(directory, entries[-1])


def tool(build_tools, name):
    for suffix in ('', '.exe', '.bat'):
        p = os.path.join(build_tools, name + suffix)
        if os.path.exists(p):
            return p
    raise SystemExit(f'package.py: {name} is not in {build_tools}')


def java_env():
    # apksigner and keytool need a JDK: JAVA_HOME when it is one, else the java on PATH
    env = dict(os.environ)
    home = env.get('JAVA_HOME')
    if not home or not os.path.exists(os.path.join(home, 'bin')):
        java = shutil.which('java')
        if not java:
            raise SystemExit('package.py: no JDK (java on PATH or JAVA_HOME)')
        env['JAVA_HOME'] = os.path.dirname(os.path.dirname(os.path.realpath(java)))
    return env


def run(cmd, env=None):
    r = subprocess.run(cmd, env=env, capture_output=True, text=True)
    if r.returncode != 0:
        sys.stderr.write(r.stdout + r.stderr)
        raise SystemExit(f'package.py: {os.path.basename(cmd[0])} failed')


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--lib', required=True, help='the example as a shared library (lib<name>.so)')
    ap.add_argument('--name', required=True, help='the library name without lib and .so (the activity loads it)')
    ap.add_argument('--package', required=True)
    ap.add_argument('--label', default=None)
    ap.add_argument('--sdk', required=True)
    ap.add_argument('--abi', default='arm64-v8a')
    ap.add_argument('--min-sdk', default='28')
    ap.add_argument('--target-sdk', default=None, help='default: the newest platform in the SDK')
    ap.add_argument('--out', required=True)
    ap.add_argument('--asset', action='append', default=[], help="a file for the APK's assets (repeatable)")
    args = ap.parse_args()

    platform = newest(os.path.join(args.sdk, 'platforms'))
    target = args.target_sdk or os.path.basename(platform).split('-')[-1]
    build_tools = newest(os.path.join(args.sdk, 'build-tools'))
    env = java_env()
    out = os.path.abspath(args.out)
    os.makedirs(os.path.dirname(out), exist_ok=True)
    with tempfile.TemporaryDirectory() as tmp:
        manifest = os.path.join(tmp, 'AndroidManifest.xml')
        with open(manifest, 'w', encoding='utf-8') as f:
            f.write(MANIFEST.format(package=args.package, label=args.label or args.name, name=args.name))
        base = os.path.join(tmp, 'base.apk')
        link = [tool(build_tools, 'aapt2'), 'link', '-o', base, '--manifest', manifest, '-I', os.path.join(platform, 'android.jar'),
                '--min-sdk-version', args.min_sdk, '--target-sdk-version', target]
        if args.asset:
            assets = os.path.join(tmp, 'assets')
            os.makedirs(assets)
            for a in args.asset:
                shutil.copyfile(a, os.path.join(assets, os.path.basename(a)))
            link += ['-A', assets]
        run(link)
        with zipfile.ZipFile(base, 'a', zipfile.ZIP_DEFLATED) as z:
            z.write(args.lib, f'lib/{args.abi}/lib{args.name}.so')
        aligned = os.path.join(tmp, 'aligned.apk')
        run([tool(build_tools, 'zipalign'), '-p', '-f', '4', base, aligned])
        keystore = os.path.join(os.path.dirname(out), 'debug.keystore')
        if not os.path.exists(keystore):
            keytool = os.path.join(env['JAVA_HOME'], 'bin', 'keytool.exe' if os.name == 'nt' else 'keytool')
            run([keytool, '-genkeypair', '-keystore', keystore, '-storepass', 'android', '-alias', 'androiddebugkey', '-keypass', 'android',
                 '-keyalg', 'RSA', '-keysize', '2048', '-validity', '10000', '-dname', 'CN=Android Debug,O=Android,C=US'], env)
        run([tool(build_tools, 'apksigner'), 'sign', '--ks', keystore, '--ks-pass', 'pass:android', '--key-pass', 'pass:android',
             '--out', out, aligned], env)
    print(f'package.py: {out}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
