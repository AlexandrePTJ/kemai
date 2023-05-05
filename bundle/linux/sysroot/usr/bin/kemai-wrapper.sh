#!/usr/bin/env bash

#
# Check if fallback libraries are present on host. If not, add embedded fallback libraries to path
#
function find_library()
{
  # Print full path to a library or return exit status 1 if not found
  local library="$1" # take library name as input (e.g. "libfoo.so")
  # Look for the library in $LD_LIBRARY_PATH
  local dir IFS=':' # split path into array on this separator
  for dir in "${LD_LIBRARY_PATH[@]}"; do
    if [[ -e "${dir}/${library}" ]]; then
      echo "${dir}/${library}"
      return # Library found
    fi
  done
  # Not found yet so check system cache
  ldconfig -p | awk -v lib="${library}" -v arch="${HOSTTYPE}" \
'
BEGIN {status=1}
($1 == lib && index($0, arch)) {print $NF; status=0; exit}
END {exit status}
'
}

fallback_libs=""
for fallback_dir in "${APPDIR}/fallback"/*; do
  libname="${fallback_dir##*/}"
  if ! find_library "${libname}"; then
    echo "${APPIMAGE}: Using fallback for ${libname}"
    fallback_libs="${fallback_libs}:${fallback_dir}"
  fi
done

export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}${fallback_libs}"

#
# Wrapper to run expected embedded Qinertia app
#
${APPDIR}/usr/bin/Kemai
