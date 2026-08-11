#!/usr/bin/env bash
set -euo pipefail

prefix="${HAMBAND_GCC83_ROOT:-$HOME/opt/gcc-8.3.0}"
cache="${HAMBAND_GCC83_CACHE:-$HOME/gcc83-debs}"
gcc_url="https://old-releases.ubuntu.com/ubuntu/pool/main/g/gcc-8"
isl_url="https://old-releases.ubuntu.com/ubuntu/pool/main/i/isl"

mkdir -p "$prefix" "$cache"

packages=(
  cpp-8_8.3.0-6ubuntu1_amd64.deb
  gcc-8_8.3.0-6ubuntu1_amd64.deb
  gcc-8-base_8.3.0-6ubuntu1_amd64.deb
  g++-8_8.3.0-6ubuntu1_amd64.deb
  libgcc-8-dev_8.3.0-6ubuntu1_amd64.deb
  libstdc++-8-dev_8.3.0-6ubuntu1_amd64.deb
)

for package in "${packages[@]}"; do
  if [[ ! -f "$cache/$package" ]]; then
    curl -fsSLo "$cache/$package" "$gcc_url/$package"
  fi
  dpkg-deb -x "$cache/$package" "$prefix"
done

isl_package=libisl19_0.20-2_amd64.deb
if [[ ! -f "$cache/$isl_package" ]]; then
  curl -fsSLo "$cache/$isl_package" "$isl_url/$isl_package"
fi
dpkg-deb -x "$cache/$isl_package" "$prefix"

mkdir -p "$prefix/usr/lib/x86_64-linux-gnu" "$prefix/bin"
if [[ ! -e "$prefix/usr/lib/x86_64-linux-gnu/libstdc++.so.6" ]]; then
  ln -s /usr/lib/x86_64-linux-gnu/libstdc++.so.6 \
    "$prefix/usr/lib/x86_64-linux-gnu/libstdc++.so.6"
fi

cat > "$prefix/bin/gcc" <<'WRAPPER'
#!/usr/bin/env bash
set -euo pipefail
prefix="${HAMBAND_GCC83_ROOT:-$HOME/opt/gcc-8.3.0}"
export LD_LIBRARY_PATH="$prefix/usr/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH:-}"
exec "$prefix/usr/bin/gcc-8" \
  -B"$prefix/usr/lib/gcc/x86_64-linux-gnu/8/" "$@"
WRAPPER

cat > "$prefix/bin/g++" <<'WRAPPER'
#!/usr/bin/env bash
set -euo pipefail
prefix="${HAMBAND_GCC83_ROOT:-$HOME/opt/gcc-8.3.0}"
export LD_LIBRARY_PATH="$prefix/usr/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH:-}"
exec "$prefix/usr/bin/g++-8" \
  -B"$prefix/usr/lib/gcc/x86_64-linux-gnu/8/" \
  -isystem "$prefix/usr/include/c++/8" \
  -isystem "$prefix/usr/include/x86_64-linux-gnu/c++/8" \
  -isystem "$prefix/usr/include/c++/8/backward" "$@"
WRAPPER

chmod +x "$prefix/bin/gcc" "$prefix/bin/g++"
"$prefix/bin/g++" --version | head -1
