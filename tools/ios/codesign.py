#!/usr/bin/env python3
"""Signs an iOS app bundle (the examples' .app, built with the ios preset) for the devices of a provisioning profile.

    python3 tools/ios/codesign.py build/ios/bin/showcase.app                    # the best profile Xcode has
    python3 tools/ios/codesign.py build/ios/bin/showcase.app --profile x.mobileprovision

Without --profile: the profiles Xcode downloaded (~/Library/Developer/Xcode/UserData/Provisioning Profiles, and the
older ~/Library/MobileDevice/Provisioning Profiles) that have not expired, are for iOS, match the bundle identifier
(exactly, else through a wildcard such as TEAMID.*) and name a certificate whose private key is in the keychain; an
exact match before a wildcard, then development before ad hoc, then the one that expires last. Any profile works for
the devices it lists: development and ad hoc ones, from a paid team or Xcode's free "Personal Team".

The profile goes into the bundle (embedded.mobileprovision), the entitlements are the profile's with the wildcards
resolved to the bundle identifier, and codesign signs with the profile's certificate. No matching profile: a warning
and exit code 0, the bundle stays unsigned (CI builds without one); a failing codesign is an error.
"""
import argparse
import datetime
import glob
import os
import plistlib
import re
import subprocess
import sys
import tempfile

PROFILE_DIRS = [
    "~/Library/Developer/Xcode/UserData/Provisioning Profiles",
    "~/Library/MobileDevice/Provisioning Profiles",
]


def decode(path):
    out = subprocess.run(["security", "cms", "-D", "-i", path], capture_output=True)
    if out.returncode != 0:
        return None
    try:
        return plistlib.loads(out.stdout)
    except Exception:
        return None


def keychain_identities():
    """SHA-1 fingerprints of the code signing identities with their private key in the keychain."""
    out = subprocess.run(["security", "find-identity", "-v", "-p", "codesigning"], capture_output=True, text=True).stdout
    return set(re.findall(r"\b([0-9A-F]{40})\b", out))


def sha1(der):
    import hashlib
    return hashlib.sha1(der).hexdigest().upper()


def match(profile, bundle_id):
    """0 = exact, 1 = wildcard, None = not for this bundle identifier."""
    app_id = profile.get("Entitlements", {}).get("application-identifier", "")
    team = (profile.get("TeamIdentifier") or [""])[0]
    if not app_id.startswith(team + "."):
        return None
    pattern = app_id[len(team) + 1:]
    if pattern == bundle_id:
        return 0
    if pattern.endswith("*") and bundle_id.startswith(pattern[:-1]):
        return 1
    return None


def candidates(bundle_id, identities):
    now = datetime.datetime.now(datetime.timezone.utc)
    found = []
    for d in PROFILE_DIRS:
        for path in glob.glob(os.path.join(os.path.expanduser(d), "*.mobileprovision")):
            p = decode(path)
            if not p:
                continue
            expires = p.get("ExpirationDate")
            if expires and expires.replace(tzinfo=datetime.timezone.utc) < now:
                continue
            if "iOS" not in p.get("Platform", ["iOS"]):
                continue
            how = match(p, bundle_id)
            if how is None:
                continue
            certs = [sha1(c) for c in p.get("DeveloperCertificates", [])]
            identity = next((c for c in certs if c in identities), None)
            if not identity:
                continue
            development = bool(p.get("Entitlements", {}).get("get-task-allow"))
            found.append(((how, 0 if development else 1, -(expires.timestamp() if expires else 0)), path, p, identity))
    found.sort(key=lambda f: f[0])
    return found


def entitlements(profile, bundle_id):
    """The profile's entitlements with TEAMID.* resolved to TEAMID.<bundle id>."""
    team = (profile.get("TeamIdentifier") or [""])[0]
    resolved = team + "." + bundle_id
    out = {}
    for key, value in profile.get("Entitlements", {}).items():
        if key == "application-identifier":
            out[key] = resolved
        elif key == "keychain-access-groups":
            out[key] = [resolved if v.endswith("*") else v for v in value]
        elif isinstance(value, str) and value.endswith("*"):
            continue   # other wildcards (iCloud containers ...) are not the app's
        else:
            out[key] = value
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("app", help="the .app bundle")
    ap.add_argument("--profile", help="a .mobileprovision (default: the best one Xcode has)")
    args = ap.parse_args()

    app = args.app.rstrip("/")
    with open(os.path.join(app, "Info.plist"), "rb") as f:
        bundle_id = plistlib.load(f)["CFBundleIdentifier"]
    identities = keychain_identities()

    if args.profile:
        profile = decode(args.profile)
        if not profile:
            sys.exit(f"codesign: {args.profile} is no provisioning profile")
        identity = next((c for c in (sha1(d) for d in profile.get("DeveloperCertificates", [])) if c in identities), None)
        if not identity:
            sys.exit(f"codesign: no certificate of {args.profile} has its private key in the keychain")
        path = args.profile
    else:
        found = candidates(bundle_id, identities)
        if not found:
            print(f"codesign: warning: no provisioning profile for {bundle_id} with its certificate in the keychain: "
                  f"{os.path.basename(app)} stays unsigned (sign in to Xcode with your Apple ID, or --profile)")
            return 0
        _, path, profile, identity = found[0]

    with open(path, "rb") as src, open(os.path.join(app, "embedded.mobileprovision"), "wb") as dst:
        dst.write(src.read())
    with tempfile.NamedTemporaryFile(suffix=".plist", delete=False) as f:
        plistlib.dump(entitlements(profile, bundle_id), f)
        ent = f.name
    try:
        subprocess.run(["codesign", "--force", "--sign", identity, "--entitlements", ent, "--timestamp=none",
                        "--generate-entitlement-der", app], check=True)
    finally:
        os.unlink(ent)
    devices = len(profile.get("ProvisionedDevices", [])) or "all"
    print(f"codesign: {os.path.basename(app)} signed for {bundle_id} with \"{profile.get('Name')}\" ({devices} devices)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
